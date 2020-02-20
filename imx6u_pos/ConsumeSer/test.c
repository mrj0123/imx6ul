
#include <termio.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include <openssl/des.h>
#include <openssl/rand.h>

#include "basetype.h"

/*
extern int decrypt_3des_ecb3(U8 *in, U8 *out, S8 *key);
extern int encrypt_3des_ecb3(U8 *in, U8 *out, S8 *key);

extern char *base64_encode(unsigned char *bindata, char * base64, int binlength);
extern int base64_decode(const char * base64, unsigned char * bindata);
*/
extern char *base64_encode(unsigned char *bindata, char * base64, int binlength);
extern int base64_decode(const char * base64, unsigned char * bindata);

/************************************************************************
 * 3DES-ECB加密方式
 * 8字节密钥，加密内容8位补齐，补齐方式为：PKCS7。
 *
 * file: test_des3_ecb.c
 * gcc -Wall -O2 -o test_des3_cbc test_des3_cbc.c hex.c -lcrypto
 *
 * author: tonglulin@gmail.com by www.qmailer.net
 ************************************************************************/
int main(int argc, char *argv[])
{
    int i = 0;
    int len = 0;
    int nlen = 0;

    char ch = '\0';
    char key1[] = "woshikey";  /* 原始密钥, 十六进制字符串 */
    char key2[] = "kkkk1113";  /* 原始密钥, 十六进制字符串 */
    char key3[] = "33222111";  /* 原始密钥, 十六进制字符串 */
    char data[] = "1234君不见黄河之水天上来奔流到海不复回君不见高堂明镜悲白发朝如青丝暮成雪人生得意须尽欢莫使金樽空对月天生我才必有用千金散尽还复来中国人nkty2018";  /* 原始明文, 十六进制字符串 */
    //char data[] = "1234君不见黄河之水天上nkty2018";  /* 原始明文, 十六进制字符串 */
    //char data[] = "123456789A123456走私789B123456789C123456789D123456789E123456789F123456789G123456789H123456789A123456789B123456789C123456789D123456789E123456789F12345678夹带9G123456789H1";  /* 原始明文, 十六进制字符串 */
    char tempdate[1024];
    unsigned char src[1024] = {0};
    memset(src,0,1024);
    unsigned char out[1024] = {0};
    memset(out,0,1024);
    unsigned char tmp[1024] = {0};
    memset(tmp,0,1024);
    unsigned char tmp1[1024] = {0};
    memset(tmp1,0,1024);

    unsigned char *ptr  = NULL;
    unsigned char block[8] = {0};
    DES_key_schedule ks1, ks2, ks3;
    DES_cblock ivec;
    DES_cblock ivsetup = "nihaobua";

    nlen=strlen(data);
    ////////////////////////////////
    myPtf("明文数据: ");
    for (i = 0; i < nlen; i++) {
        myPtf("%02X", *(data + i));
    }
    myPtf("\n");
    ////////////////////////////////

    memset(src,0,1024);
    memcpy(src,data,nlen);
    myPtf("加密前数据: ");
    for (i = 0; i <nlen; i++) {
        myPtf("%02X", *(src + i));
    }
    myPtf("\n");

    //BASE64
    base64_encode(src, tmp, nlen);
    nlen=strlen(tmp);
    myPtf("加密前base64后字符串:%s\n",tmp);
    memset(src,0,1024);
    memcpy(src,tmp,nlen);
    /////////////////////////////
    myPtf("加密前base64后长度%d:",nlen);
    for (i = 0; i < nlen; i++) {
        myPtf("%02X", *(src + i));
    }
    myPtf("\n");
    //////////////////////////////
    //pkcs #7 填充方式
    /*
    len = (nlen / 8 + (nlen % 8 ? 1: 0)) * 8;
    ch = 8 - nlen % 8;
    memset(src + nlen, ch, (8 - nlen % 8) % 8);

    //pkcs #5 填充方式
*/
    len = (nlen / 8 + 1) * 8;
    ch = 8 - nlen % 8;
    if (ch==4)
    {
        ch=0x04;
    }
    memset(src + nlen, ch, len-nlen);
    //len=nlen;
    //////////////////////////////
    myPtf("加密前base64后补足长度%d:",len);
    for (i = 0; i < len; i++) {
        myPtf("%02X", *(src + i));
    }
    myPtf("\n");
    /////////////////////////////
    /* 设置密码表 */
    DES_set_key_unchecked((C_Block *)key1, &ks1);
    DES_set_key_unchecked((C_Block *)key2, &ks2);
    DES_set_key_unchecked((C_Block *)key3, &ks3);

    memcpy(ivec, ivsetup, strlen(ivsetup));
    /* 按照8字节数据进行加密，length=8 */
    for (i = 0; i < len; i += 8) {
        DES_ede3_cbc_encrypt(src + i, out + i, 8, &ks1, &ks2, &ks3, &ivec, DES_ENCRYPT);
    }
    //len=strlen(out);
    myPtf("加密后长度%d,数据: ",len);
    for (i = 0; i < len; i++) {
        myPtf("%02X" , *(out + i));
    }
    myPtf("\n");
    /////////////////////////////
    //BASE64
    base64_encode(out, tmp, len);
    len=strlen(tmp);
    myPtf("加密后长度%d,base64:%s\n",len,tmp);
    memset(out,0,1024);
    memcpy(out,tmp,len);
    //tempdate="O8MDyyGeOMNrEHHIfLe6fQ2WOHKCvETdeXKhfzMJv3Y=";
    //memcpy(out,temdate,strlen(tempdate));
    /////////////////////////////

    /////////////////////////////
    //dBASE64
    memset(tmp,0,1024);
    len=base64_decode(out, tmp);

    myPtf("解密前dbase64后:");
    for (i = 0; i < len; i++) {
        myPtf("%02X", *(tmp + i));
    }
    myPtf("\n");
    memset(out,0,1024);
    memcpy(out,tmp,len);
    /////////////////////////////

    memcpy(ivec, ivsetup, sizeof(ivsetup));
    /* 按照8字节数据进行解密，length=8 */
    for (i = 0; i < len; i += 8) {
        DES_ede3_cbc_encrypt(out + i, tmp1 + i, 8, &ks1, &ks2, &ks3, &ivec, DES_DECRYPT);
    }
    myPtf("解密后%d:",len);
    for (i = 0; i < len; i++) {
        myPtf("%02X", *(tmp1 + i));
    }
    myPtf("\n");
    ////////////////////////////////////////////////
    //去掉填充？？？？？
    unsigned char ucdel=0;
    /*
    ucdel=tmp1[len-1];
    unsigned char * temp2=malloc(len-ucdel+1);
    memset(temp2,0,len-ucdel+1);
    len=len-ucdel;
    memcpy(temp2,tmp1,len);
    */
    unsigned char temp2[1024];
    memcpy(temp2,tmp1,len);
    //////////////////////////////
    myPtf("解密后2 ucdel=%d,len=%d:",ucdel,len);
    for (i = 0; i < len; i++) {
        myPtf("%02X", *(temp2 + i));
    }
    myPtf("\n");
    ////////////////////////////////////////////////
    /////////////////////////////
    //dBASE64
    memset(tmp,0,1024);
    len=base64_decode(temp2, tmp);
    myPtf("解密后dbase64后:%s\n",tmp);
    memset(tmp1,0,1024);
    memcpy(tmp1,tmp,len);
    /////////////////////////////

    myPtf("明文数据: ");
    for (i = 0; i < len; i++) {
        myPtf("%02X", *(tmp1 + i));
    }
    myPtf("\n");
    myPtf("string:%s\n",tmp1);

    return 0;
}
