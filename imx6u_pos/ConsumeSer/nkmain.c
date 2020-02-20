#include "a.h"
#include "common.h"

#define MAX_PATH 260
#define CONF_FILE_PATH	"Config.ini"

void main()
{
    int iErrCode=0;
    //初始化
    /*
    //初始化显示线程
    iErrCode=fn_StartDisplayThread();
    if (iErrCode<0)
    {
        //错误提示,没有显示只有蜂鸣器
        fn_NoticErr(iErrCode);
        return;
    }
        //初始化键盘线程
    iErrCode=fn_StartKeyboardThread();
    if (iErrCode<0)
    {
        //错误提示
        fn_ShowErr(iErrCode);
        return;
    }
    */

    //读取当前目录
    char szConfigPath[MAX_PATH];
	memset(szConfigPath,0,sizeof(szConfigPath));
	fn_GetCurrentPath(szConfigPath,CONF_FILE_PATH);

    //读取配置信息
    strcpy(g_ServerIP,fn_GetIniKeyString("Server","ip",szConfigPath));
    g_ServerPort=fn_GetIniKeyInt("Server","port",szConfigPath);

    if ((g_ServerPort==0)||(strlen(g_ServerIP)==0))
    {
        iErrCode=-111;
        //错误提示
        fn_ShowErr(iErrCode);
        return;
    }
    myPtf("IP=%s  Port=%d\n",g_ServerIP,g_ServerPort);
    /*
    //自检
    iErrCode=fn_InitPos();
    if (iErrCode<0)
    {
        //错误提示
        fn_ShowErr(iErrCode);
        return;
    }
    //从网络获取初始信息
    iErrCode=fn_GetNetConfig();
    if (iErrCode<0)
    {
        //错误提示
        fn_ShowErr(iErrCode);
        return;
    }
    //调用结算程序等待消费
    myPtf("start consume!\n");
    //启动键盘监控线程
    fn_KeyboardThread();
    while ()
    //售饭
    fn_Consume();
    */
    //结束
    return;
}

//系统自检
int fn_InitPos()
{
    return 1;
}

