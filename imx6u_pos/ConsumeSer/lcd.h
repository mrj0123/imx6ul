#ifndef LCD_H
#define LCD_H

#include "a.h"

/*******************************************************************************************************/
//��ʾ�������������������е�˵��
/*******************************************************************************************************/
#define   ADDR8576     	0x70		   //ѡ��PCF8576,��LCD
#define   OPERLCD      	0xE0		   //����Ա���ַ
#define   USERLCD      	0xE1           //�û����ַ
#define   NUMADVER     	0x01		   //��ʾ�����ż�����汾��
#define   ERRORNUM     	0x02		   //��ʾϵͳ������
#define   FULLSCRN     	0x03		   //ȫ����ʾ��ͬ����
#define   EXCDHOUR     	0x04		   //����������ʱ����ʾ
#define   BADERROR     	0x05		   //���صĴ���,����������
#define   SCOLLDIS     	0x06		   //������ʾ��1����
#define   SCOLLDS1     	0x07		   //������ʾ��2����
#define   SCOLLDS2     	0x08		   //������ʾ��3����
#define   SCOLLDS3     	0x09		   //������ʾ��4����
#define   SCOLLDS4     	0x0A		   //������ʾ��5����
#define   SCOLLDS5     	0x0B		   //������ʾ��6����
#define   SCOLLDS6     	0x0C		   //������ʾ��7����
#define   MACHSTAT     	0x0D		   //�����������ס�����ʾ
#define   SIMPLESET    	0x0E		   //��Ʒ�ֵ������ý�������ʱ����
#define   OPENPRICE    	0x0F		   //��Ʒ�ֵ���������˸���棺����5λ��
#define   CLOSEPRICE   	0x10		   //��Ʒ�ֵ���������˸���棺����δ�������
#define   MULTISET	 	0x11		   //��Ʒ�ֵ������ý���ʱ����
#define   OPENBIT      	0x12		   //��ʾLCDָ����ָ��λ
#define   CLOSEBIT     	0x13		   //�ر�LCDָ����ָ��λ
#define   PSWDINPUT    	0x14		   //��ʾ�ȴ���������
#define   NORMALDS	 	0x15		   //����״̬��ʾ����ʾʱ��

#define   OPENSECOND   	0x16		   //������ʾ
#define   CLOSESECOND  	0x17  		   //�ر�����ʾ
#define   TRADEREADY	0x18		   //��ʾ����ſ���
#define	  INPUTMONEY	0x19
#define   CSMDIS		0X1A		   //���ѽ����ʾ
#define   REMDIS		0X1B		   //�����ʾ
#define   RADDIS     	0x1C		   //����״��ʾ
#define   PSWDBIT    	0x1D		   //��ʾ�ȴ���������
#define   OPERERROR		0x1E		   //��������
#define   TRADEAGAIN	0x1F		   //��ʾ���طſ���
#define   GROUPINPUT1   0x20		   //���鷽ʽ�£���ʾ������
#define   GROUPINPUT2	0x21		   //���鷽ʽ�£���ʾ�����ܽ��
#define	  ALOWSET		0x22
#define	  CASHDIS		0x23		   //���ɻ�������ʾ
#define	  CASHTYPE		0x24

//LCD ��ʾ���
/*
const BYTE  NumberTable[25] ={0x5f,0x03,0x3e,0x2f,0x63,0x6d,0x7d,0x07,0x7f,0x6f,//LCD��ʾ���0-9
							  0x77,0x79,0x5c,0x3b,0x7c,0x74,					//A-F
							  0X5D,0x73,0x58,0x76,0x5B,0x31,0x20,0x24,0x2c};	//G H L P U n - �� ��
*/
int lcd_module_init();  //��ʼ��Һ��
int lcd_clear(void);    //���Һ����ʾ
//���ֽ���ʾ
int lcd_bytes_display(BYTE DisStartAddr, BYTE *DataStartPtr, BYTE DataNum, BYTE LcdSlaveAddr);
//���ֽ���ʾ
int lcd_byte_display(BYTE DisAddr,BYTE DisData,BYTE LcdSlaveAddr);
void Remind_info_display(BYTE Command,BYTE DisData);
void Multi_word_display(BYTE Command,DWORD DisDword);
//��ʾ�汾��
//��һ����ʾ���к�
//�ڶ�����ʾ�汾��
void Version_display();
void SerialNo_display();
//��ʾʱ��
//��һ����ʾʱ�䣨hh:nn��
//�ڶ�����ʾ���ڣ�mm-dd��
void DateTime_display();
//��ʾ������
//��һ����ʾPAUSE
//�ڶ�����ʾInputmoney
void InputMoney_display(int InputMoney);
//��0�ȴ�ˢ�� ZeroPos��ʾ��ʾ��0��λ�ã�0~4��
void WaitForCard_display(int InputMoney,int ZeroPos);
//���ѳɹ�����һ����ʾRemainMoney��ʾ���ڶ���ConsumeMoney��ʾ���Ѷ�
void ConsumeSuccess_display(int RemainMoney,int ConsumeMoney);
//����ʧ�ܣ�������ʾ
void ErrorCode_display(int Errorcode);

#endif
