#include "afunix_udp.h"
#include "sockfile.h"

#define SEND_BODY 8888


//客户端初始化
int c_init_net(char cli_path[108])
{
    int client_fd = unsock_server_init(cli_path);
    return client_fd;
}

//服务端初始化
int s_init_net(char ser_path[108])
{
    int server_fd = unsock_server_init(ser_path);
    return server_fd;
}
//关闭服务端连接，清楚通讯文件
int close_net(int fd,char path[108])
{
    close(fd);
    unlink(path);
    return 0;
}


//一次接收
char * recv_once_packet(cmd_head_t * pHead,int fd,struct sockaddr_un * src_addr,socklen_t * addrlen)
{
    int ret;
    size_t n;
    char recv_once_pak[MAXLINE];
    //拷贝包头
    if (pHead == NULL)
    {
        myPtf("head error!\n");
        return NULL;
    }
    memset(pHead,0,sizeof(cmd_head_t));
    memset(recv_once_pak,0,MAXLINE);
    myPtf("clean phead:cmd=%d,len=%d,pHead=%d?????????????\n",pHead->cmd,pHead->length,(int)pHead);
    //一次收取不大于MAXLINE的数据
    myPtf("begin recv : recvfd=%d\n",fd);
    ret = unsock_server_recvfrom(fd, (struct sockaddr *)src_addr, addrlen, recv_once_pak, MAXLINE , 10000);
    myPtf("end recv : recvfd=%d\n",fd);
    if (ret<(int)sizeof(cmd_head_t))
    {
        //包头接收失败，返回
        myPtf("recv head error %d\n", ret);
        myPtf("return phead:cmd=%d,len=%d?????????????\n",pHead->cmd,pHead->length);
        return NULL;
    }
    memcpy(pHead,recv_once_pak,sizeof(cmd_head_t));
    myPtf("recv head cmd=%d,length=%d\n",pHead->cmd,pHead->length);
    if (ret != pHead->length+sizeof(cmd_head_t)) {
        //包身接收失败，返回
        myPtf("recv data error %d\n", ret);
        return NULL;
    }
    if (pHead->length==0)
    {
        myPtf("recv head but no data!\n");
        return NULL;
    }
    //分配包身
    char * recvbuf =(char *) malloc(pHead->length);
    memcpy(recvbuf,recv_once_pak+sizeof(cmd_head_t),pHead->length);
    ///////////////////////////////////////////////
    myPtf("recvdata:");
    for(int i=0;i<pHead->length;i++)
    {
        myPtf("%02X",*((char *)(recvbuf + i)));
    }
    myPtf("\n");
    ///////////////////////////////////////////////

    myPtf("recv data OK!\n");
    return recvbuf;
}

//客户端（主动方）一次发送全部数据
int c_send_once_packet(int fd, int cmd, int tsp, u8 *buf, size_t n ,char ser_path[108])
{
    char send_once_pak[MAXLINE];
    memset(send_once_pak,0,MAXLINE);
    myPtf("fd=%d c_send packet start!",fd);
    cmd_head_t Head;
    Head.cmd = cmd;
    Head.length = n;
    Head.tsp = tsp;
    memcpy(send_once_pak,&Head,sizeof(cmd_head_t));
    memcpy(send_once_pak+sizeof(cmd_head_t),buf,n);
    //发送整包数据
    int ret = unsock_client_sendto(fd, send_once_pak, sizeof(cmd_head_t) + n,ser_path);
    myPtf("send once packet data:");
    for(int i=0;i<sizeof(cmd_head_t)+n;i++)
    {
        myPtf("%02X",*(send_once_pak+i));
    }
    myPtf(" ;ret:%d\n",ret);
    if (ret < 0 )
    {
        //发送失败
        return -3;
    }

    ///////////////////////////////////////////////
    myPtf("c send once packet over!\n");
    return ret;
}
//服务端（被动方）一次发送全部数据
//发送数据包，需要目的地址。客户端的目的地址是c_init_net返回的ser_addr，服务器端目的地址是s_recv_packet返回的cli_addr
int s_send_once_packet(int fd, int cmd, int tsp, u8 *buf, size_t n ,struct sockaddr_un * dst_addr,socklen_t * addrlen)
{
    myPtf("s_send_once_packet:fd=%d,cmd=%d,buf=%s,n=%d,dst_addr=%s,addrlen=%d\n",fd,cmd,buf,n,dst_addr->sun_path,*addrlen);
    if(sizeof(cmd_head_t) + n > MAXLINE)
    {
        return -1;
    }
    char send_once_pak[MAXLINE];
    memset(send_once_pak,0,MAXLINE);
    myPtf("fd=%d s_send packet start!",fd);
    cmd_head_t Head;
    Head.cmd = cmd;
    Head.length = n;
    Head.tsp = tsp;
    memcpy(send_once_pak,&Head,sizeof(cmd_head_t));
    memcpy(send_once_pak+sizeof(cmd_head_t),buf,n);
    //发送整包数据
    int ret = unsock_server_sendto(fd, (struct sockaddr *)dst_addr, addrlen, send_once_pak, sizeof(cmd_head_t) + n);
    myPtf("send once packet data:");
    for(int i=0;i<sizeof(cmd_head_t)+n;i++)
    {
        myPtf("%02X",*(send_once_pak+i));
    }
    myPtf(" ;ret:%d\n",ret);
    if (ret < 0 )
    {
        //发送失败
        return -3;
    }
    ///////////////////////////////////////////////
    myPtf("s send once packet over!\n");
    return ret;
}

