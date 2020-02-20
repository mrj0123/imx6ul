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

#include <stdio.h>
#include <sys/time.h>

#define PKCS7 7
#define PKCS5 5
#define NOPADDING 0
#define MAXLINE 10000
//1.014已完成终端信息获取，解密并发送给界面成功
//1.016完成取流水数量金额 流水明细的接口
//1.017完成同步时间
//1.019联调版
//1.021改读取JSON数据类型
//1.0.22读nktyconfig配置文件，内部通讯路径
//1.0.23增加读版本号接口
//1.0.24修改配置文件路径
//1.0.25稳定版1
//1.0.26修改printf为宏myPtf，release不输出
//1.0.27增加卡自检接口
//1.0.28增加防止重复启动判断
//1.0.31取消对副屏的连接调用
//1.0.32修改扫码服务版本返回值
//1.0.34增加扫码读取二维码内容
//1.0.35增加afunix通讯包头的时间戳
//1.0.36将心跳包单独拿出来作为独立线程处理
//1.0.37修改扫码读取二维码内容时间戳错误问题
//1.0.38增加交易类型扫码或读卡
//1.0.39扫码或读卡间隙，增加usleep，让系统处理界面请求
//1.0.40如果出现消费线程关闭，但状态不是FINISH的错误时，将状态改为FINISH
//1.0.41增加了调用ScanQrSer时的互斥afunix_mutex
//1.0.42将消费线程和扫码线程从每次得到请求都启动改为初始化启动，没有请求sleep，有请求再工作，而不关闭线程。
//1.0.43如果消费请求是相同时间戳，直接返回时间戳错误，不再等待
//1.0.44修改断网时系统不提示的问题
//1.0.45增加定时启动POSMain320
//1.0.46增加socket连接超时判断，将连接改非阻塞
//1.0.47增加外接二维码扫描枪支持
//1.0.48修改联网策略，连续4次连接不成功才算失败
//1.0.49修改外接扫码枪和外接键盘混用识别错误的问题(_test_pwd是测试密码版)
//1.0.50增加外接扫描枪热拔插支持
//1.0.51修改二维码格式，取消8位厂商编码
//1.0.52增加指定流水退款功能
//1.0.53在上传流水中增加消费发起时间（发起调用云服务时间，也可理解为刷卡、扫码或出纳点击确认时间）
//1.0.54修改外置扫码枪扫码设置终端配置的问题
//1.0.55增加消费状态改变回调机制，UI端需做相应改变后提高效率
//1.0.56支持微信支付宝等第三方支付方式
//1.0.57增加定时同步服务器时间
//1.0.58增加脱机刷卡暂存流水功能
//1.0.59增加脱机流水备份和回读校验的功能
//1.0.60是否使用内置摄像头识别二维码，在配置文件中增加配置项区别
//1.0.61适配北京航食4字节SN卡号要求。现在改为只有广分使用3字节卡号，其余均使用4字节卡号
//1.0.62调试时间走慢的问题，增加日志，同时修改同步时间每10分钟同步一次
//1.0.63增加青岛分行模式，使用4字节序列号+广分扫码模式。
//1.0.64修改脱机消费要求，增加扫码消费支持，以及下载名单处理
//1.0.65增加科技馆模式，可以使用内部码，也可以使用外部码，还可以刷卡
//1.0.66科技馆模式改为48字节二维码，包括4字节厂商编码
//1.0.67读卡读取卡码将高低位颠倒
//1.0.68修改读卡器buf[3]字节识别卡片类型的方法，原方法读卡终端按协议判断卡类型方式有误，现改为按照用户代码来处理字节顺序
//1.0.69广东分行新规则：8位商户编号+24位卡号（前补0）+14位时间+00
#define CONSUMESERVERSION "2.0.1"

///////////////////////////////////////////////////////////////////
#define USERTYPE_BOC_HQ             1001       //中国银行总行,刷卡使用3字节SN（HID卡？？暂未使用），可以刷（加密后）64位APP付款码，也可刷18位第三方付款码
#define USERTYPE_BOC_GZ             1002       //中国银行广东分行，刷卡使用3字节SN（M1卡），二维码编码采用（加密后）52位付款码，无商户编号
#define USERTYPE_BOC_QD             1003       //中国银行青岛分行，刷卡4字节SN，同时使用类似广东分行的解码方式
#define USERTYPE_EDU_NKBH           2001       //南开大学滨海学院，只能刷18位第三方付款码和M1卡（1扇区卡号）
#define USERTYPE_BJHS               2002       //北京航食只能刷M1卡4字节SN
#define USERTYPE_BJKJG              2003       //北京科技馆模式，可以扫二维码，可以读SN，可以

