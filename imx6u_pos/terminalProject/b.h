#ifndef B_H
#define B_H



typedef struct
{
    char serverip[16];//服务器IP
    int serverport=0;//服务器端口
}Server_Setting_t;

// extern Server_Setting_t g_server_setting[2];//两个服务器连接

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

//extern Wired_Setting_t g_wired_setting;

typedef struct
{
    char essidname[30];//无线ESSID名字
    char password[30];//无线登录密码
    int netstatus;//无线网卡启动状态
}Wireless_Setting_t;

//extern Wireless_Setting_t g_wireless_setting;
typedef struct
{
    int nettype;//1有线 2无线 3GPRS(4G)
    int status;//4G状态 0未启用 1已启用
    char ipaddress[16];//ip地址
}GPRS_Setting_t;




#endif // B_H
