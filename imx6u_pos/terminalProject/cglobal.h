#ifndef CGLOBAL_H
#define CGLOBAL_H
#include "b.h"

class CGlobal
{
public:
    CGlobal();
    ~CGlobal();

public:
    static  Server_Setting_t g_server_setting[2];//两个服务器连接
    static   Wired_Setting_t g_wired_setting;
    static    Wireless_Setting_t g_wireless_setting;
    static GPRS_Setting_t g_gprs_setting;
};

#endif // CGLOBAL_H
