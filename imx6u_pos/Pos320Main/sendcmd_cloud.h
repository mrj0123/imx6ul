#ifndef SENDCMD_CLOUD_H
#define SENDCMD_CLOUD_H

#include "a.h"
#include "nksocket.h"

#define MAXSLEEP 10000

//客户端初始化链接
int init_client_socket(ser_conn_t server_conn[MAX_SERVER_NUM]);
//客户端关闭链接
int close_client_socket();
//发送命令
char * sendCmd_socket(int *pCmd, char * pBuf, int bufLen, int *retLen);
#endif // SENDCMD_CLOUD_H
