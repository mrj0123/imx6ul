#ifndef NETSETTINGVAR_H
#define NETSETTINGVAR_H
//参数
/*
#define GETSERVERSETTING    1
#define GETWIREDSETTING     2
#define GETWIRELESSSETTING  3



typedef struct
{
    char serverip[16];//服务器IP
    int serverport;//服务器端口
}Server_Setting_t;

Server_Setting_t g_server_setting[2];//两个服务器连接

typedef struct
{
    char ipaddress[16];//ip地址
    char netmask[16];//子网掩码
    char gateway[16];//网关
    char mac[30];//mac地址
    int netstatus;//网络状态
    int dhcpflag;//IP地址获得方式 0静态IP,1动态分配
    char dns1[20];
    char dns2[20];
}Wired_Setting_t;

Wired_Setting_t g_wired_setting;

typedef struct
{
    char essidname[30];//无线ESSID名字
    char password[30];//无线登录密码
    int netstatus;//无线网卡启动状态
}Wireless_Setting_t;

Wireless_Setting_t g_wireless_setting;
*/
#endif // NETSETTINGVAR_H
