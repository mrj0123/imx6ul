#ifndef  A_H
#define A_H

#include <arpa/inet.h>
#include <asm-generic/ioctls.h>
#include <errno.h>
#include <iconv.h>
#include <fcntl.h>

#include <linux/input.h>
#include <linux/serial.h>

#include <netinet/in.h>


#include <openssl/des.h>

#include <signal.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <string.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/un.h>

#include <time.h>
#include <termio.h>
#include <termios.h>

#include <pthread.h>
#include <poll.h>
#include <unistd.h>

#include "common.h"
#include "cJSON.h"

#include "afunix_udp.h"

#define PKCS7 7
#define PKCS5 5
#define NOPADDING 0
#define MAXLINE 8000

///////////////////////////////////////////////////////////////////
//错误码：
#define AF_UNIX_ERRORCOD_1001 -1001 //错误码定义，以后有具体含义应修改
#define AF_UNIX_ERRORCOD_1002 -1002
///////////////////////////////////////////////////////////////////
//命令字:(发送命令字和返回命令字)
#define FIRST_CMD 1
#define FIRST_RET 10001
#define SECOND_CMD 2
#define SECOND_RET 10002

//扫二维码命令字
#define SCANQR_INIT_CMD
///////////////////////////////////////////////////////////////////

typedef unsigned char u8;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;

//读到卡号为1，读到二维码为2，读到键盘返回为3，什么都没读到为0

int g_StopRead;

char g_ServerIP[16];    //服务器地址
int  g_ServerPort;      //服务器端口

char g_SerialNo[20];     //序号
char g_Version[20];      //版本号
char g_TermID[20];       //终端号
char g_AreaID[20];       //当前区号


//用户消费返回结构
struct ConsumeRet
{
   char username[20];
   char userid[20];
   int consumemoney;
   int remainmoney;
   int ret;
} consumeret;

#endif
