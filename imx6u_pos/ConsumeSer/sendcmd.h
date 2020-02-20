#ifndef SENDCMD_H
#define SENDCMD_H

#include "a.h"

typedef struct client_unsocket{
    char clipath[108];
    char serpath[108];
    int  clisockfd;
}client_unsocket_t;


//客户端初始化链接
int conn_afunix(char cliPath[108]);
//客户端关闭链接
int close_afunix(int sockfd,char cliPath[108]);
//客户端重连
int reconn_afunix(int * psockfd,char cliPath[108]);
//发送命令
char * sendCmd_afunix(int sockfd,char cliPath[108], char serPath[108], int *pCmd, char * pBuf, int bufLen);

#endif // SENDCMD_H
