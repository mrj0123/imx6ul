
#include "cglobal.h"

CGlobal::CGlobal()
{
}
CGlobal::~CGlobal()
{
}
Server_Setting_t CGlobal::g_server_setting[2];//两个服务器连接
Wired_Setting_t CGlobal::g_wired_setting;
Wireless_Setting_t CGlobal::g_wireless_setting;
GPRS_Setting_t CGlobal::g_gprs_setting;
