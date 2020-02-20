#include "consumecallbackthread.h"


static pthread_t cbthreadid;

//启动线程，应在主程序初始化时调用
int fn_StartCallbackThread(SignalSender * signal)
{
    //启动显示线程
    myPtf("thread is ready to start!\n");
    int ret=pthread_create(&cbthreadid,NULL,fn_CsmCallBackThread,(void *)signal);
    return ret;
}
//结束键盘线程，可以不要
int fn_EndCallbackThread()
{
    pthread_join(cbthreadid,NULL);
    return 0;
}

void *fn_CsmCallBackThread(void *args)
{
    SignalSender * signal = (SignalSender *)args;
    //启动服务
    int serverfd = s_init_net(CONSUMECALLBACK_PATH_S);
    myPtf("callback serverfd init CONSUMECALLBACK_PATH_S");
    if (serverfd<=0)
    {
         myPtf("callback serverfd init error:%d",serverfd);
         return (void *)NULL;
    }
    //循环接收命令
    fn_dispose(serverfd,signal);
    //关闭网络
    close_net(serverfd,CONSUMECALLBACK_PATH_S);
    return (void *)NULL;
}

//命令处理函数
void fn_dispose(int servfd,SignalSender * signal)
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
    strcpy(cli_addr.sun_path,CONSUMECALLBACK_PATH_C);
    socklen_t addrlen=sizeof(cli_addr);
    //发送数据包长度
    int send_len;
    //关闭服务标志
    int stopflag = 0;
    //循环接收服务
    while(stopflag != 1)
    {
        myPtf("callback ready to recv_packet!!!!\n");
        /////////////////////////////////////////
        recv_data = s_recv_packet(&recv_head,servfd,(struct sockaddr_un *)&cli_addr,&addrlen);
        /////////////////////////////////////////
        if (recv_head.cmd==0)
        {
            myPtf("callback recv no parket,recv_cmd=%d!\n",recv_head.cmd);
        }
        else
        {
            if((recv_head.length!=0)&&(recv_data==NULL))
            {
                myPtf("callback recv data is empty,but recv_cmd=%d length=%d!\n",recv_head.cmd,recv_head.length);
                continue;
            }
            myPtf("callback recv lenth=%d,data:%s\n",recv_head.length,recv_data);
            //构造包结构
            //分析命令
            tsp = recv_head.tsp; //返回的时间戳即为接收到的时间戳
            switch (recv_head.cmd)
            {
            //获得消费或查询返回结果
            case CONSUME_CALLBACK_QUERET:
                signal->callEmitSignal(recv_data);
                send_cmd = recv_head.cmd+10000;
                send_data = NULL;
                send_len = 0;
                break;
            default:
                //返回错误代码
                send_data = NULL;
                send_len = 0;
                send_cmd = -1002;
                break;
            }
            //返回数据
            myPtf("callback send_packet!\n");
            if (s_send_packet(servfd,send_cmd,tsp,(u8*)send_data, send_len,(struct sockaddr_un *)&cli_addr,&addrlen)<=0)
            {
                myPtf("callback s_send_packet error something to do for error!\n");
            }
            else
            {
                myPtf("callback send_packet over!\n");
            }
            free(recv_data);
            if (send_len!=0)
            {
                free(send_data);
                send_data = NULL;
            }
        }
    }
}

//获取结果
/*
char * fn_getResult(char *recv_buf,int recv_len,int * ret_len)
{
    //解析JSON串，获得图片背景、显示位置
    *ret_len=0;
    if (recv_buf==NULL)
    {
        myPtf("NORECVBUF_ERROR\n");
        return NULL;
    }

    //解析Json串
    cJSON * jRoot=cJSON_Parse((char *)recv_buf);
    if(!jRoot)
    {
        myPtf("JSON_ERROR\n");
        return NULL;
    }
    //清空
    memset(callback_queret,0,sizeof(callback_queret));
    //取时间戳
    cJSON * jTimeStamp=cJSON_GetObjectItem(jRoot,"timeStamp");
    if(!jTimeStamp)
    {
        myPtf("JSON timeStamp is error\n");
        cJSON_Delete(jRoot);
        return NULL;
    }
    else
    {
        callback_queret.timeStamp =  atoi(jTimeStamp->valuestring);
    }
    //取状态
    cJSON * jState=cJSON_GetObjectItem(jRoot,"state");
    if(!jState)
    {
        myPtf("JSON state is error\n");
        cJSON_Delete(jRoot);
        return NULL;
    }
    else
    {
        callback_queret.state =  jState->valueint;
        //取返回值
        if(callback_queret.state == 1)
        {
            memcpy(callback_queret.retstring,recv_buf,recv_len);
        }
    }

    return NULL;
}
*/
