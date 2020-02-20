#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "a.h"
static char FCODE[]="12457801";
static char QRdeskey[24] = "YXP,sEY@.X&yrt@lZnk.2@O0";//秘钥
static DES_cblock QRivsetup = "R*AK@Z&w";

extern int DES_dbs64_3dcbc_decrypt(const unsigned char *input,   //输入待解密数据（密文）
                                   unsigned char *output,       //输出解密后数据（明文）
                                   long length,                 //输出数据长度  填8，表示按8字节数据进行加密
                                   char *key1,                  //8字节秘钥1
                                   char *key2,                  //8字节秘钥2
                                   char *key3,                  //8字节秘钥3
                                   DES_cblock  ivec,            //偏移量
                                   int pack);                    //填充方式


//加密：对原始待加密数据进行cbc 3des 加密，然后再进行base64操作，最后得到结果
extern int DES_3cbc_bs64_encrypt(const unsigned char *input,      //输入待加密数据
                                   unsigned char *output,       //输出加密后数据
                                   long length,                 //输入数据长度
                                   char *key1,                  //8字节秘钥1
                                   char *key2,                  //8字节秘钥2
                                   char *key3,                  //8字节秘钥3
                                   DES_cblock ivec,           //偏移量
                                   int pack);                    //填充方式

int fn_DecryptForBOC(char * ciphertext,int * accountId)
{

    char deskey1[9];
    char deskey2[9];
    char deskey3[9];
    memset(deskey1,0,9);
    memset(deskey2,0,9);
    memset(deskey3,0,9);

    memcpy(deskey1,&QRdeskey[0],8);
    memcpy(deskey2,&QRdeskey[8],8);
    memcpy(deskey3,&QRdeskey[16],8);

    int recv_len = strlen(ciphertext);
    char * outbuff;
    outbuff = malloc(recv_len);
    memset(outbuff,0,recv_len);

    //////////////////////////////////////////////////
    /*char pBuf[1024]="124578010000000000000000003591852018091214584500";
    char data[1024]="";
    //测试加密
    if(DES_3cbc_bs64_encrypt((unsigned char *)pBuf,(unsigned char *)data,strlen(pBuf),deskey1,deskey2,deskey3,QRivsetup,NOPADDING)>0)
    {
        myPtf("加密结果:%s\n",data);
    }
    else
    {
        myPtf("error!\n");
    }

    recv_len = strlen(data);
    memcpy(ciphertext,data,recv_len);*/
    //////////////////////////////////////////////////

    int ret = DES_dbs64_3dcbc_decrypt((unsigned char *)ciphertext,(unsigned char *)outbuff,recv_len,deskey1,deskey2,deskey3,QRivsetup,NOPADDING);
    myPtf("return fn_DecryptForBOC ok\n");
    if(ret>0)
    {
        myPtf("解码结果:%s\n",outbuff);
    }
    else
    {
        free(outbuff);
        myPtf("error 1!\n");
        return -1;
    }
    //广分
    if (UseTermType == USERTYPE_BOC_GZ)
    {
        if (strlen(outbuff)!=48)
        {
            free(outbuff);
            myPtf("error 2!\n");
            return -1;
        }
    }
    else if (UseTermType == USERTYPE_BOC_HQ)
    {
        if (strlen(outbuff)!=56)
        {
            free(outbuff);
            myPtf("error 2!\n");
            return -1;
        }
    }

    //初始化参数
    char factoryCode[9];
    memset(factoryCode,0,9);
    char accId[25];
    memset(accId,0,25);
    char consumetime[17];
    memset(consumetime,0,17);
    //取出参数
    memcpy(factoryCode,outbuff,8);
    memcpy(accId,outbuff+8,24);
    memcpy(consumetime,outbuff+32,14);
    free(outbuff);
    //比较厂商编号
    if (strcmp(factoryCode,FCODE)!=0)
    {
        return -3;
    }
    //比较时间
    struct tm tm1;
    myPtf("Qrtime %s\n",consumetime);
    sscanf(consumetime, "%4d%2d%2d%2d%2d%2d",
          &tm1.tm_year,
          &tm1.tm_mon,
          &tm1.tm_mday,
          &tm1.tm_hour,
          &tm1.tm_min,
          &tm1.tm_sec);
    tm1.tm_year -= 1900;
    tm1.tm_mon --;
    tm1.tm_isdst=-1;

    time_t QrTime = mktime(&tm1);
    time_t nowTime;
    time(&nowTime);
    double diff = difftime(nowTime,QrTime);
    myPtf("nowTime=%d,QrTime=%d",(int)nowTime,(int)QrTime);
    if((diff>300)||(diff<-300))
    {
        return -2;
    }
    //返回账号
    *accountId = atoi(accId);
    return 0;
}


int main()
{
    //char strOld[] = "3khW4lQngBm0itA07VoeTQnhHWN8rowe0a6PBGjvye7gJ+WOJVTLT9+/r4l1zYk=";//"3khW4lQngBm0itA07VoeTQnhHWN8roweMgXrn1kBCaXFQS0p1U4iySmRvRGq5cYP";
    char strOld[] = "https://u.wechat.com/MIeGnGntyuH2sSA8868XW1A";
    int accountid = 0;
    myPtf("%s\n",strOld);
    if (strlen(strOld)!=64)
    {
        myPtf("length error!\n");
        return 0;
    }

    int ret = fn_DecryptForBOC(strOld,&accountid);
    myPtf("ret = %d,accountId=%d",ret,accountid);
    return 0;
}
