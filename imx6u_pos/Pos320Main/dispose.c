#include "afunix_udp.h"
#include "dispose.h"
#include "dealcmd.h"
#include "log.h"
#include "a.h"
#include <sys/sysinfo.h>
//命令处理函数
void fn_dispose(int servfd)
{
    //接收数据的包头
    cmd_head_t recv_head;
    //接收数据的包身
    char * recv_data = NULL;
    int tsp;//时间戳，当天时间不含日期，精确到0.1毫秒
    //发送数据的命令
    int send_cmd;
    /////////////////////////////////////////////
    //发送数据的包身
    char * send_data = NULL;


    //链接的客户端
    struct sockaddr_un cli_addr;
    bzero(&cli_addr,sizeof(cli_addr));
    socklen_t addrlen=sizeof(struct sockaddr_un);
    //发送数据包长度
    int send_len;
    //关闭服务标志
    int stopflag = 0;
    //循环接收服务
    while(stopflag != 1)
    {
        //printf("ready to recv_packet!!!!\n");

	write_log_s("%s\n", "ready to recv_packet!!!!");
        /////////////////////////////////////////
        //recv_packet = s_recv_packet(servfd,&cli_addr);
        //char * recv_packet(int fd,struct sockaddr_un * dst_addr,socklen_t * addr_len);
        recv_data = recv_packet(&recv_head,servfd,(struct sockaddr_un *)&cli_addr,&addrlen);
        /////////////////////////////////////////
        if (recv_head.cmd==0)
        {
	        //printf("recv no parket,recv_cmd=%d!\n",recv_head.cmd);
		write_log_s("recv no parket,recv_cmd=%d!\n",recv_head.cmd);
        }
        else
        {
            if((recv_head.length!=0)&&(recv_data==NULL))
            {
//                printf("recv data is empty,but length=%d!\n",recv_head.length);
		write_log_s("recv data is empty,but length=%d!\n",recv_head.length);
                continue;
            }
            //显示接收
            ///////////////////////////////////////////////////////////
            //char * showdata= (char *)malloc(recv_head.length+1);
            //memset(showdata,0,recv_head.length+1);
            //memcpy(showdata,recv_data,recv_head.length);
            //printf("recv servfd=%d,cmd=%d,length=%d,data=%s\n",servfd,recv_head.cmd,recv_head.length,showdata);
    	    //write_log_s("recv servfd=%d,cmd=%d,length=%d,data=%s\n",servfd,recv_head.cmd,recv_head.length,showdata);
	    	
            //free(showdata);
  	    //showdata=NULL;

            ///////////////////////////////////////////////////////////
            //构造包结构
  	    tsp=recv_head.tsp;//时间戳，当天时间不含日期，精确到0.1毫秒
            //分析命令
            send_data = NULL;
		write_log_s("==================进入函数前================\n");	
		struct sysinfo si;
		sysinfo(&si);
		write_log_s("Totalram:       %d\n", si.totalram);
		write_log_s("Available:      %d\n", si.freeram);
            switch (recv_head.cmd)
            {
            //处理命令 获得硬件序列号
            case HARDWARE_SERIALNO_GET_CMD:
                send_data = fn_dealCmd_GetHardwareSerialNo(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
	     //处理命令 获得网络连接状态
            case NETWORK_CONNECT_STATUS_GET:
                send_data = fn_dealCmd_GetNetworkConnectStatus(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;


             //处理命令 设置网络参数
            case NETWORK_INFOMATION_SET_CMD:
                send_data = fn_dealCmd_SetNetWorkSetting(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
	     //处理命令 获得网络参数
            case NETWORK_INFOMATION_GET_CMD:
                send_data = fn_dealCmd_GetNetWorkSetting(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
	    //处理命令 设置无线wifi参数
            case WIFI_SETTING_SET_CMD:
                send_data = fn_dealCmd_SetWifiSetting(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
	     //处理命令 获得无线wifi参数
            case WIFI_SETTING_GET_CMD:
                send_data = fn_dealCmd_GetWifiSetting(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
	    //处理命令 GPRS，开、关操作
            case GPRS_SWITCH_SET_CMD:
                send_data = fn_dealCmd_SetGPRSSetting(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
	     //处理命令 获得GPRS状态
            case GPRS_STATUS_GET_CMD:
                send_data = fn_dealCmd_GetGPRSSetting(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
	    //处理命令 服务器参数设置
            case SERVER_SETTING_SET_CMD:
                send_data = fn_dealCmd_SetServerSetting(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
	     //处理命令 获得服务器参数
            case SERVER_SETTING_GET_CMD:
                send_data = fn_dealCmd_GetServerSetting(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
	    //处理命令 获得运行版本
            case RUNNING_VERSION_GET:
                send_data = fn_dealCmd_GetVersion(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
	    //处理命令 获得wifi状态
            case WIFI_STATUS_GET:
                send_data = fn_dealCmd_GetWifiStatus(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
	    //处理命令 更新各个进程时间
	    case ALL_PROCESS_STATUS_UPDATE:
                send_data = fn_dealCmd_UpdateAllProcessStatus(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
                break;
	    //处理命令  保存设置参数
 	    case SETTING_SAVEALL_SET:
                send_data = fn_dealCmd_SAVEALL_SETTING(recv_data,recv_head.length,&send_len);
                send_cmd = recv_head.cmd+10000;
		break;
            default:
                //返回错误代码
                send_len = 0;
                send_cmd = AF_UNIX_ERRORCOD_1001;
                break;
            }
            //返回数据
            //printf("send_packet!\n");
	write_log_s("==================函数结束后================\n");	
	struct sysinfo si2;
	sysinfo(&si2);
	write_log_s("Totalram:       %d\n", si2.totalram);
	write_log_s("Available:      %d\n", si2.freeram);

	    write_log_s("send_packet!\n");
//		printf("准备发送数据:%s,%d\n",send_data,send_len);

	

	//char sendbuffer[send_len+1];        
	//sprintf(sendbuffer,"%s",send_data);
	//char *send_data2=(char *)malloc(send_len+1);
    	//memset(send_data2,0,strlen(sendbuffer)+1);
    	//memcpy(send_data2,sendbuffer,strlen(sendbuffer));
	 //发送
    	//ret = s_send_packet(sockfd,send_cmd,(u8 *)send_data2,send_len,GATE_PATH_S);
    	//free(send_data);



            if (s_send_packet(servfd,send_cmd,tsp,(u8*)send_data, send_len,(struct sockaddr_un *)&cli_addr,&addrlen)<=0)
            {
                //printf("something to do for error!\n");
		write_log_s("something to do for error!,%d,%d,%d,%s,%d,%d\n",servfd,send_cmd,tsp,send_data,strlen(send_data),addrlen);
            }
//            printf("send_packet over!\n");
  	    write_log_s("send_packet over!，data:%s,send_cmd:%d\n",send_data,send_cmd);

		
            
	    if (recv_data!=NULL)
            {
                free(recv_data);
                recv_data = NULL;
		
        
        
            }
            if (send_data!=NULL)
            {
                free(send_data);
                send_data = NULL;
		
        
        
            }
            //if (send_data2!=NULL)
            //{
            //    free(send_data2);
            //    send_data2 = NULL;
            //}
	

        }
    }
    //close_net(servfd);
}

