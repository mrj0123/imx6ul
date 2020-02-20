#ifndef COMMON_H
#define COMMON_H
#include "sendcmd.h"


#define DEV_BUS         0
#define DEV_VENDOR      1
#define DEV_PORDUCT     2
#define DEV_VERSION     3
#define DEV_NAME        4
#define DEV_PHYS        5
#define DEV_SYSFS       6
#define DEV_HANDLERS    7

#define DEV_ITEMLEN     8
#define DEV_INFOLEN     256

typedef struct
{
    QString netState;
    QString networkType;
    QString powerrange;
    QString serverState;
}networkStruct;

class common
{
public:
    common();
    sendCmd * scmd_network_common;
    networkStruct * networkStruct_common;


    void getNetworkConnet();//网络连接状态--初始化函数
};

//获取当天的当前时间（不包括日期）的时间戳，精确到0.1毫秒
int getNowTime();

char * GetKBList(int * pKBNumber);

#endif // COMMON_H