char * recv_packet(cmd_head_t * pHead,int fd,struct sockaddr_un * src_addr,socklen_t * addrlen)
{
    /*
    int ret;
    size_t n;
    //分配包头空间
    //cmd_head_t * pHead = (cmd_head_t *)malloc(sizeof(cmd_head_t));
    if (pHead == NULL)
    {
        myPtf("head error!\n");
        return NULL;
    }
    memset(pHead,0,sizeof(cmd_head_t));
    n = sizeof(cmd_head_t);
    myPtf("sizeof(cmd_head_t)=%d\n",n);
    //接收包头
    //////////////////////////////////////////////////////////////////////////////////
    ret = unsock_server_recvfrom(fd, (struct sockaddr *)src_addr, addrlen, pHead, n , 10000);
    ///////////////////////////////////////////////
    myPtf("recvhead:cmd=%d,length=%d\n",pHead->cmd,pHead->length);
    ///////////////////////////////////////////////
    if (ret != (int)n) {
        //如果未能成功接受包头，返回错误
        myPtf("recv head error %d %d\n", ret, n);
        return NULL;
    }
    if (pHead->length>0) {
        //根据包身长度重新分配包空间
        char * recvbuf =(char *) malloc(pHead->length);
        //重新分配空间失败，返回
        if (recvbuf == NULL)
        {
            myPtf("malloc pack data error!\n");
            return NULL;
        }
        n = pHead->length;
        myPtf("sockaddr=%s\n",src_addr->sun_path);

        //请求发送包身
        myPtf("send answer:\n");
        cmd_head_t head;
        head.cmd = SEND_BODY;
        head.length = 0;
        usleep(10000);
        ret = unsock_server_sendto(fd, (struct sockaddr *)src_addr, addrlen, &head, sizeof(cmd_head_t));
        myPtf("send answer over %d\n",ret);
        if (ret<0)
        {
            return NULL;
        }
        //接收包身
        ///////////////////////////////////////////////////////////////
        ret = unsock_server_recvfrom(fd, (struct sockaddr *)src_addr, addrlen, recvbuf, n , 10000);
        ///////////////////////////////////////////////
        myPtf("recvdata:");
        for(int i=0;i<(int)n;i++)
        {
            myPtf("%02X",*((char *)(recvbuf + i)));
        }
        myPtf("\n");
        ///////////////////////////////////////////////
        if (ret != (int)n) {
            //包身接收失败，返回
            myPtf("recv data error %d\n", ret);
            return NULL;
        }
        myPtf("recv data OK!\n");
        return recvbuf;
    }
    else
    {
        return NULL;
    }
    */
    char * ret = recv_once_packet(pHead,fd,src_addr,addrlen);
    myPtf("c pHead cmd=%d,length=%d,pHead=%d",pHead->cmd,pHead->length,(int)pHead);
    return ret;
}

