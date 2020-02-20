#ifndef NKKEYBOARD_H
#define NKKEYBOARD_H

//——————————————键盘相关定义begin————————————///////
//键值定义
#define NUM1_KEY 79      //1键
#define NUM2_KEY 80      //2键
#define NUM3_KEY 81      //3键
#define NUM4_KEY 75      //4键
#define NUM5_KEY 76      //5键
#define NUM6_KEY 77      //6键
#define NUM7_KEY 71      //7键
#define NUM8_KEY 72      //8键
#define NUM9_KEY 73      //9键
#define NUM0_KEY 82      //0键

#define A_KEY 30         //A键,同时表示二级菜单键，表示进入二级菜单
#define B_KEY 48         //B键
#define C_KEY 46         //C键
#define D_KEY 32         //D键
#define E_KEY 18         //-键与E键复用

#define RETURN_KEY 96    //回车符，表示输入结束，进入等待刷卡或扫码消费
#define CLEAR_KEY 111    //退格键，表示删除原输入
#define FIXED_KEY 69     //固定键
#define CASH_KEY  78     //现金键与+键键复用

#define POINT_KEY 32     //.键与D键复用
#define ADD_KEY   78     //+键与现金键复用
#define SUB_KEY   18     //-键与E键复用

#define UNUM1_KEY 2     //用户面1键
#define UNUM2_KEY 3     //用户面2键
#define UNUM3_KEY 4     //用户面3键
#define UNUM4_KEY 5     //用户面4键
#define UNUM5_KEY 6     //用户面5键
#define UNUM6_KEY 7     //用户面6键
#define UNUM7_KEY 8     //用户面7键
#define UNUM8_KEY 9     //用户面8键ls
#define UNUM9_KEY 10     //用户面9键
#define UNUM0_KEY 11     //用户面0键

#define UCLEAR_KEY 14    //用户面退格键
#define UENTER_KEY 28    //用户面回车键

#define NO_KEY    0xFF     //未读到键值
#define EXIT_KEY  0xFE     //退出

#define MAX_KEYBUFFER_LEN 0x500 //最大键盘缓存
#define MAX_UKEYBUFFER_LEN 0x500//最大用户面键盘缓存
//——————————————键盘相关定义end————————————///////

#define OPER_SIDE 0         //操作员面键盘
#define USER_SIDE 1         //用户面键盘

#define KEY_MYUPPER 0xFD    //表示后面跟随一个大写字母

//初始化键盘线程
int fn_StartKeyboardThread();
//其它读键盘函数
unsigned char fn_ReadKey();
unsigned char fn_ReadUKey();
int fn_ClearKeyBuff();
int fn_ClearUKeyBuff();
//结束键盘线程
int fn_EndKeyboardThread();
#endif
