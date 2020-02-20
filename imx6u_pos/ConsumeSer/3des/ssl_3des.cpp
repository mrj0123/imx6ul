#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/des.h>
#include <openssl/rand.h>

#include "basetype.h"

#define KEY_SIZE (24)

extern char *base64_encode(unsigned char *bindata, char * base64, int binlength);
extern int base64_decode(const char * base64, unsigned char * bindata);

int encrypt_3des_ecb3(U8 *in, U8 *out, S8 *key)
{
    DES_key_schedule ks1, ks2, ks3;
    S8 acKey[KEY_SIZE];
    int iIndex = 0;
    int len = 0;
    int nlen = 0;
    char ch = '\0';
    U8 src[1024] = {0};

    memset(acKey, 0, sizeof(acKey));
    len = strlen((char *)key);
    for(iIndex = 0; iIndex < len; iIndex++)
    {
        acKey[iIndex] = key[iIndex];
    }
    for(; iIndex < sizeof(acKey); iIndex++)
    {
        acKey[iIndex] = 0x0;
    }

    DES_set_key_unchecked((C_Block *)(&acKey[0]), &ks1);
    DES_set_key_unchecked((C_Block *)(&acKey[8]), &ks2);
    DES_set_key_unchecked((C_Block *)(&acKey[16]), &ks3);

    nlen = strlen((char *)in);
    memcpy(src, in, nlen);
    len = (nlen / 8 + (nlen % 8 ? 1 : 0)) * 8;

    ch = 8 - nlen % 8;
    memset(src + nlen, ch, (8 - nlen % 8) % 8);
    for(iIndex = 0; iIndex < len; iIndex += 8)
    {
        DES_ecb3_encrypt((C_Block *)(src + iIndex), (C_Block *)(out + iIndex), &ks1, &ks2, &ks3, DES_ENCRYPT);
    }

    return 0;
}

int decrypt_3des_ecb3(U8 *in, U8 *out, S8 *key)
{
    DES_key_schedule ks1, ks2, ks3;
    S8 acKey[KEY_SIZE];
    int iIndex = 0;
    int len = 0;
    int nlen = 0;
    char ch = '\0';
    U8 src[1024] = {0};

    memset(acKey, 0, sizeof(acKey));
    len = strlen((char *)key);
    for(iIndex = 0; iIndex < len; iIndex++)
    {
        acKey[iIndex] = key[iIndex];
    }
    for(; iIndex < sizeof(acKey); iIndex++)
    {
        acKey[iIndex] = 0x0;
    }

    DES_set_key_unchecked((C_Block *)(&acKey[0]), &ks1);
    DES_set_key_unchecked((C_Block *)(&acKey[8]), &ks2);
    DES_set_key_unchecked((C_Block *)(&acKey[16]), &ks3);

    nlen = strlen((char *)in);
    memcpy(src, in, nlen);
    len = (nlen / 8 + (nlen % 8 ? 1 : 0)) * 8;

    ch = 8 - nlen % 8;
    memset(src + nlen, ch, (8 - nlen % 8) % 8);
    for(iIndex = 0; iIndex < len; iIndex += 8)
    {
        DES_ecb3_encrypt((C_Block *)(src + iIndex), (C_Block *)(out + iIndex), &ks1, &ks2, &ks3, DES_DECRYPT);
    }

    return 0;
}