//发送数据包，需要目的地址。客户端的目的地址是c_init_net返回的ser_addr，服务器端目的地址是s_recv_packet返回的cli_addr
int s_send_packet(int fd, int cmd, int tsp, u8 *buf, size_t n ,struct sockaddr_un * dst_addr,socklen_t * addrlen)
{
    myPtf("s_send_packet:fd=%d,cmd=%d,buf=%s,n=%d,dst_addr=%s,addrlen=%d\n",fd,cmd,buf,n,dst_addr->sun_path,*addrlen);
    /*
    myPtf("send packet start!!!!\n");
    cmd_head_t * pHead = (cmd_head_t *)malloc(sizeof(cmd_head_t));
    pHead->cmd = cmd;
    pHead->length = n;
    myPtf("send packet!\n");

    //int ret = sendto(fd, phead, sizeof(cmd_head_t), 0, (struct sockaddr *)dst_addr, len);
    int ret = unsock_server_sendto(fd, (struct sockaddr *)dst_addr, addrlen, pHead, sizeof(cmd_head_t));
    if (ret < 0 )
    {
        myPtf("send packet error ret=%d!",ret);
        free(pHead);
        return -1;
    }
    //如果只发包头，返回
    if (buf==NULL || n==0)
    {
        free(pHead);
        return ret;
    }
    myPtf("client:send head OK!");

    //接收返回确认数据
    //ret = recvfrom(fd, phead, sizeof(cmd_head_t) , 0 ,  (struct sockaddr *)dst_addr , &len);
    ret = unsock_server_recvfrom(fd, (struct sockaddr *)dst_addr, addrlen, pHead, sizeof(cmd_head_t),10000);
    if (pHead->cmd != SEND_BODY)
    {
        free(pHead);
        return -2;
    }

    free(pHead);
    //发送包身
    //ret = sendto(fd, buf, n, 0, (struct sockaddr *)dst_addr, len);
    ret = unsock_server_sendto(fd, (struct sockaddr *)dst_addr, addrlen, buf, n);
    if (ret < 0 )
    {
        return -3;
    }
    ///////////////////////////////////////////////
    myPtf("send packet over!!!!\n");
    return ret;
    */
    return s_send_once_packet(fd, cmd, tsp, buf, n ,dst_addr,addrlen);
}

//客户端发送数据包，需要服务器段路径作为目的地址
int c_send_packet(int fd, int cmd, int tsp, u8 *buf, size_t n ,char ser_path[108])
{
    /*
    myPtf("send packet start!");
    cmd_head_t * pHead = (cmd_head_t *)malloc(sizeof(cmd_head_t));
    pHead->cmd = cmd;
    pHead->length = n;

    int ret = unsock_client_sendto(fd, pHead, sizeof(cmd_head_t),ser_path);
    if (ret < 0 )
    {
        myPtf("send packet error ret=%d!",ret);
        free(pHead);
        return -1;
    }
    //如果只发包头，返回
    if (buf==NULL || n==0)
    {
        free(pHead);
        return ret;
    }

    //接收返回确认数据
    //ret = recvfrom(fd, phead, sizeof(cmd_head_t) , 0 ,  (struct sockaddr *)dst_addr , &len);
    struct sockaddr_un src_addr;
    socklen_t addrlen=sizeof(src_addr);
    memset(&src_addr,0,addrlen);
    ret = unsock_server_recvfrom(fd, (struct sockaddr *)&src_addr, &addrlen, pHead, sizeof(cmd_head_t),10000);
    if (pHead->cmd != SEND_BODY)
    {
        free(pHead);
        return -2;
    }

    free(pHead);

    //发送包身
    //ret = sendto(fd, buf, n, 0, (struct sockaddr *)dst_addr, len);
    ret = unsock_client_sendto(fd, buf, n, ser_path);
    if (ret < 0 )
    {
        return -3;
    }
    ///////////////////////////////////////////////
    myPtf("send packet over!\n");
    return ret;
    */
    return c_send_once_packet(fd,cmd,tsp,buf,n,ser_path);
}