#define USETTP_DISABLE              0           //禁止使用第三方支付码二维码支付
#define USETTP_ENABLE               1           //允许使用第三方支付码二维码支付
//错误码：
#define ERRORCOD_NOCMD              -1001   //命令字不存在
#define ERRORCOD_1002               -1002
#define JSON_ERROR                  -1003   //JSON串错误
#define JSON_NOITEM_ERROR           -1004   //JSON中缺少元素
#define RET_JSON_ERROR              -1006   //返回JSON串错误
#define JSON_ADDCMD_ERROR           -1007   //JSON串前加CMD字时错误

#define CLOUD_SENDCMD_ERROR         -1008   //调用云服务时错误

#define NORECVBUF_ERROR             -1009   //命令需要包身，但未收到包身错误
#define CREATE_CONSUMETHREAD_ERROR  -1010   //启动消费线程失败
#define TIMESTAMP_ERROR             -1011   //时间戳错误
#define CREATE_SCANQRTHREAD_ERROR   -1012   //启动扫码线程失败
///////////////////////////////////////////////////////////////////
//命令字:(发送命令字和返回命令字)
//云端命令字
#define CLD_CONSUME_CMD             3001    //消费命令字（出纳混用该命令字）
#define CLD_GETTERMINFO_CMD         3003    //获取终端信息
#define CLD_HEARTBEAT_CMD           3005    //心跳包
#define CLD_GETFLOWD_CMD            3006    //获取流水明细
#define CLD_GETFLOWT_CMD            3007    //查询流水总数
#define CLD_GETTIME_CMD             3008    //获取服务器时间
#define CLD_GETCONSUMERET_CMD       3009    //获取消费结果
#define CLD_GETACCOINFO_CMD         3010    //获取账户信息
#define CLD_GETTERMUPINFO_CMD       3011    //查询升级信息，本程序不使用，POSMAIN320使用该命令字
#define CLD_CANCELFLOW_CMD          3012    //退款操作
#define CLD_UPLOADFLOW_CMD          3013    //上传脱机流水
#define CLD_GETACCOLIST_CMD         3014    //批量获取账户名单，用于脱机消费

//主界面命令 0000~0999
#define UI_GET_TERMINFO             1       // 初始化界面信息_______________________预存，后取
#define UI_GET_CONSUMEINFO          2       // 获取消费总金额，消费总次数___________写0
#define UI_REQ_CONSUME              3       // 请求消费_____________________________存储，等待联网发送
#define UI_QUE_CONSUMERET           4       // 查询消费结果_________________________
#define UI_CANCEL_CONSUME           5       // 取消消费XXXXXXXXXXXXXXXXXXXXXXXXXXXXX
#define UI_GET_FLOWTOTAL            6       // 获取流水总金额和总条数XXXXXXXXXXXXXXX
#define UI_GET_FLOWINFO             7       // 获取流水明细XXXXXXXXXXXXXXXXXXXXXXXXX
#define UI_GET_VERSION              8       // 获取消费服务版本号___________________
#define UI_GET_SCANQRVERSION        9       // 获取扫码服务版本号___________________
#define UI_REQ_ACCOUNTINFO          10      // 获取账户信息XXXXXXXXXXXXXXXXXXXXXXXXX
#define UI_REQ_SCANQRCODE           11      // 申请扫描二维码_______________________
#define UI_EXEC_CANCELFLOW          12      // 执行取消流水操作XXXXXXXXXXXXXXXXXXXXX
#define UI_REQ_OFFLINECONSUME       13      //脱机消费
#define UI_REQ_UPLOADFLOW           14      //上传脱机流水
#define UI_GET_OFFLINEFLOWNUM       15      //获取尚未上传的脱机流水条数
#define UI_REQ_DOWNLOADUSERLIST     16      //下载用户列表
#define UI_GET_USERLISTNUM          17      //获取已下载列表包数 UI_GET_USERLISTNUM 返回 {"UserPackCount":10,"state":1}，UserPackCount>0 显示第几包，=0表示下载结束，<0表示错误
//消费回调接口
#define CONSUME_CALLBACK_QUERET     6001    //获得消费结果以后，回调通知UI程序

#define ECHO_CMD                    6666    // 特殊命令字，返回输入JSON串和输入命令字，调试和测接口时使用

//扫二维码命令字  1000~1999
#define SCANQR_BEGIN_CAPTURE_CMD    1001    // 开始采集图像
#define SCANQR_GET_QRCODE_CMD       1002    // 获取解码信息
#define SCANQR_STOP_CAPTURE_CMD     1003    // 停止采集图像
#define SCANQR_STOP_QRCODE_CMD      1004    // 停止获取解码信息
#define SCANQR_GET_VERSION_CMD      1005    // 获取版本信息

