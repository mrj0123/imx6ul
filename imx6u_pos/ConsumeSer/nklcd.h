
//��ʾ���ͣ����ڱ���������ʾҳ��
//enum DispType{Version,Datetime,Inputmoney,Waitforcard,Consumesuccess,Errorcode};
#define DT_VERSION 1
#define DT_DATETIME 2
#define DT_INPUTMONEY 3
#define DT_WAITFORCARD 4
#define DT_CONSUMESUCEES 5
#define DT_ERRORCODE 6


typedef struct{
    int display_type;   //��ʾ����
    int Money;               //��������Ѷ�
    int Havedot;             //������С����Ϊ1
    int RemainMoney;         //���
    int ErrCode;             //������
}Disp_Para;

Disp_Para disp_para;


//���������̣߳�Ӧ���������ʼ��ʱ����
int fn_StartDisplayThread();
//���������̣߳����Բ�Ҫ
int fn_EndDisplayThread();
