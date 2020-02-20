#ifndef DEALCMD_H
#define DEALCMD_H

#include <stddef.h>


//命令处理函数，获得硬件序列号
char * fn_dealCmd_GetHardwareSerialNo(char * recv_buf,int recv_len,int * ret_len);

//命令处理函数，获得网络连接状态
char * fn_dealCmd_GetNetworkConnectStatus(char * recv_buf,int recv_len,int * ret_len);


//命令处理函数，设置网络参数
char * fn_dealCmd_SetNetWorkSetting(char * recv_buf,int recv_len,int * ret_len);


//命令处理函数，获得网络参数
char * fn_dealCmd_GetNetWorkSetting(char * recv_buf,int recv_len,int * ret_len);



//命令处理函数，设置无线wifi参数
char * fn_dealCmd_SetWifiSetting(char * recv_buf,int recv_len,int * ret_len);


//命令处理函数，获得无线wifi参数
char * fn_dealCmd_GetWifiSetting(char * recv_buf,int recv_len,int * ret_len);


//命令处理函数，设置GPRS参数
char * fn_dealCmd_SetGPRSSetting(char * recv_buf,int recv_len,int * ret_len);


//命令处理函数，获得GPRS参数
char * fn_dealCmd_GetGPRSSetting(char * recv_buf,int recv_len,int * ret_len);


//命令处理函数，设置服务器参数
char * fn_dealCmd_SetServerSetting(char * recv_buf,int recv_len,int * ret_len);


//命令处理函数，获得服务器参数
char * fn_dealCmd_GetServerSetting(char * recv_buf,int recv_len,int * ret_len);

//命令处理函数，获得运行版本号
char * fn_dealCmd_GetVersion(char * recv_buf,int recv_len,int * ret_len);

//获得网络状态
int GetNetStat( char *devicename);


//命令处理函数，获得无线wifi状态
char * fn_dealCmd_GetWifiStatus(char * recv_buf,int recv_len,int * ret_len);

//命令处理命令
char * fn_dealCmd_UpdateAllProcessStatus(char * recv_buf,int recv_len,int * ret_len);
//命令处理命令 保存所有参数
char * fn_dealCmd_SAVEALL_SETTING(char * recv_buf,int recv_len,int * ret_len);

#endif