//二维码解析结果
#define QR_DECODE_SUCCESS           0       //解码成功，且符合规则，允许继续消费步骤
#define QR_DECODE_ERROR             -2001   //二维码解码失败
#define QR_DECODE_TIMEOUT           -2002   //超时，返回给UI进行处理，UI可在报警后自动发起下一次消费请求
#define QR_DECODE_FACTORYERR        -2003   //厂商识别号错误，更可能是密码错误
#define QR_DECODE_MERCHANTID        -2005   //商户编号不一致 wang mao tan 2019-07-25 added

#define NO_CONSUME_REQ              -2004   //本次消费无结果或未收到消费请求
//云端命令发送结果
#define CLD_SEND_ERROR              -1      //云服务发送失败
#define CLD_RECV_ERROR              -2      //云服务接收失败

//消费流程中的4种状态
#define CONSUME_SLEEP               0       //无消费状态
#define CONSUME_WAITFORCARD         1       //有新的消费请求，等待读卡或扫码
#define CONSUME_HAVECARDNO          2       //已读到卡号，但还未提交云端
#define CONSUME_WAITFORCLOUD        3       //已提交云端，但云端未返回消费结果
#define CONSUME_FINISH              4       //已返回结果，等待取回结果（可以考虑在取回结果后置0）


//副屏显示命令字  2000~2999
#define SECSCREEN_SHOWCARD_CMD      2001    // 显示:请刷卡
#define SECSCREEN_SHOWQR_CMD        2002    // 显示:请扫码
#define SECSCREEN_CONSUMEMONEY_CMD  2003    // 显示:输入金额XXX
#define SECSCREEN_CONSUMEOK_CMD     2004    // 显示:消费成功：余额XXX（剩余次数XXX）
#define SECSCREEN_CONSUMEERR_CMD    2004    // 显示:消费失败  XXX（原因）
#define SECSCREEN_WAIT_CMD          2005    // 显示:请等待
#define SECSCREEN_WELCOME_CMD       2006    // 显示:欢迎光临

//交易类型
#define TRSACTTYPE_READCARD         1       // 仅读卡
#define TRSACTTYPE_SCANQRCODE       2       // 仅扫码
#define TRSACTTYPE_READANDSCAN      3       // 读卡及扫码
#define TRSACTTYPE_SCANQRANDRET     4       // 读二维码直接返回，不交易


//命令正常返回为命令字+10000
#define FIRST_RET   10001
#define SECOND_RET  10002

//单笔离线流水JSON串大小
#define OFFLINEFLOW_JSONSIZE 108
///////////////////////////////////////////////////////////////////

#define SCANQR_PATH_C "/usr/local/nkty/scanqr_c"
#define SCANQR_PATH_S "/usr/local/nkty/scanqr_s"

#define SECSCREEN_PATH_C "/usr/local/nkty/secscreen_c"
#define SECSCREEN_PATH_S "/usr/local/nkty/secscreen_s"

#define CONSUMEBIZ_PATH_S "/usr/local/nkty/consumebiz_s"
#define CONSUMEBIZ_PATH_C "/usr/local/nkty/consumebiz_c"

#define CONFIG_PATH "/usr/local/nkty/script/nktyserver.conf"

#define CONSUMECALLBACK_PATH_C "/usr/local/nkty/consumecallback_c"
#define CONSUMECALLBACK_PATH_S "/usr/local/nkty/consumecallback_s"

//#define __DEBUG__
#ifdef __DEBUG__
#define myPtf(format, ...) printf (format, ##__VA_ARGS__)
#define myFullPtf(format,...) printf("FILE: "__FILE__", LINE: %d: "format"/n", __LINE__, ##__VA_ARGS__)
#else
#define myPtf(format,...)
#define myFullPtf(format,...)
#endif


typedef unsigned char u8;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;

//读到卡号为1，读到二维码为2，读到键盘返回为3，什么都没读到为0

//用户消费返回结构
struct ConsumeRet
{
   char username[20];
   char userid[20];
   int consumemoney;
   int remainmoney;
   int ret;
} consumeret;
//脱机流水
typedef struct _OffLine_Flow
{
   int timeStamp;   //本地时间戳，由UI发起
   int curTime;     //存储时间戳，记录刷卡时间，0.01ms为单位
   int accountId;   //账号
   int cardNo;      //卡号
   int flowMoney;   //金额
} offline_flow_t;

//脱机账户
typedef struct _User_List
{
   int accountId;   //账号
   int cardNo;      //卡号
} userlist_t;

//脱机参数
typedef struct _Save_Config
{
    int UseTermType;    //用户分类代码
    int FixedMondy;     //固定金额
    char termCode[52];  //终端唯一识别码
} config_t;
#endif
