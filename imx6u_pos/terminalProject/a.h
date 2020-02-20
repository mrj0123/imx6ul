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
//#include "afunix_udp.h"

#define PKCS7 7
#define PKCS5 5
#define NOPADDING 0
#define MAXLINE 8000
//1.014已完成终端信息获取，解密并发送给界面成功
//1.016完成取流水数量金额 流水明细的接口
//1.017完成同步时间
//
#define CONSUMESERVERSION "1.0.69" //wang mao tan changed to 1.0.69 2019-07-25
///////////////////////////////////////////////////////////////////
//错误码：
#define ERRORCOD_NOCMD              -1001 //命令字不存在
#define ERRORCOD_1002               -1002
#define JSON_ERROR                  -1003  //JSON串错误
#define JSON_NOITEM_ERROR           -1004   //JSON中缺少元素
#define RET_JSON_ERROR              -1006   //返回JSON串错误
#define JSON_ADDCMD_ERROR           -1007  //JSON串前加CMD字时错误

#define CLOUD_SENDCMD_ERROR         -1008   //调用云服务时错误

#define NORECVBUF_ERROR             -1009   //命令需要包身，但未收到包身错误
#define CREATE_CONSUMETHREAD_ERROR  -1010   //启动消费线程失败
#define TIMESTAMP_ERROR             -1011   //时间戳错误
///////////////////////////////////////////////////////////////////
//命令字:(发送命令字和返回命令字)
//云端命令字
#define CLD_GETTERMINFO_CMD         3003    //获取终端信息
#define CLD_CONSUME_CMD             3001    //消费命令字（出纳混用该命令字）
#define CLD_HEATBEAT_CMD            3005    //心跳包
#define CLD_GETTIME_CMD             3008    //获取服务器时间
#define CLD_GETFLOWT_CMD            3007    //查询流水总数
#define CLD_GETFLOWD_CMD            3006    //获取流水明细
//主界面命令 0000~0999
#define UI_GET_TERMINFO             0001    // 初始化界面信息
#define UI_GET_CONSUMEINFO          0002    // 获取消费总金额，消费总次数
#define UI_REQ_CONSUME              0003    // 请求消费
#define UI_REQ_ACCOUNTINFO          10      // 获取账户信息
#define UI_QUE_CONSUMERET           0004    // 查询消费结果
#define UI_CANCEL_CONSUME           0005    // 取消消费
#define UI_GET_FLOWTOTAL            0006    // 获取流水总金额和总条数
#define UI_GET_FLOWINFO             0007    // 获取流水明细
#define UI_REQ_SCANQRCODE           11      // 申请扫描二维码
#define UI_EXEC_CANCELFLOW          12      // 执行取消流水操作



#define ECHO_CMD                    6666    // 特殊命令字，返回输入JSON串和输入命令字，调试和测接口时使用

//扫二维码命令字  1000~1999
#define SCANQR_BEGIN_CAPTURE_CMD    1001    // 开始采集图像
#define SCANQR_GET_QRCODE_CMD       1002    // 获取解码信息
#define SCANQR_STOP_CAPTURE_CMD     1003    // 停止采集图像

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

///////////////////////////////////////////////////////////////////
#define SCANQR_PATH_C "/usr/local/nkty/scanqr_c"//scanqrser服务通讯路径
#define SCANQR_PATH_S "/usr/local/nkty/scanqr_s"//scanqrser服务通讯路径

#define SECSCREEN_PATH_C "/usr/local/nkty/secscreen_c"//secscreen服务通讯路径
#define SECSCREEN_PATH_S "/usr/local/nkty/secscreen_s"//secscreen服务通讯路径

#define CONSUMEBIZ_PATH_S "/usr/local/nkty/consumebiz_s" //consumeser服务通讯路径
#define CONSUMEBIZ_PATH_C "/usr/local/nkty/consumebiz_c"// consumeser服务通讯路径


//#define __DEBUG__
#ifdef __DEBUG__
#define myPtf(format, ...) printf (format, ##__VA_ARGS__)
#define myFullPtf(format,...) printf("FILE: " __FILE__ ", LINE: %d: " format "/n", __LINE__, ##__VA_ARGS__)
#else
#define myPtf(format,...)
#define myFullPtf(format,...)
#endif




typedef unsigned char u8;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;

//读到卡号为1，读到二维码为2，读到键盘返回为3，什么都没读到为0

#endif
