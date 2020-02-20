#include "a.h"
#include "nklcd.h"
#include "lcd.h"

//�����߳�ID
static pthread_t displaythreadid;

//��ʾ�߳��ڴ������ʵҵ���߼�
void *fn_DisplayInThread(void *args)
{
    Disp_Para dispara;
    int ZeroPos=0;
    int LoopTimes=0;
    //һֱѭ���ж϶�ȡKey��ÿ50ms�ж�һ����ʾ
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

//���������̣߳�Ӧ���������ʼ��ʱ����
int fn_StartDisplayThread()
{
    //������ʾ�߳�
    myPtf("display thread is ready to start!\n");
	//��ʼ��LCD��Ļ
	lcd_module_init();
	memset(&disp_para,0,sizeof(Disp_Para));
    int ret=pthread_create(&displaythreadid,NULL,fn_DisplayInThread,NULL);
    myPtf("display thread is started!\n");
    return ret;
}

//���������̣߳����Բ�Ҫ
int fn_EndDisplayThread()
{
    pthread_join(displaythreadid,NULL);
    return 0;
}