//加密：对原始待加密数据进行base64操作，并进行cbc 3des 加密，然后再进行base64操作，最后得到结果
int DES_bs64_3cbc_bs64_encrypt(const unsigned char *input,      //输入待加密数据
                                   unsigned char *output,       //输出加密后数据
                                   long length,                 //输入数据长度
                                   char *key1,                  //8字节秘钥1
                                   char *key2,                  //8字节秘钥2
                                   char *key3,                  //8字节秘钥3
                                   DES_cblock ivec,           //偏移量
                                   int pack)                    //填充方式
{
    //初始化
    int i=0;
    int len=0;
    int nlen=0;
    char ch='\0';
    unsigned char * tmp;
    unsigned char * tmp1;
    unsigned char * out;
    unsigned char * ptr=NULL;
    unsigned char block[8]={0};
    DES_key_schedule ks1, ks2, ks3;

    //获得明文长度
    nlen=length;
    ////////////////////////////////
    myPtf("\n inputdata: ");
    for (i = 0; i < nlen; i++) {
        myPtf("%02X", *(input + i));
    }
    myPtf("\n");
    ////////////////////////////////
    //对明文BASE64
    tmp=malloc((nlen/3+1)*4+8+1);
    memset(tmp,0,(nlen/3+1)*4+8+1);
    //base64
    base64_encode((unsigned char *)input, (char *)tmp, nlen);
    nlen=strlen((char *)tmp);
    /////////////////////////////
    myPtf("first base64 len=%d:",nlen);
    for (i = 0; i < nlen; i++) {
        myPtf("%02X", *(tmp+ i));
    }
    myPtf("\n");
    //////////////////////////////
    if (pack==PKCS7)
    {
        //pkcs #7 填充方式
        len = (nlen / 8 + (nlen % 8 ? 1: 0)) * 8;
        ch = 8 - nlen % 8;
        memset(tmp + nlen, ch, (8 - nlen % 8) % 8);
    }else if (pack==PKCS5)
    {
        //pkcs #5 填充方式
        len = (nlen / 8 + 1) * 8;
        ch = 8 - nlen % 8;
        memset(tmp + nlen, ch, len-nlen);
    }
    else if(pack==NOPADDING)
    {
        //pkcs #7 填充方式
        len = (nlen / 8 + (nlen % 8 ? 1: 0)) * 8;
        ch = 0;
        memset(tmp + nlen, ch, (8 - nlen % 8) % 8);
    }
    else
    {
        return -1;
    }
    //////////////////////////////
    myPtf("PKCS5 len=%d:",len);
    for (i = 0; i < len; i++) {
        myPtf("%02X", *(tmp + i));
    }
    myPtf("\n");
    /////////////////////////////
    /* 设置密码表 */
    DES_set_key_unchecked((C_Block *)key1, &ks1);
    DES_set_key_unchecked((C_Block *)key2, &ks2);
    DES_set_key_unchecked((C_Block *)key3, &ks3);
    myPtf("加密：key1=%s,key2=%s,key3=%s,sizeof(ivec)=%d\n",key1,key2,key3,sizeof(DES_cblock));
    DES_cblock ivsetup;
    memcpy(ivsetup, ivec,8);
    myPtf("len=%d,sizeof(ivec)=%d\n",len,sizeof(DES_cblock));
    tmp1=malloc(len);
    myPtf("malloc no err!\n");
    memset(tmp1,0,len);
    /* 按照8字节数据进行加密，length=8 */
    for (i=0;i<len;i+=8)
    {
        myPtf("i=%d\n",i);
        DES_ede3_cbc_encrypt(tmp+i,tmp1+i,8,&ks1,&ks2,&ks3,&ivsetup,DES_ENCRYPT);
    }
    myPtf("3des len=%d: ",len);
    for (i=0;i<len;i++) {
        myPtf("%02X",*(tmp1+i));
    }
    myPtf("\n");
    /////////////////////////////
    //再次BASE64
    /*
    out=malloc((len/3+1)*4+1);
    memset(out,0,(len/3+1)*4+1);
    base64_encode(tmp1,(char *)out,len);
    len=strlen((char*) out);
    myPtf("second base64 len=%d:\n",len);
    for (i = 0; i < len; i++) {
        myPtf("%02X", *(out + i));
    }
    myPtf("\n");
    memcpy(output,out,len);
    */
    /////////////////////////////
    base64_encode(tmp1,(char *)output,len);
    len=strlen((char*) output);
    myPtf("second base64 len=%d:\n",len);
    for (i = 0; i < len; i++) {
        myPtf("%02X", *(output + i));
    }
    myPtf("\n");

    myPtf("\n");
    free(tmp);
    free(tmp1);
    return 1;
}

//解密时：对输入的密文进行反base64操作，然后用cbc 3des 进行解密，最后再进行dbase64操作，最后得到明文
int DES_dbs64_3dcbc_dbs64_decrypt(const unsigned char *input,   //输入待解密数据（密文）
                                   unsigned char *output,       //输出解密后数据（明文）
                                   long length,                 //输出数据长度  填8，表示按8字节数据进行加密
                                   char *key1,                  //8字节秘钥1
                                   char *key2,                  //8字节秘钥2
                                   char *key3,                  //8字节秘钥3
                                   DES_cblock ivec,            //偏移量
                                   int pack)                    //填充方式
{
    //初始化
    int i=0;
    int len=0;

    unsigned char * tmp,* tmp1;

    DES_key_schedule ks1, ks2, ks3;

    //解base64
    tmp=malloc((length/3+1)*4+1);
    memset(tmp,0,(length/3+1)*4+1);
    len=base64_decode((char *)input, tmp);
    myPtf("解密前dbase64后:");
    for (i = 0; i < len; i++) {
        myPtf("%02X", *(tmp + i));
    }
    myPtf("\n");
    /////////////////////////
    /* 设置密码表 */
    DES_set_key_unchecked((C_Block *)key1, &ks1);
    DES_set_key_unchecked((C_Block *)key2, &ks2);
    DES_set_key_unchecked((C_Block *)key3, &ks3);
    /////////////////////////////
    /* 按照8字节数据进行解密，length=8 */
    DES_cblock ivsetup;
    memcpy(ivsetup, ivec, sizeof(DES_cblock));
    myPtf("解密：key1=%s,key2=%s,key3=%s\n",key1,key2,key3);
    tmp1=malloc(len);
    memset(tmp1,0,len);
    for (i = 0; i < len; i += 8) {
        DES_ede3_cbc_encrypt(tmp + i, tmp1 + i, 8, &ks1, &ks2, &ks3, &ivsetup, DES_DECRYPT);
    }
    myPtf("解密后数据:");
    for (i = 0; i < len; i++) {
        myPtf("%02X", *(tmp1 + i));
    }
    myPtf("\n");
    /////////////////////////////
    //去掉填充
    unsigned char ucdel=0;
    if(pack==NOPADDING)
    {
        for(i=0;i<8;i++)
        {
            if (tmp1[len-i-1]!=0)
            {
                break;
            }
        }
        ucdel=i;
    }
    else
    {
        ucdel=tmp1[len-1];
    }
    unsigned char * temp2=malloc(len-ucdel+1);
    memset(temp2,0,len-ucdel+1);
    len=len-ucdel;
    memcpy(temp2,tmp1,len);
    myPtf("解密后dbase前数据ucdel=%d,len=%d:",ucdel,len);
    for (i = 0; i < len; i++) {
        myPtf("%02X", *(temp2 + i));
    }
    myPtf("\n");
    /////////////////////////////
    //dBASE64
    len=base64_decode((char *)temp2, output);
    output[len]='\0';//结束符
    myPtf("解密后dbase64后:");
    myPtf("解密后数据:");
    for (i = 0; i < len; i++) {
        myPtf("%02X", *(output + i));
    }
    myPtf("\n");
    free(tmp);
    free(tmp1);
    return 1;
}
