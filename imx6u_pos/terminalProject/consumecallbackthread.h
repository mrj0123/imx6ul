#ifndef CONSUMECALLBACKTHREAD_H
#define CONSUMECALLBACKTHREAD_H


#include "a.h"
#include "afunix_udp.h"
#include "signalsender.h"

#define CONSUMECALLBACK_PATH_C "/usr/local/nkty/consumecallback_c"
#define CONSUMECALLBACK_PATH_S "/usr/local/nkty/consumecallback_s"

//消费回调接口
#define CONSUME_CALLBACK_QUERET       6001    //获得消费结果以后，回调通知UI程序


//初始化回调服务线程
int fn_StartCallbackThread(SignalSender * signal);
//结束回调服务线程
int fn_EndCallbackThread();

//启动线程
void *fn_CsmCallBackThread(void *args);
//循环处理服务请求
void fn_dispose(int servfd,SignalSender * signal);
//获取结果
char * fn_getResult(char *recv_buf,int recv_len,int * ret_len);

#endif // CONSUMECALLBACKTHREAD_H
