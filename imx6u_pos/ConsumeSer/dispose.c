#include "afunix_udp.h"
#include "dispose.h"
#include "dealcmd.h"


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
    int send_cmd=0;
    int tsp=0;
    /////////////////////////////////////////////
    //发送数据的包身
    char * send_data = NULL;
    //链接的客户端
    struct sockaddr_un cli_addr;
    bzero(&cli_addr,sizeof(cli_addr));
    socklen_t addrlen=sizeof(struct sockaddr_un);
    //发送数据包长度
    int send_len;
    while(1)
    {
        myPtf("ready to recv_once_packet recv_head=%d!!!!\n",(int)(&recv_head));
        myPtf("dispose: begin recv phead:cmd=%d,len=%d!!!!!!!!!!\n",recv_head.cmd,recv_head.length);
        /////////////////////////////////////////
        recv_data = recv_packet(&recv_head,servfd,(struct sockaddr_un *)&cli_addr,&addrlen);
        myPtf("dispose: end recv phead:cmd=%d,len=%d!!!!!!!!!!\n",recv_head.cmd,recv_head.length);
        //recv_data 是不带结束符'\0'的
        /////////////////////////////////////////
        if (recv_head.cmd==0)
        {
            myPtf("error: recv_cmd = 0\n");
        }
        else
        {
            if((recv_head.length!=0)&&(recv_data==NULL))
            {
                myPtf("error: recv data is empty,but length=%d!\n",recv_head.length);
                continue;
            }
            //显示接收
            ///////////////////////////////////////////////////////////
            char * showdata= (char *)malloc(recv_head.length+1);
            memset(showdata,0,recv_head.length+1);
            memcpy(showdata,recv_data,recv_head.length);
            myPtf("recv Conserver servfd=%d,cmd=%d,length=%d,data=%s\n",servfd,recv_head.cmd,recv_head.length,showdata);
            free(showdata);
            myPtf("consumeser recv UI here 1 cmd = %d\n",recv_head.cmd);
            ///////////////////////////////////////////////////////////
            //构造包结构
            //分析命令
            send_data = NULL;
            send_len = 0;
            tsp = recv_head.tsp; //返回的时间戳即为接收到的时间戳
            switch (recv_head.cmd)
            {
            case ECHO_CMD:
                send_data = fn_dealCmd_Echo(recv_data,recv_head.length,&send_len,&send_cmd);
                break;
            //初始化界面信息
            case UI_GET_TERMINFO:
                send_data = fn_dealCmd_GetTermInfo(recv_data,recv_head.length,&send_len,&send_cmd);
                break;
            /*
            //获取消费总金额，消费总次数(该接口废弃，可用UI_GET_FLOWTOTAL获得相关数据)
            case UI_GET_CONSUMEINFO:
                myPtf("recv2");
                send_data = fn_dealCmd_GetConsumeInfo(recv_data,recv_head.length,&send_len,&send_cmd);
                myPtf("recv3");
                break;
            */
            //请求消费
            case UI_REQ_CONSUME:
                send_data = fn_dealCmd_ReqConsume(recv_data,recv_head.length,&send_len,&send_cmd);
                break;
            //查询消费结果
            case UI_QUE_CONSUMERET:
                send_data = fn_dealCmd_QueConsumeRet(recv_data,recv_head.length,&send_len,&send_cmd);
                break;
            //取消消费
            case UI_CANCEL_CONSUME:
                send_data = fn_dealCmd_CancelConsume(recv_data,recv_head.length,&send_len,&send_cmd);
                break;
            //获取流水总金额和总条数
            case UI_GET_FLOWTOTAL:
                send_data = fn_dealCmd_GetFlowTatal(recv_data,recv_head.length,&send_len,&send_cmd);
                break;
            //获取流水明细
            case UI_GET_FLOWINFO:
                send_data = fn_dealCmd_GetFlowInfo(recv_data,recv_head.length,&send_len,&send_cmd);
                break;
            //获取消费服务版本号
            case UI_GET_VERSION:
                send_data = fn_dealCmd_GetVersion(recv_data,recv_head.length,&send_len,&send_cmd);
                break;
            //获取扫码服务版本号
            case UI_GET_SCANQRVERSION:
                send_data = fn_dealCmd_GetScanQrSerVersion(recv_data,recv_head.length,&send_len,&send_cmd);
                break;
            //获取账户信息
            case UI_REQ_ACCOUNTINFO:
                send_data = fn_dealCmd_GetAccountInfo(recv_data,recv_head.length,&send_len,&send_cmd);
                break;
            //扫描二维码数据并返回
            case UI_REQ_SCANQRCODE:
                send_data = fn_dealCmd_GetQrcode(recv_data,recv_head.length,&send_len,&send_cmd);
                break;
            case UI_EXEC_CANCELFLOW:
                send_data = fn_dealCmd_CancelFlow(recv_data,recv_head.length,&send_len,&send_cmd);
                break;
            case UI_REQ_OFFLINECONSUME:
                send_data = fn_dealCmd_ReqOfflineConsume(recv_data,recv_head.length,&send_len,&send_cmd);
                break;
            case UI_REQ_UPLOADFLOW:
                send_data = fn_dealCmd_ReqUploadFlow(recv_data,recv_head.length,&send_len,&send_cmd);
                break;
            case UI_GET_OFFLINEFLOWNUM:
                send_data = fn_dealCmd_GetOfflineFlowNum(recv_data,recv_head.length,&send_len,&send_cmd);
                break;
            case UI_REQ_DOWNLOADUSERLIST:
                send_data = fn_dedlCmd_DownloadUserList(recv_data,recv_head.length,&send_len,&send_cmd);
                break;
            case UI_GET_USERLISTNUM:
                send_data = fn_dedlCmd_GetUserListPackNum(recv_data,recv_head.length,&send_len,&send_cmd);
                break;
            default:
                //返回错误代码
                send_len = 0;
                send_cmd =ERRORCOD_NOCMD;
                break;
            }
            //返回数据
            myPtf("send_packet!send_cmd=%d,send_len=%d,send_data=%s,cli_addr=%s,addrlen=%d\n",send_cmd,send_len,send_data,cli_addr.sun_path,addrlen);
            int ret = s_send_packet(servfd,send_cmd,tsp,(u8*)send_data,send_len,(struct sockaddr_un *)&cli_addr,&addrlen);
            //int ret = send_once_packet(servfd,send_cmd, (u8*)send_data, send_len,(struct sockaddr_un *)&cli_addr,&addrlen);
            if (ret<=0)
            {
                myPtf("something to do for error %d!\n",ret);
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
    //close_net(servfd);
}

