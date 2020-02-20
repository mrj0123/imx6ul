#include "afunix_udp.h"
#include "dispose.h"
#include "dealcmd.h"
#include <pthread.h>


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
    //bzero(&cli_addr,sizeof(cli_addr));
    cli_addr.sun_family = AF_UNIX;
    strcpy(cli_addr.sun_path,SECSCREEN_PATH_C);
    socklen_t addrlen=sizeof(cli_addr);
    //发送数据包长度
    int send_len;
    //关闭服务标志
    int stopflag = 0;
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
            myPtf("recv servfd=%d,cmd=%d,length=%d,data=%s\n",servfd,recv_head.cmd,recv_head.length,showdata);
            free(showdata);
            ///////////////////////////////////////////////////////////
            //构造包结构
            //分析命令
            tsp = recv_head.tsp; //返回的时间戳即为接收到的时间戳
            send_data = NULL;
            switch (recv_head.cmd)
            {
            //显示照片，但不现实文字
            case SECSCREEN_SHOWPIC_CMD:
                send_data = fn_dealCmd_ShowPic(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
            //在原来显示的内容上追加文字
            case SECSCREEN_APPENDTXT_CMD:
                send_data = fn_dealCmd_AppendTxt(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
            //显示照片，也显示文字
            case SECSCREEN_SHOWALL_CMD:
                send_data = fn_dealCmd_ShowAll(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
            case SECSCREEN_GETVERSION_CMD:
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
            myPtf("send_packet!\n");
            if (s_send_packet(servfd,send_cmd,tsp,(u8*)send_data, send_len,(struct sockaddr_un *)&cli_addr,&addrlen)<=0)
            {
                myPtf("s_send_packet error something to do for error!\n");
            }
            myPtf("send_packet over!\n");
            free(recv_data);
            if (send_len!=0)
            {
                free(send_data);
                send_data = NULL;
            }
        }
    }
}

