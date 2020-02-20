#ifndef LCD_H
#define LCD_H

#include "a.h"

/*******************************************************************************************************/
//显示类命令，具体操作见程序中的说明
/*******************************************************************************************************/
#define   ADDR8576     	0x70		   //选择PCF8576,即LCD
#define   OPERLCD      	0xE0		   //操作员面地址
#define   USERLCD      	0xE1           //用户面地址
#define   NUMADVER     	0x01		   //显示机器号及程序版本号
#define   ERRORNUM     	0x02		   //显示系统错误编号
#define   FULLSCRN     	0x03		   //全屏显示相同数，
#define   EXCDHOUR     	0x04		   //黑名单超限时间提示
#define   BADERROR     	0x05		   //严重的错误,机器将死机
#define   SCOLLDIS     	0x06		   //滚动显示第1个数
#define   SCOLLDS1     	0x07		   //滚动显示第2个数
#define   SCOLLDS2     	0x08		   //滚动显示第3个数
#define   SCOLLDS3     	0x09		   //滚动显示第4个数
#define   SCOLLDS4     	0x0A		   //滚动显示第5个数
#define   SCOLLDS5     	0x0B		   //滚动显示第6个数
#define   SCOLLDS6     	0x0C		   //滚动显示第7个数
#define   MACHSTAT     	0x0D		   //启动机器“米”字显示
#define   SIMPLESET    	0x0E		   //单品种单价设置进入和完成时界面
#define   OPENPRICE    	0x0F		   //单品种单价设置闪烁界面：点亮5位数
#define   CLOSEPRICE   	0x10		   //单品种单价设置闪烁界面：关尚未输入的数
#define   MULTISET	 	0x11		   //多品种单价设置进入时界面
#define   OPENBIT      	0x12		   //显示LCD指定上指定位
#define   CLOSEBIT     	0x13		   //关闭LCD指定上指定位
#define   PSWDINPUT    	0x14		   //提示等待密码输入
#define   NORMALDS	 	0x15		   //正常状态显示：显示时间

#define   OPENSECOND   	0x16		   //打开秒显示
#define   CLOSESECOND  	0x17  		   //关闭秒显示
#define   TRADEREADY	0x18		   //提示“请放卡”
#define	  INPUTMONEY	0x19
#define   CSMDIS		0X1A		   //消费金额显示
#define   REMDIS		0X1B		   //余额显示
#define   RADDIS     	0x1C		   //放射状显示
#define   PSWDBIT    	0x1D		   //提示等待密码输入
#define   OPERERROR		0x1E		   //操作错误
#define   TRADEAGAIN	0x1F		   //提示“重放卡”
#define   GROUPINPUT1   0x20		   //分组方式下，显示输入金额
#define   GROUPINPUT2	0x21		   //分组方式下，显示输入总金额
#define	  ALOWSET		0x22
#define	  CASHDIS		0x23		   //出纳机正常显示
#define	  CASHTYPE		0x24

//LCD 显示码表
/*
const BYTE  NumberTable[25] ={0x5f,0x03,0x3e,0x2f,0x63,0x6d,0x7d,0x07,0x7f,0x6f,//LCD显示码表：0-9
							  0x77,0x79,0x5c,0x3b,0x7c,0x74,					//A-F
							  0X5D,0x73,0x58,0x76,0x5B,0x31,0x20,0x24,0x2c};	//G H L P U n - 二 三
*/
int lcd_module_init();  //初始化液晶
int lcd_clear(void);    //清除液晶显示
//多字节显示
int lcd_bytes_display(BYTE DisStartAddr, BYTE *DataStartPtr, BYTE DataNum, BYTE LcdSlaveAddr);
//单字节显示
int lcd_byte_display(BYTE DisAddr,BYTE DisData,BYTE LcdSlaveAddr);
void Remind_info_display(BYTE Command,BYTE DisData);
void Multi_word_display(BYTE Command,DWORD DisDword);
//显示版本号
//第一排显示序列号
//第二排显示版本号
void Version_display();
void SerialNo_display();
//显示时间
//第一排显示时间（hh:nn）
//第二排显示日期（mm-dd）
void DateTime_display();
//显示输入金额
//第一排显示PAUSE
//第二排显示Inputmoney
void InputMoney_display(int InputMoney);
//滚0等待刷卡 ZeroPos表示显示滚0的位置（0~4）
void WaitForCard_display(int InputMoney,int ZeroPos);
//消费成功，第一排显示RemainMoney表示余额，第二排ConsumeMoney表示消费额
void ConsumeSuccess_display(int RemainMoney,int ConsumeMoney);
//消费失败，错误提示
void ErrorCode_display(int Errorcode);

#endif
