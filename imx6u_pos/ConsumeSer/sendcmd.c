#include "sendcmd.h"
#include "afunix_udp.h"
#include <pthread.h>

static pthread_mutex_t afunix_mutex;

//客户端建立链接
int conn_afunix(char cliPath[108])
{
    //初始化通讯互斥
    pthread_mutex_init(&afunix_mutex,NULL);
    return c_init_net(cliPath);
}

//客户端关闭链接
int close_afunix(int sockfd,char cliPath[108])
{
    if (sockfd != 0)
    {
        close_net(sockfd,cliPath);
        sockfd = 0;
        //释放互斥变量
        pthread_mutex_destroy(&afunix_mutex);
    }
    return 0;
}

//客户端重连
int reconn_afunix(int * psockfd,char cliPath[108])
{
    if (psockfd==NULL)
        return -1;
    if (*psockfd!=0)
    {
        close_net(*psockfd,cliPath);
    }
    *psockfd = c_init_net(cliPath);
    return 0;
}

//发送命令
char * sendCmd_afunix(int sockfd,char cliPath[108], char serPath[108], int *pCmd, char * pBuf, int bufLen)
{

    if (sockfd == 0)
    {
        *pCmd = ERRORCOD_1002;//发送失败
        return NULL;
    }
    char * send_data = pBuf;
    int cmd = *pCmd;
    time_t timep;
    time (&timep);
    myPtf("sockfd=%d,serPath=%s:sendCmd_afunix start\n",sockfd,serPath);
    //发送
    int tsp = getCurrentTime();
    //加锁
    pthread_mutex_lock(&afunix_mutex);
    int ret = c_send_packet(sockfd,cmd,tsp,(u8 *)send_data,bufLen,serPath);
    if (ret<=0)
    {
        pthread_mutex_unlock(&afunix_mutex);
        //尝试重连，然后退出
        reconn_afunix(&sockfd,cliPath);
        *pCmd = ERRORCOD_1002;//发送失败
        myPtf("sockfd=%d,serPath=%s:sendCmd_afunix error to reconnect！\n",sockfd,serPath);
        return NULL;
    }
    //接收（暂未考虑超时。超时应在上一级考虑）
    //接收数据
    char * recv_data = NULL;
    //接收数据的包头
    cmd_head_t recv_head;
    memset(&recv_head,0,sizeof(cmd_head_t));
    //定义服务器地址，用于recv(其实后面也不用)
    struct sockaddr_un ser_addr;
    socklen_t ser_addrlen;
    //////////////////////////////////////////////
    ser_addr.sun_family = AF_UNIX;
    strcpy(ser_addr.sun_path,serPath);
    ser_addrlen = sizeof(ser_addr);
    //////////////////////////////////////////////
    myPtf("sockfd=%d,serPath=%s:sendCmd_afunix start recv！\n",sockfd,serPath);
    time (&timep);
    myPtf("%s:sendCmd_afunix start recv\n",ctime(&timep));
    recv_data = recv_packet(&recv_head,sockfd,&ser_addr,&ser_addrlen);
    time (&timep);
    myPtf("%s:sendCmd_afunix ret\n",ctime(&timep));
    /////////////////////////////////////
    //如果收到非本次发出的命令，丢弃该命令，再接收一次
    if(recv_head.tsp != tsp)
    {
        myPtf("sockfd=%d,serPath=%s:sendCmd_afunix tsp error recv_head.tsp=%d,tsp=%d！\n",sockfd,serPath,recv_head.tsp,tsp);
        if (recv_data != NULL)
            free(recv_data);
        memset(&recv_head,0,sizeof(cmd_head_t));
        recv_data = recv_packet(&recv_head,sockfd,&ser_addr,&ser_addrlen);
    }
    myPtf("sockfd=%d,serPath=%s:sendCmd_afunix recv over！\n",sockfd,serPath);
    //解锁
    pthread_mutex_unlock(&afunix_mutex);
    /////////////////////////////////////
    if ((recv_data==NULL)||(recv_head.length==0))
    {
        recv_data = NULL;
        myPtf("sockfd=%d,serPath=%s:sendCmd_afunix recv no parket,recv_cmd=%d!\n",sockfd,serPath,recv_head.cmd);
        if(recv_head.cmd==0)
        {
            //尝试重连，然后退出
            reconn_afunix(&sockfd,cliPath);
            *pCmd = ERRORCOD_1002;//接收应答失败
            return NULL;
        }
    }
    //收到了返回数据
    *pCmd = recv_head.cmd;
    time (&timep);
    myPtf("sockfd=%d,serPath=%s:sendCmd_afunix over:%s\n",sockfd,serPath,ctime(&timep));
    return recv_data;
}
