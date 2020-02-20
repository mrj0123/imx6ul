#include "sendcmd_cloud.h"
#include "nksocket.h"

//static char deskey[24] = "ayjQkRQhKYewjz6dCwaEwgNZ";//"woshikeykkkk111333222111";//秘钥
//static DES_cblock ivsetup = "nktynkty"; //"nihaobua";

static char deskey[24] = "woshikeykkkk111333222111";//秘钥
static DES_cblock ivsetup = "nihaobua";

static char deskey1[9];
static char deskey2[9];
static char deskey3[9];

static char recv_data[MAXLINE];
static pthread_mutex_t socket_mutex;


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

int init_key()
{
    memset(deskey1,0,9);
    memset(deskey2,0,9);
    memset(deskey3,0,9);
    memcpy(deskey1,&deskey[0],8);
    memcpy(deskey2,&deskey[8],8);
    memcpy(deskey3,&deskey[16],8);
    return 0;
}

//初始化链接
int init_client_socket(ser_conn_t server_conn[MAX_SERVER_NUM])
{
    //初始化通讯互斥
    pthread_mutex_init(&socket_mutex,NULL);
    init_key();
    return init_socket(server_conn);
}
//断开链接
int close_client_socket()
{
    //释放互斥变量
    pthread_mutex_destroy(&socket_mutex);
    return close_socket();
}
//发送命令
char * sendCmd_socket(int *pCmd, char * pBuf, int bufLen, int *retLen)
{

    time_t timep;
    time (&timep);
    myPtf("sendCmd_socket input *pCmd=%d,pBuf=%s,bufLen=%d,*retLen=%d,time:%s\n",*pCmd,pBuf,bufLen,*retLen,ctime(&timep));
    int tsp = getCurrentTime();
    int nowTime = getCurrentTime();
    myPtf("sendCmd_socket nowTime start-----------------tsp=%d,nowTime=%d\n",tsp,nowTime);
    pthread_mutex_lock(&socket_mutex);
    *retLen = 0;
    *pCmd = -1;
    ///////////////////////////////////////////////////////////////
    //每次重新连接服务
    if(realinit_socket() == -1)
    {
        time (&timep);
        myPtf("connect socket error!time:%s\n",ctime(&timep));
        //链接服务失败
        pthread_mutex_unlock(&socket_mutex);
        myPtf("sendCmd_socket finish-----------------%d\n",tsp);
        return NULL;
    }
    nowTime = getCurrentTime();
    myPtf("sendCmd_socket nowTime realinit_socket ok-----------------tsp=%d,nowTime=%d\n",tsp,nowTime);
    //分配输出缓存
    time (&timep);
    myPtf("sendCmd_socket bufLen=%d,time:%s\n",bufLen,ctime(&timep));
    int outlen=(((bufLen/3+1)*4+1+8)/3+1)*4+2;//经过base64,再补padding，再base64后的长度
    myPtf("sendCmd_socket outlen=%d\n",outlen);
    unsigned char * data; //输出
    data=malloc(outlen);
    memset(data,0,outlen);
    //加密
    //myPtf("heererererer \n");
    if(DES_bs64_3cbc_bs64_encrypt((unsigned char *)pBuf,data,bufLen,deskey1,deskey2,deskey3,ivsetup,PKCS5)>0)
    {
        //myPtf("加密结果:%s\n",data);
    }
    else
    {
        myPtf("error!\n");
    }

    ///////////////////////////////////////////////////////////////
    //发送命令
    nowTime = getCurrentTime();
    myPtf("sendCmd_socket nowTime send_socket begin-----------------tsp=%d,nowTime=%d\n",tsp,nowTime);
    if (send_socket((char *)data,strlen((char *)data)+1)<0) //OK
    {
        //发送失败，网络故障
        nowTime = getCurrentTime();
        myPtf("sendCmd_socket nowTime send_socket error-----------------data=%d,tsp=%d,nowTime=%d\n",(int)data,tsp,nowTime);
        *pCmd = CLD_SEND_ERROR;
        free(data);
        close_socket();
        pthread_mutex_unlock(&socket_mutex);
        myPtf("sendCmd_socket finish-----------------%d\n",tsp);
        return NULL;
    }
    nowTime = getCurrentTime();
    myPtf("sendCmd_socket nowTime send_socket ok-----------------tsp=%d,nowTime=%d\n",tsp,nowTime);
    //释放发送内存
    free(data);
    //myPtf("发送命令完成\n");
    //接收返回
    int recv_len = 0;
    memset(recv_data,0,MAXLINE);
    //接收数据错误
    /////////////////////////////////////////////////////////////////////yyyyyyyyyyyyyyyyyyyyyyyyyyyy
    recv_len = recv_socket(recv_data,MAXLINE);//recv_data最后一个字符非结束符'\0'
    if(recv_len<=0)
    {
        //发送失败，网络故障
        nowTime = getCurrentTime();
        myPtf("sendCmd_socket nowTime recv_socket error-----------------tsp=%d,nowTime=%d\n",tsp,nowTime);
        *pCmd = CLD_RECV_ERROR;
        close_socket();
        //myPtf("???\n");
        pthread_mutex_unlock(&socket_mutex);
        myPtf("sendCmd_socket finish-----------------%d\n",tsp);
        return NULL;
    }
    nowTime = getCurrentTime();
    myPtf("sendCmd_socket nowTime recv_socket ok-----------------tsp=%d,nowTime=%d\n",tsp,nowTime);
    //关闭连接
    close_socket();
    ///////////////////////////////////////////////////////////////
    //解密
    char * outbuff;
    //分配的内存足够大，比返回的ret+1大
    outbuff=malloc(recv_len);
    memset(outbuff,0,recv_len);
    myPtf("maxoutbuf=%d\n",recv_len);
    int ret = DES_dbs64_3dcbc_dbs64_decrypt((unsigned char *)recv_data,(unsigned char *)outbuff,recv_len,deskey1,deskey2,deskey3,ivsetup,PKCS5);
    if(ret>0)
    {
        myPtf("解码结果:%s,ret=%d\n",outbuff,ret);
    }
    else
    {
        myPtf("error!\n");
    }
    *retLen = ret+1;    //返回字符串包含结束符'\0'
    myPtf("i am return retlen=%d data=",*retLen);
    for(int i=0;i<ret+1;i++)
    {
        myPtf("%02X",*(outbuff+i));
    }
    myPtf("!!!\n");
    *pCmd = 1;
    //返回
    pthread_mutex_unlock(&socket_mutex);
    nowTime = getCurrentTime();
    myPtf("sendCmd_socket nowTime over-----------------tsp=%d,nowTime=%d\n",tsp,nowTime);
    return outbuff;
}
