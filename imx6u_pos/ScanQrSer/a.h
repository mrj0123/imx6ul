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
//#define SCANQR_GET_VERSION_CMD      1005    // 获取版本信息
#define SCANQR_GET_VERSION_CMD      9    // 获取版本信息


//副屏显示命令字  2000~2999
#define SECSCREEN_SHOWCARD_CMD      2001    // 显示:请刷卡
#define SECSCREEN_SHOWQR_CMD        2002    // 显示:请扫码
#define SECSCREEN_CONSUMEMONEY_CMD  2003    // 显示:输入金额XXX
#define SECSCREEN_CONSUMEOK_CMD     2004    // 显示:消费成功：余额XXX（剩余次数XXX）
#define SECSCREEN_CONSUMEERR_CMD    2004    // 显示:消费失败  XXX（原因）
#define SECSCREEN_WAIT_CMD          2005    // 显示:请等待
#define SECSCREEN_WELCOME_CMD       2006    // 显示:欢迎光临

//命令正常返回为命令字+10000
#define FIRST_RET   10001
#define SECOND_RET  10002

#define SCANTHREAD_STATE_NOWORK     0       //无任务，且上一次扫码结果已被取走
#define SCANTHREAD_STATE_WORKING    1       //有任务，且未读到码，循环读取二维码
#define SCANTHREAD_STATE_WORKOVER   2       //任务已完成，但结果未被取走，不再读二维吗，等待取走结果


//1.0.23 增加获取版本号
//1.0.25 增加唯一启动识别
//1.0.26 增加afunix通讯包头的时间戳
//1.0.27 修改扫码线程从每次得到请求都启动改为初始化启动，没有请求sleep，有请求再工作，而不关闭线程
#define SCANQRSERVERSION "2.0.1"

///////////////////////////////////////////////////////////////////

#define SCANQR_PATH_C "/usr/local/nkty/scanqr_c"
#define SCANQR_PATH_S "/usr/local/nkty/scanqr_s"


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
