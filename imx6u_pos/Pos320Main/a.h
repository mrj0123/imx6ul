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
#define MAXLINE 16384

///////////////////////////////////////////////////////////////////
//错误码：
#define AF_UNIX_ERRORCOD_1001 -1001 //错误码定义，以后有具体含义应修改
#define AF_UNIX_ERRORCOD_1002 -1002
///////////////////////////////////////////////////////////////////
//命令字:(发送命令字和返回命令字)
//主界面命令 0000~0999
#define FIRST_CMD   0001
#define SECOND_CMD  0002

//网络参数，设置
#define NETWORK_INFOMATION_SET_CMD 3001
//网络参数，读取
#define NETWORK_INFOMATION_GET_CMD 4001

//无线wifi参数，设置
#define WIFI_SETTING_SET_CMD 3002
//无线wifi参数，读取
#define WIFI_SETTING_GET_CMD 4002
//获得硬件唯一识别码
#define HARDWARE_SERIALNO_GET_CMD 3003
//GPRS，开、关操作
#define GPRS_SWITCH_SET_CMD 3004
//GPRS，状态
#define GPRS_STATUS_GET_CMD 4004

//服务器参数，设置
#define SERVER_SETTING_SET_CMD 3005
//服务器参数，读取
#define SERVER_SETTING_GET_CMD 4005

//网络连接状态
#define NETWORK_CONNECT_STATUS_GET 3007


//运行版本号
#define RUNNING_VERSION_GET 3008

//设置参数 保存全部设置参数
#define SETTING_SAVEALL_SET 3009

//获得升级版本信息
#define UPDATE_INFO_GET 3011

//获得wifi状态
#define WIFI_STATUS_GET 5002

//更新各个进程状态
#define ALL_PROCESS_STATUS_UPDATE 6001
//1 判断几个进程是否存在
//2、判断几个进程版本号是否最新

#define POSMAIN_VERSION "2.0.1"
//程序是否需要重启的超时秒数
#define RESTART_DELAY 5

//调试状态
#define DEBUG_STATUS 0

//自动启动其他模块
#define AUTOSTART_OTHER 1


//自动升级
#define AUTOUPDATE_FLAG 1


//判断其他模块时间戳是否超时，如果超时，杀死进程
#define AUTOKILL_IFTIMEOUT 0

//允许使用wifi
#define CANUSEWIFI_FLAG 0

//允许使用GPRS(4G)
#define CANUSEGPRS_FLAG 0


#define USER_HOME_DIRECTORY "/usr/local/nkty"
#define USER_SCRIPT_DIRECTORY "/usr/local/nkty/script"
#define USER_TEMP_DIRECTORY "/usr/local/nkty/temp"
/*
//用户版本信息结构
typedef struct
{
   char pos320main[20];
   char consumeser[20];
   char scanqr[20];
   char secscreen[20];
   char terminalproject[20];
} Versionstruct ;

typedef Versionstruct *VerPointer;

VerPointer g_allver;


typedef struct
{
	char processname[50];
	char version[20];
	char updatetime[20];
	int flag;
}VersionWithTimeStruct;

//主程序版本信息数组
VersionWithTimeStruct g_versiontimelist[50];
*/

typedef struct structSetting{
	char buffer[30];
}setting_t;


//客户端发送命令字结构
typedef struct structCmd{

    	int cmd;//命令字，3011
	char termVersion[100];//终端版本号
	char termCode[100];//终端唯一识别码
} client_Cmd_t;





//服务器返回的终端升级信息
typedef struct struct_TermUpdateInfo{

	char md5[256];//MD5
	char downloadPath[256];//下载路径
	int upGrades;//下载模式，1 http 2 ftp
} client_term_updateinfo_t;


//服务器返回的升级信息
typedef struct struct_GetUpdateInfo{

	client_term_updateinfo_t upInfos[256];//终端升级信息
	int cmd;//命令字
	int ret;//返回值
	char msg[256];
	char ftpUser[100];//ftpUser
	char ftpPwd[100];//ftpPwd
} client_ret_updateinfo_t;


//键盘输入结构，2018-12-03
typedef struct keyboardStruct{
    	char bus[256];
	char vendor[256];
	char product[256];
	char version[256];
	char name[256];
	char phys[256];
	char sysfs[1024];
	char uniq[256];
	char handlers[256];
} kbd_list_t;
typedef kbd_list_t *kbdPointer;

kbdPointer g_kbd_list;

/*
//扫二维码命令字  1000~1999
#define SCANQR_BEGIN_CAPTURE_CMD    1001    // 开始采集图像
#define SCANQR_GET_QRCODE_CMD       1002    // 获取解码信息
#define SCANQR_STOP_CAPTURE_CMD     1003    // 停止采集图像
#define SCANQR_STOP_QRCODE_CMD      1004    // 取消获取解码信息

//副屏显示命令字  2000~2999
#define SECSCREEN_SHOWCARD_CMD      2001    // 显示:请刷卡
#define SECSCREEN_SHOWQR_CMD        2002    // 显示:请扫码
#define SECSCREEN_CONSUMEMONEY_CMD  2003    // 显示:输入金额XXX
#define SECSCREEN_CONSUMEOK_CMD     2004    // 显示:消费成功：余额XXX（剩余次数XXX）
#define SECSCREEN_CONSUMEERR_CMD    2004    // 显示:消费失败  XXX（原因）
#define SECSCREEN_WAIT_CMD          2005    // 显示:请等待
#define SECSCREEN_WELCOME_CMD       2006    // 显示:欢迎光临
*/
//命令正常返回为命令字+10000
#define FIRST_RET   10001
#define SECOND_RET  10002


///////////////////////////////////////////////////////////////////

#define GATE_PATH_S "/usr/local/nkty/gate_s" //服务器端，启动服务时使用s_init_net//"/home/root/nkty/gate_s";
#define GATE_PATH_C "/usr/local/nkty/gate_c" //客户端 // "/home/root/nkty/gate_c"


//typedef unsigned char u8;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;

#define MAX_PATH 260

//#define __DEBUG__
#ifdef __DEBUG__
#define myPtf(format, ...) printf (format, ##__VA_ARGS__)
#define myFullPtf(format,...) printf("FILE: "__FILE__", LINE: %d: "format"/n", __LINE__, ##__VA_ARGS__)
#else
#define myPtf(format,...)
#define myFullPtf(format,...)
#endif

//读到卡号为1，读到二维码为2，读到键盘返回为3，什么都没读到为0
/*
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
*/
#endif
