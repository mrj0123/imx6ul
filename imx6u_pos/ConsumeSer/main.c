#include "a.h"
#include "basetype.h"
#include "dealcmd.h"
#include "dispose.h"
#include "afunix_udp.h"
#include "common.h"

//////////////////////////////////
/*
extern int decrypt_3des_ecb3(U8 *in, U8 *out, S8 *key);
extern int encrypt_3des_ecb3(U8 *in, U8 *out, S8 *key);

extern int DES_bs64_3cbc_bs64_encrypt(const unsigned char *input,      //输入待加密数据
                                   unsigned char *output,       //输出加密后数据
                                   long length,                 //输出数据长度  填8，表示按8字节数据进行加密
                                   char *key1,                  //8字节秘钥1
                                   char *key2,                  //8字节秘钥2
                                   char *key3,                  //8字节秘钥3
                                   DES_cblock  ivec,            //偏移量
                                   int pack);                    //填充方式
extern int DES_dbs64_3dcbc_dbs64_decrypt(const unsigned char *input,   //输入待解密数据（密文）
                                   unsigned char *output,       //输出解密后数据（明文）
                                   long length,                 //输出数据长度  填8，表示按8字节数据进行加密
                                   char *key1,                  //8字节秘钥1
                                   char *key2,                  //8字节秘钥2
                                   char *key3,                  //8字节秘钥3
                                   DES_cblock  ivec,            //偏移量
                                   int pack);                    //填充方式


int getCurrentTime()
{
   struct timeval tv;
   gettimeofday(&tv,NULL);
   myPtf("tv.tv_sec=%ld,tv.tv_usec=%ld\n",tv.tv_sec,tv.tv_usec);
   //return ((long long int)tv.tv_sec) * 1000 + ((long long int)tv.tv_usec) / 1000;
   return (tv.tv_sec%(24*60*60))*10000 + tv.tv_usec/100;
}
*/
char* getPidFromStr(const char *str)
{
    myPtf("process name is %s\n",str);
    static char sPID[8] = {0};
    int tmp = 0;
    int pos1 = 0;
    int pos2 = 0;
    int i = 0;
    int j = 0;

    for (i=0; i<strlen(str); i++) {
        if ( (tmp==0) && (str[i]>='0' && str[i]<='9') ) {
            tmp = 1;
            pos1 = i;
        }
        if ( (tmp==1) && (str[i]<'0' || str[i]>'9') ) {
            pos2 = i;
            break;
        }
    }
    for (j=0,i=pos1; i<pos2; i++,j++) {
        sPID[j] = str[i];
    }
    myPtf("oldPid=%s\n",sPID);
    return sPID;
}


int isRunning()
{
    int ret = 0;
    char sCurrPid[16] = {0};
    sprintf(sCurrPid, "%d", getpid());
    myPtf("sCurrPid is %s\n",sCurrPid);
    FILE *fstream=NULL;
    char buff[1024] = {0};
    if(NULL==(fstream=popen("ps -e -o pid,comm | grep consumeser | grep -v PID | grep -v grep", "r")))
    {
        fprintf(stderr,"execute command failed: %s", strerror(errno));
        return -1;
    }
    while(NULL!=fgets(buff, sizeof(buff), fstream)) {
        char *oldPID = getPidFromStr(buff);
        if ( strcmp(sCurrPid, oldPID) != 0 ) {
            myPtf("程序已经运行，PID=%s\n", oldPID);
            ret = 1;
        }
    }
    pclose(fstream);

    return ret;
}


//启动主程序
int main()
{
    myPtf("consumeser version=%s\n",CONSUMESERVERSION);
    int cct=getCurrentTime();
    myPtf("c/c++ program:%d %02d:%02d:%02d\n",cct,cct/10000/60/60+8,cct/10000/60%60,cct/10000%60);
    /////////////////////////////////////
    if (isRunning()==1)
    {
        myPtf("consumeser is running!\n");
        return 0;
    }
    //////////////////////////////////////////////////////////////////
    //
    fn_init_dispose();
    //启动服务
    int serverfd = s_init_net(CONSUMEBIZ_PATH_S);
    if (serverfd<=0)
    {
         myPtf("serverfd init error:%d",serverfd);
         return -1;
    }
    myPtf("consumeser init serverfd=%d------------------------------servpath=%s\n",serverfd,CONSUMEBIZ_PATH_S);
    //循环接收服务
    fn_dispose(serverfd);
    //关闭服务
    close_net(serverfd,CONSUMEBIZ_PATH_S);
    fn_destroy_dispose();
    return 0;
}
