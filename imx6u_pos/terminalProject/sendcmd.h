#ifndef SENDCMD_H
#define SENDCMD_H


#include <QString>
#include <QJsonObject>
#include "afunix_udp.h"

#define AF_UNIX_ERRORCOD_1001 -1001 //错误码定义，以后有具体含义应修改
#define AF_UNIX_ERRORCOD_1002 -1002

#define FIRST_CMD 1
//#define FIRST_RET 10001
#define SECOND_CMD 2
//#define SECOND_RET 10002

//版本号////////////////////////////////////////////////////////
#define UI_GET_VERSION              8       // 获取消费服务版本号，consumeser服务
#define UI_GET_SCANQRVERSION        9       // 获取扫码服务版本号
#define SECSCREEN_GETVERSION_CMD    2004    // 获取版本号,secscreen服务

/*//主界面命令 0000~0999
#define UI_GET_TERMINFO             0001    // 初始化界面信息
#define UI_GET_CONSUMEINFO          0002    // 获取消费总金额，消费总次数
#define UI_REQ_CONSUME              0003    // 请求消费
#define UI_QUE_CONSUMERET           0004    // 查询消费结果
#define UI_CANCEL_CONSUME           0005    // 取消消费
#define UI_GET_FLOWTOTAL            0006    // 获取流水总金额和总条数
#define UI_GET_FLOWINFO             0007    // 获取流水明细

#define ECHO_CMD                    6666    // 特殊命令字，返回输入JSON串和输入命令字，调试和测接口时使用*/

#define ERRORCODE -1



//二维码解析结果
#define QR_DECODE_SUCCESS           0       //解码成功，且符合规则，允许继续消费步骤
#define QR_DECODE_ERROR             -2001   //二维码解码失败
#define QR_DECODE_TIMEOUT           -2002   //超时，返回给UI进行处理，UI可在报警后自动发起下一次消费请求
#define QR_DECODE_FACTORYERR        -2003   //厂商识别号错误，更可能是密码错误
#define NO_CONSUME_REQ              -2004   //本次消费无结果或未收到消费请求
#define QR_DECODE_MERCHANTID        -2005   //商户编号错误 wang mao tan 2019-07-25
//云端返回错误
#define CLD_SEND_ERROR              -1      //云服务发送失败
#define CLD_RECV_ERROR              -2      //云服务接收失败

//交易结果ret
#define CONSUME_SUCCESS 10  //交易成功
#define CONSUME_ERROR 20  //消费失败
#define CONSUME_NOT_FUNDS 30  //余额不足
#define CONSUME_NOT_ACCOUNTS 40  //账户不存在
#define CONSUME_NEED_PASSWORD 50  //需要输入密码
#define CONSUME_FROZEN_LOSS  60 //冻结或挂失

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

//获得wifi状态
#define WIFI_STATUS_GET 5002

//网络连接状态
#define NETWORK_CONNECT_STATUS_GET 3007

//运行版本号
#define RUNNING_VERSION_GET 3008

//更新各个进程状态
#define ALL_PROCESS_STATUS_UPDATE 6001

//设置参数 保存全部设置参数
#define SETTING_SAVEALL_SET 3009

#define GATE_PATH_S "/usr/local/nkty/gate_s" //服务器端，启动服务时使用s_init_net//"/home/root/nkty/gate_s";
#define GATE_PATH_C "/usr/local/nkty/gate_c" //客户端 // "/home/root/nkty/gate_c"


//副屏显示命令字  2000~2999
#define SECSCREEN_SHOWPIC_CMD       2001    // 显示底图（只有底图，没有文字）
#define SECSCREEN_SHOWALL_CMD       2002    // 显示背景和文字（有底图，也有文字）
#define SECSCREEN_APPENDTXT_CMD     2003    // 在原来显示的内容上追加文字

#define UIVERSION "2.0.1"  //UI程序版本号 wang mao tan changed 1.0.110 2019-07-25
#define ERRORNUM  5         //consumeser服务版本号请求次数
#define MAXMONEY 9999       //出纳模式时，存款上限

#define FSTSCREEN   101   //主屏ID
#define SECSCREEN   102   //副屏ID
#define MAXTIMESTAMP    0x7fffffff   //最大时间戳
#define CLEARTIMESTAMP  24*60*60*10000  //清屏时间戳

#define GETSERVERSETTING    1
#define GETWIREDSETTING     2
#define GETWIRELESSSETTING  3
#define GETGPRSSETTING  4

#define UI_REQ_OFFLINECONSUME       13  //脱机消费
#define UI_REQ_UPLOADFLOW           14  //上传脱机流水
#define UI_GET_OFFLINEFLOWNUM       15  //获取尚未上传的脱机流水条数
#define OFFLINEMAXMONEY             100 //脱机消费时，最大消费金额上限
#define UI_REQ_DOWNLOADUSERLIST     16      //下载用于列表
#define UI_GET_USERLISTNUM          17      //获取已下载列表包数



#define SECSCREEN_FONT "./mgyh.ttf"  //引入中文字体

class sendCmd
{
public:
    sendCmd(char ser_path[108],char cli_path[108]);
    ~sendCmd();
    int conn_afunix();
    int close_afunix();
    QJsonObject send_Command(int * pCmd,QJsonObject jsonObj,char retJson[1024]);
    QJsonObject send_Command_NoRecv(int * pCmd,QJsonObject jsonObj,char retJson[1024]);
    //QJsonObject send_Command(int * pCmd,QJsonObject jsonObj);

private:
    int sockfd;
    struct sockaddr_un ser_addr;
    socklen_t ser_addrlen;
    char serPath[108];
    char cliPath[108];
    int sendnum;
    pthread_mutex_t send_mutex;
    QJsonObject s_Cmd(int * pCmd,QJsonObject jsonObj,char retJson[1024]);
};

#endif // SENDCMD_H
