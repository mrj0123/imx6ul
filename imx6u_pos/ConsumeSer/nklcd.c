#include "a.h"
#include "nklcd.h"
#include "lcd.h"

//键盘线程ID
static pthread_t displaythreadid;

//显示线程内处理的真实业务逻辑
void *fn_DisplayInThread(void *args)
{
    Disp_Para dispara;
    int ZeroPos=0;
    int LoopTimes=0;
    //一直循环判断读取Key，每50ms判断一次显示
    while(1)
    {
        //if (LoopTimes%20==0) myPtf("disp_para.disp_type=%d,Money=%d\n",disp_para.display_type,dispara.Money);
        memcpy(&dispara,&disp_para,sizeof(Disp_Para));
        //if (LoopTimes%20==0) myPtf("disp_para.disp_type=%d,Money=%d\n",disp_para.display_type,dispara.Money);
        switch(dispara.display_type)
        {
        case DT_VERSION:
            Version_display();
            ZeroPos=0;
            break;
        case DT_DATETIME:
            DateTime_display();
            ZeroPos=0;
            break;
        case DT_INPUTMONEY:
            InputMoney_display(dispara.Money);
            ZeroPos=0;
            break;
        case DT_WAITFORCARD:
            WaitForCard_display(dispara.Money,ZeroPos);
            if(LoopTimes%10==0)
                ZeroPos=(ZeroPos>=4)?0:ZeroPos+1;
            break;
        case DT_CONSUMESUCEES:
            ConsumeSuccess_display(dispara.RemainMoney,dispara.Money);
            ZeroPos=0;
            break;
        case DT_ERRORCODE:
            ErrorCode_display(dispara.ErrCode);
            ZeroPos=0;
            break;
        default:
            ZeroPos=0;
            break;
        }
        usleep(100000);
        LoopTimes++;
    }
}

//启动键盘线程，应在主程序初始化时调用
int fn_StartDisplayThread()
{
    //启动显示线程
    myPtf("display thread is ready to start!\n");
	//初始化LCD屏幕
	lcd_module_init();
	memset(&disp_para,0,sizeof(Disp_Para));
    int ret=pthread_create(&displaythreadid,NULL,fn_DisplayInThread,NULL);
    myPtf("display thread is started!\n");
    return ret;
}

//结束键盘线程，可以不要
int fn_EndDisplayThread()
{
    pthread_join(displaythreadid,NULL);
    return 0;
}
