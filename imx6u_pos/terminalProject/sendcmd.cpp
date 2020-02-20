#include "sendcmd.h"
#include <QJsonDocument>
#include <QDateTime>

#include <stdio.h>
#include <sys/time.h>

//QString 转 char*
char * fn_QString2char(QString data)
{
    QByteArray* byte = new QByteArray(data.toLocal8Bit());
    char * charData = byte->data();
    char * charRet =(char *)malloc(strlen(charData)+1);
    memset(charRet,0,strlen(charData)+1);
    memcpy(charRet,charData,strlen(charData));
    delete byte;
    return charRet;
}


int getCurrentTime()
{
   struct timeval tv;
   gettimeofday(&tv,NULL);
   //myPtf("tv.tv_sec=%ld,tv.tv_usec=%ld\n",tv.tv_sec,tv.tv_usec);
   //return ((long long int)tv.tv_sec) * 1000 + ((long long int)tv.tv_usec) / 1000;
   return (tv.tv_sec%86400)*10000 + tv.tv_usec/100;
}

sendCmd::sendCmd(char ser_path[108],char cli_path[108])
{
    sockfd = 0;
    strcpy(serPath,ser_path);
    strcpy(cliPath,cli_path);
    //bzero(&ser_addr,sizeof(ser_addr));
    ser_addr.sun_family = AF_UNIX;
    strcpy(ser_addr.sun_path,ser_path);
    ser_addrlen = sizeof(ser_addr);
    sendnum = 0;
    pthread_mutex_init(&send_mutex,NULL);
}

sendCmd::~sendCmd()
{
    pthread_mutex_destroy(&send_mutex);
}

int sendCmd::conn_afunix()
{
    if (sockfd!=0)
    {
       close_net(sockfd,cliPath);
    }
    sockfd = c_init_net(cliPath);
    return 0;
}

int sendCmd::close_afunix()
{
    close_net(sockfd,cliPath);
    sockfd = 0;
    return 0;
}

QJsonObject sendCmd::send_Command_NoRecv(int * pCmd,QJsonObject jsonObj,char retJson[1024])
{
    sendnum++;
    QJsonObject recvJson;
    recvJson.empty();
    memset(retJson,0,1024);
    //QJsonObject转字符串
    char * send_data;
    int sendlen=0;
    int cmd = *pCmd;
    //先转成QJsonDocument
    if(jsonObj.isEmpty())
    {
        send_data=NULL;
    }
    else
    {
        QJsonDocument doc(jsonObj);
        //再转成QStringtrans_data
        QString strJson(doc.toJson(QJsonDocument::Compact));
        //最后转成char*
        send_data = fn_QString2char(strJson);
        sendlen = strlen(send_data);
    }
    //发送
    int tsp = getCurrentTime();
    int ret = c_send_packet(sockfd,cmd,tsp,(u8 *)send_data,sendlen,serPath);
    if (send_data != NULL)
        free(send_data);
    if (ret<=0)
    {
        //尝试重连，然后退出
        conn_afunix();
        *pCmd = AF_UNIX_ERRORCOD_1002;
        myPtf("send_packet error!!!!\n");
        return recvJson;
    }
    *pCmd = cmd + 10000;
    return recvJson;
}
QJsonObject sendCmd::send_Command(int * pCmd,QJsonObject jsonObj,char retJson[1024])
{
    QJsonObject ret;
    QDateTime time = QDateTime::currentDateTime();
    int tsp = time.toTime_t();//当前的时间戳
    myPtf("lock send_mutex in %d,path=%s\n",tsp,serPath);
    pthread_mutex_lock(&send_mutex);
    ret = s_Cmd(pCmd,jsonObj,retJson);
    pthread_mutex_unlock(&send_mutex);
    myPtf("unlock send_mutex in %d,path=%s\n",tsp,serPath);
    return ret;
}

