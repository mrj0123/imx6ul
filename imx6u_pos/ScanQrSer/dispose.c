#include "afunix_udp.h"
#include "dispose.h"
#include "dealcmd.h"
#include<time.h>

int fn_init_dispose()
{
    return fn_dealCmd_init();
}

int fn_destroy_dispose()
{
    return fn_dealCmd_destroy();
}

//命令处理函数
void fn_dispose(int servfd)
{
    //接收数据的包头
    cmd_head_t recv_head;
    //接收数据的包身
    char * recv_data = NULL;

    //发送数据的命令
    int send_cmd;
    /////////////////////////////////////////////
    //发送数据的包身
    char * send_data = NULL;
    int tsp=0;
    //链接的客户端
    struct sockaddr_un cli_addr;
    bzero(&cli_addr,sizeof(cli_addr));
    socklen_t addrlen=sizeof(struct sockaddr_un);
    //发送数据包长度
    int send_len;
    //关闭服务标志
    int stopflag = 0;
    time_t timep;
    //循环接收服务
    while(stopflag != 1)
    {
        myPtf("ready to recv_packet!!!!\n");
        /////////////////////////////////////////
        //recv_packet = s_recv_packet(servfd,&cli_addr);
        //char * recv_packet(int fd,struct sockaddr_un * dst_addr,socklen_t * addr_len);
        recv_data = recv_packet(&recv_head,servfd,(struct sockaddr_un *)&cli_addr,&addrlen);
        /////////////////////////////////////////
        if (recv_head.cmd==0)
        {
            myPtf("recv no parket,recv_cmd=%d!\n",recv_head.cmd);
        }
        else
        {
            if((recv_head.length!=0)&&(recv_data==NULL))
            {
                myPtf("recv data is empty,but length=%d!\n",recv_head.length);
                continue;
            }
            //显示接收
            ///////////////////////////////////////////////////////////
            char * showdata= (char *)malloc(recv_head.length+1);
            memset(showdata,0,recv_head.length+1);
            memcpy(showdata,recv_data,recv_head.length);
            time (&timep);
            myPtf("%s:recv servfd=%d,cmd=%d,length=%d,data=%s\n",ctime(&timep),servfd,recv_head.cmd,recv_head.length,showdata);
            free(showdata);
            ///////////////////////////////////////////////////////////
            //构造包结构
            //分析命令
            tsp = recv_head.tsp; //返回的时间戳即为接收到的时间戳
            send_data = NULL;
            switch (recv_head.cmd)
            {
            //处理命令 开始捕获
            case SCANQR_BEGIN_CAPTURE_CMD:
                send_data = fn_dealCmd_BeginCap(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
            //处理命令 解码
            case SCANQR_GET_QRCODE_CMD:
                send_data = fn_dealCmd_GetQRCode(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
            case SCANQR_STOP_QRCODE_CMD:
                send_data = fn_dealCmd_StopQRCode(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
            //处理命令 停止捕获
            case SCANQR_STOP_CAPTURE_CMD:
                send_data = fn_dealCmd_EndCap(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
            //处理命令 获取版本号
            case SCANQR_GET_VERSION_CMD:
                send_data = fn_dealCmd_GetVersion(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
            default:
                //返回错误代码
                send_len = 0;
                send_cmd = AF_UNIX_ERRORCOD_1002;
                break;
            }
            //返回数据
            time (&timep);
            myPtf("%s:send_packet!\n",ctime(&timep));
            if (s_send_packet(servfd,send_cmd,tsp,(u8*)send_data, send_len,(struct sockaddr_un *)&cli_addr,&addrlen)<=0)
            {
                myPtf("something to do for error!\n");
            }
            time (&timep);
            myPtf("%s:send_packet over!\n",ctime(&timep));
            free(recv_data);
            if (send_len!=0)
            {
                free(send_data);
                send_data = NULL;
            }
        }
    }
    //close_net(servfd);
}

