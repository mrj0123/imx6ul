#include "nksocket.h"

#define MAXSLEEP 1

static struct sockaddr_in tcp_servaddr;
static int tcp_sockfd;
static struct sigaction act;

static ser_conn_t serconn[MAX_SERVER_NUM];   //服务器链接对象应逐个尝试链接

static int netstatus=-1; //网络状态，-1表示无法连接服务器，其余表示连接哪组服务器
/////////////////////////////////

//链接服务器
int con_socket()
{
    int nsec;
    myPtf("con_socket start\n");
    //按照碰撞机制，如果本次链接不上，间隔双倍时间再次重连，最多到10秒以内
    for(nsec=1;nsec<=MAXSLEEP;nsec<<=1)
    {
        myPtf("ready for connect new socket %d\n",nsec);
        if(Connect(tcp_sockfd, (struct sockaddr *)&tcp_servaddr, sizeof(tcp_servaddr)) == 0)
        {
            myPtf("connect new socket success\n");
            return 0;//connection accepted
        }
        myPtf("connect new socket fault %d\n",nsec);
        if(nsec <= MAXSLEEP/2)//sleep nesc,then connect retry
            usleep(nsec*10000);
    }
    return -1;
}

int realinit_socket()
{
    //链接参数设置
    act.sa_handler = handle_sig;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGPIPE,&act,NULL);
    tcp_sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    myPtf("cloud socketfd is %d---------------------------------------\n",tcp_sockfd);
    //设置IP地址
    if (netstatus>=0)
    {

        bzero(&tcp_servaddr, sizeof(tcp_servaddr));
        tcp_servaddr.sin_family = AF_INET;
        inet_pton(AF_INET, serconn[netstatus].SerIP, &tcp_servaddr.sin_addr);
        tcp_servaddr.sin_port = htons(serconn[netstatus].SerPort);
        myPtf("old con_socket %d,ip=%s,port=%d\n",netstatus,serconn[netstatus].SerIP,serconn[netstatus].SerPort);
        if (con_socket()==0)
        {
            myPtf("old con_socket is ok!\n");
            return 0;
        }
    }
    for(int i=0;i<MAX_SERVER_NUM;i++)
    {
        netstatus = -1;
        bzero(&tcp_servaddr, sizeof(tcp_servaddr));
        tcp_servaddr.sin_family = AF_INET;
        inet_pton(AF_INET, serconn[i].SerIP, &tcp_servaddr.sin_addr);
        tcp_servaddr.sin_port = htons(serconn[i].SerPort);
        int ret = con_socket();
        myPtf("con_socket %d,ip=%s,port=%d,ret=%d\n",i,serconn[i].SerIP,serconn[i].SerPort,ret);
        if (ret==0)
        {
            netstatus = i;
            return 0;
        }
    }
    return -1;
}

void handle_sig(int signle)
{
    myPtf("*** error = %d ***\n",signle);
    usleep(100000);
    myPtf("close\n");
    close_socket();
    myPtf("ini\n");
    realinit_socket();
}

int init_socket(ser_conn_t server_conn[MAX_SERVER_NUM])
{
    for(int i=0;i<MAX_SERVER_NUM;i++)
    {
        strcpy(serconn[i].SerIP,server_conn[i].SerIP);
        serconn[i].SerPort = server_conn[i].SerPort;
        myPtf("%d: ip=%s,port=%d\n",i+1,serconn[i].SerIP,serconn[i].SerPort);
    }
    //return realinit_socket();
    return 0;
}


//返回0表示成功，<0表示失败？
int send_socket(char * buff,int len)
{

    int n;
    int loop=0;
again:
    myPtf("start write socket fd=%d!\n",tcp_sockfd);
    //发送数据
    n = Write(tcp_sockfd, buff, len);
    //如果发送失败，尝试重连
    if (n<0)
    {
        //关闭原链接
        close_socket();
        myPtf("close old socket!\n");
        myPtf("ini\n");
        //重连成功，再次发送
        if(realinit_socket()>=0)
        {
            myPtf("open new socket!\n");
            //循环3次失败，则退出失败，返回-1（能连不能发）
            if (loop++>0)
            {
                return -1;
            }
            //未到3次，再次尝试；
            goto again;
        }
        //重连失败，返回-2（链接失败）
        else
            return -2;
    }
    myPtf("send %d over:%s\n",n,buff);
    return 0;
}
//0表示接收成功，接收到len个字符
int recv_socket(char * buff,int maxlen)
{

    char recvchar=-1;
    int i=0;
    int n=0;
    int maxtimes=1;
    int loops=0;
    //循环读取数据，直到读到0
    while(recvchar!='\0')
    {
        //读取一个字符
        n = Read(tcp_sockfd, &recvchar,1);
        if (n==1)
        {
            memcpy(buff+i,&recvchar,1);
            i++;
        }
        else
        {
            recvchar=-1;
        }
        if (n!=1)
        {
            myPtf("have read %d\n",n);
            loops++;
            if(loops>maxtimes)
            {
                return i-1;
            }
        }
        //接收到最大值都没有收到'\0',表示本次接收失败
        if (i>=maxlen)
        {
            memset(buff,0,maxlen);
            ////////////////////
            //关闭socket，
            close_socket();
            //重新链接，退出
            myPtf("ini\n");
            realinit_socket();
            return -1;
        }
    }
    myPtf("recv=%d\n",i-1);
    return i-1;

}

int close_socket()
{
    Close(tcp_sockfd);
    return 0;
}