QJsonObject sendCmd::s_Cmd(int * pCmd,QJsonObject jsonObj,char retJson[1024])
{
    sendnum++;
    QJsonObject recvJson;
    recvJson.empty();
    memset(retJson,0,1024);
    //QJsonObject转字符串
    char * send_data;
    int sendlen=0;
    int cmd = *pCmd;
    //先转成QJsonDocument
    if(jsonObj.isEmpty())
    {
        send_data=NULL;
    }
    else
    {
        QJsonDocument doc(jsonObj);
        //再转成QStringtrans_data
        QString strJson(doc.toJson(QJsonDocument::Compact));
        //最后转成char*
        send_data = fn_QString2char(strJson);
        sendlen = strlen(send_data);
    }
    //发送
    int tsp = getCurrentTime();
    myPtf("trans_data sendJson:%s\n",send_data);
    int ret = c_send_packet(sockfd,cmd,tsp,(u8 *)send_data,sendlen,serPath);
    if (send_data != NULL)
        free(send_data);
    if (ret<=0)
    {
        //尝试重连，然后退出
        conn_afunix();
        *pCmd = AF_UNIX_ERRORCOD_1002;
        myPtf("send_packet error!!!!\n");
        return recvJson;
    }
    //接收（暂未考虑超时。超时应在上一级考虑）
    //接收数据
    char * recv_data;
    //接收数据的包头
    cmd_head_t recv_head;
    memset(&recv_head,0,sizeof(cmd_head_t));
    recv_data = c_recv_packet(&recv_head,sockfd,&ser_addr,&ser_addrlen);
    /////////////////////////////////////
    //如果收到非本次发出的命令，丢弃该命令，再接收一次
    if(recv_head.tsp != tsp)
    {
        if (recv_data != NULL)
            free(recv_data);
        memset(&recv_head,0,sizeof(cmd_head_t));
        recv_data = c_recv_packet(&recv_head,sockfd,&ser_addr,&ser_addrlen);
    }
    /////////////////////////////////////
    if (recv_data==NULL)
    {
        myPtf("recv no parket,recv_cmd=%d!\n",recv_head.cmd);
        if(recv_head.cmd==0)
        {
            //尝试重连，然后退出
            conn_afunix();
            *pCmd = AF_UNIX_ERRORCOD_1002;
        }
    }
    else
    {
        *pCmd = recv_head.cmd;

        //显示接收
        myPtf("recv connfd=%d,cmd=%d,length=%d\n",sockfd,recv_head.cmd,recv_head.length);
        int i;
        for(i=0;i<recv_head.length;i++)
        {
            myPtf("%02X",*(recv_data+i));
        }
        myPtf("\n");
        myPtf("%d recv ok!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",sendnum);
        char * trans_data = (char *) malloc(recv_head.length+1);
        memcpy(retJson,recv_data,(recv_head.length>=1024)?1023:recv_head.length);
        memset(trans_data,0,recv_head.length+1);
        memcpy(trans_data,recv_data,recv_head.length);
        myPtf("trans_data recvJson:%s\n",recv_data);
        QByteArray jsonString(trans_data);
        myPtf("no error 1\n");
        QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonString);
        myPtf("no error 2\n");
        recvJson = jsonDocument.object();
        myPtf("no error 3\n");
        free(trans_data);
        if(recvJson.isEmpty())
        {
            myPtf("%d trans error~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~!\n",sendnum);
        }
        myPtf("no error 4\n");
        free(recv_data);
    }
    return recvJson;
}

/*
QJsonObject sendCmd::send_Command(int * pCmd,QJsonObject jsonObj)
{
    sendnum++;
    QJsonObject recvJson;
    recvJson.empty();
    //memset(retJson,0,1024);
    //QJsonObject转字符串
    char * send_data;
    int sendlen=0;
    int cmd = *pCmd;
    //先转成QJsonDocument
    if(jsonObj.isEmpty())
    {
        send_data=NULL;
    }
    else
    {
        QJsonDocument doc(jsonObj);
        //再转成QStringtrans_data
        QString strJson(doc.toJson(QJsonDocument::Compact));
        //最后转成char*
        send_data = fn_QString2char(strJson);
        sendlen = strlen(send_data);
    }
    //发送
    int tsp = getCurrentTime();
    int ret = c_send_packet(sockfd,cmd,tsp,(u8 *)send_data,sendlen,serPath);
    if (send_data != NULL)
        free(send_data);
    if (ret<=0)
    {
        //尝试重连，然后退出
        conn_afunix();
        *pCmd = AF_UNIX_ERRORCOD_1002;
        myPtf("send_packet error!!!!\n");
        return recvJson;
    }
    //接收（暂未考虑超时。超时应在上一级考虑）
    //接收数据
    char * recv_data;
    //接收数据的包头
    cmd_head_t recv_head;
    memset(&recv_head,0,sizeof(cmd_head_t));
    recv_data = c_recv_packet(&recv_head,sockfd,&ser_addr,&ser_addrlen);
    if (recv_data==NULL)
    {
        myPtf("recv no parket,recv_cmd=%d!\n",recv_head.cmd);
        if(recv_head.cmd==0)
        {
            //尝试重连，然后退出
            conn_afunix();
            *pCmd = AF_UNIX_ERRORCOD_1002;
        }
    }
    else
    {
        *pCmd = recv_head.cmd;

        //显示接收
        myPtf("recv connfd=%d,cmd=%d,length=%d\n",sockfd,recv_head.cmd,recv_head.length);
        int i;
        for(i=0;i<recv_head.length;i++)
        {
            myPtf("%02X",*(recv_data+i));
        }
        myPtf("\n");
        myPtf("%d recv ok!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",sendnum);
        char * trans_data = (char *) malloc(recv_head.length+1);
        //memcpy(retJson,recv_data,(recv_head.length>=1024)?1023:recv_head.length);
        memset(trans_data,0,recv_head.length+1);
        memcpy(trans_data,recv_data,recv_head.length);
        QByteArray jsonString(trans_data);
        QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonString);
        recvJson = jsonDocument.object();
        free(trans_data);
        if(recvJson.isEmpty())
        {
            myPtf("%d trans error~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~!\n",sendnum);
        }
        free(recv_data);
    }
    return recvJson;
}*/
