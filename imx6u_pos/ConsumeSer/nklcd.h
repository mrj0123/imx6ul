
//显示类型，用于表述各种显示页面
//enum DispType{Version,Datetime,Inputmoney,Waitforcard,Consumesuccess,Errorcode};
#define DT_VERSION 1
#define DT_DATETIME 2
#define DT_INPUTMONEY 3
#define DT_WAITFORCARD 4
#define DT_CONSUMESUCEES 5
#define DT_ERRORCODE 6


typedef struct{
    int display_type;   //显示类型
    int Money;               //输入金额、消费额
    int Havedot;             //输入有小数点为1
    int RemainMoney;         //余额
    int ErrCode;             //错误码
}Disp_Para;

Disp_Para disp_para;


//启动键盘线程，应在主程序初始化时调用
int fn_StartDisplayThread();
//结束键盘线程，可以不要
int fn_EndDisplayThread();
