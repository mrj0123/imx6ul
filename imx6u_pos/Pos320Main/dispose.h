#ifndef DISPOSE_H
#define DISPOSE_H


/*
//网络参数，设置
#define NETWORK_INFOMATION_SET_CMD 1001  //0x1001
//网络参数，读取
#define NETWORK_INFOMATION_GET_CMD 2001  //0x2001

//无线wifi参数，设置
#define WIFI_SETTING_SET_CMD 1002 //0x1002
//无线wifi参数，读取
#define WIFI_SETTING_GET_CMD 2002 //0x2002
//获得硬件唯一识别码
#define HARDWARE_SERIALNO_GET_CMD 2003 //0x2003
//GPRS，开、关操作
#define GPRS_SWITCH_SET_CMD 1004 //0x1004
//GPRS，状态
#define GPRS_STATUS_GET_CMD 2004 //0x2004

//服务器参数，设置
#define SERVER_SETTING_SET_CMD 1005 //0x1005
//服务器参数，读取
#define SERVER_SETTING_GET_CMD 2005 //0x2005

//网络连接状态
#define NETWORK_CONNECT_STATUS_GET 1007 //0x1007



#define AF_UNIX_ERRORCOD_1001 -1
*/

void fn_dispose(int servfd);



#endif // DISPOSE_H
