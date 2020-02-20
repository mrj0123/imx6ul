#include "a.h"
#include "common.h"

#define MAX_PATH 260
#define CONF_FILE_PATH	"Config.ini"

void main()
{
    int iErrCode=0;
    //��ʼ��
    /*
    //��ʼ����ʾ�߳�
    iErrCode=fn_StartDisplayThread();
    if (iErrCode<0)
    {
        //������ʾ,û����ʾֻ�з�����
        fn_NoticErr(iErrCode);
        return;
    }
        //��ʼ�������߳�
    iErrCode=fn_StartKeyboardThread();
    if (iErrCode<0)
    {
        //������ʾ
        fn_ShowErr(iErrCode);
        return;
    }
    */

    //��ȡ��ǰĿ¼
    char szConfigPath[MAX_PATH];
	memset(szConfigPath,0,sizeof(szConfigPath));
	fn_GetCurrentPath(szConfigPath,CONF_FILE_PATH);

    //��ȡ������Ϣ
    strcpy(g_ServerIP,fn_GetIniKeyString("Server","ip",szConfigPath));
    g_ServerPort=fn_GetIniKeyInt("Server","port",szConfigPath);

    if ((g_ServerPort==0)||(strlen(g_ServerIP)==0))
    {
        iErrCode=-111;
        //������ʾ
        fn_ShowErr(iErrCode);
        return;
    }
    myPtf("IP=%s  Port=%d\n",g_ServerIP,g_ServerPort);
    /*
    //�Լ�
    iErrCode=fn_InitPos();
    if (iErrCode<0)
    {
        //������ʾ
        fn_ShowErr(iErrCode);
        return;
    }
    //�������ȡ��ʼ��Ϣ
    iErrCode=fn_GetNetConfig();
    if (iErrCode<0)
    {
        //������ʾ
        fn_ShowErr(iErrCode);
        return;
    }
    //���ý������ȴ�����
    myPtf("start consume!\n");
    //�������̼���߳�
    fn_KeyboardThread();
    while ()
    //�۷�
    fn_Consume();
    */
    //����
    return;
}

//ϵͳ�Լ�
int fn_InitPos()
{
    return 1;
}

