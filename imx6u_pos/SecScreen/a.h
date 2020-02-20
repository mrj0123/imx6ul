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
//主界面命令 0000~0999
#define FIRST_CMD   0001
#define SECOND_CMD  0002

//扫二维码命令字  1000~1999
#define SCANQR_BEGIN_CAPTURE_CMD    1001    // 开始采集图像
#define SCANQR_GET_QRCODE_CMD       1002    // 获取解码信息
#define SCANQR_STOP_CAPTURE_CMD     1003    // 停止采集图像
#define SCANQR_STOP_QRCODE_CMD      1004    // 取消获取解码信息

//副屏显示命令字  2000~2999
#define SECSCREEN_SHOWPIC_CMD       2001    // 显示底图（只有底图，没有文字）
#define SECSCREEN_SHOWALL_CMD       2002    // 显示背景和文字（有底图，也有文字）
#define SECSCREEN_APPENDTXT_CMD     2003    // 在原来显示的内容上追加文字
#define SECSCREEN_GETVERSION_CMD    2004    // 获取版本号

//命令正常返回为命令字+10000
#define FIRST_RET   10001
#define SECOND_RET  10002
//1.0.7增加afunix通讯包头的时间戳
#define SECSCREENVERSION "2.0.1"
///////////////////////////////////////////////////////////////////

#define SECSCREEN_PATH_C "/usr/local/nkty/secscreen_c"
#define SECSCREEN_PATH_S "/usr/local/nkty/secscreen_s"

#define SECSCREEN_BG_PIC    "/usr/local/nkty/000.jpg"
#define SECSCREEN_FONT      "/usr/local/nkty/msyh.ttf"
#define SECSCREEN_TXT_POSX  100
#define SECSCREEN_TXT_POSY  140
#define SECSCREEN_FONTSIZE  40

//#define __DEBUG__
#ifdef __DEBUG__
#define myPtf(format, ...) printf (format, ##__VA_ARGS__)
#define myFullPtf(format,...) printf("FILE: "__FILE__", LINE: %d: "format"/n", __LINE__, ##__VA_ARGS__)
#else
#define myPtf(format,...)
#define myFullPtf(format,...)
#endif

//typedef unsigned char u8;
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
