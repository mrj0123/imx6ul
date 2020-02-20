#ifndef NKKEYBOARD_H
#define NKKEYBOARD_H

//����������������������������������ض���begin������������������������///////
//��ֵ����
#define NUM1_KEY 79      //1��
#define NUM2_KEY 80      //2��
#define NUM3_KEY 81      //3��
#define NUM4_KEY 75      //4��
#define NUM5_KEY 76      //5��
#define NUM6_KEY 77      //6��
#define NUM7_KEY 71      //7��
#define NUM8_KEY 72      //8��
#define NUM9_KEY 73      //9��
#define NUM0_KEY 82      //0��

#define A_KEY 30         //A��,ͬʱ��ʾ�����˵�������ʾ��������˵�
#define B_KEY 48         //B��
#define C_KEY 46         //C��
#define D_KEY 32         //D��
#define E_KEY 18         //-����E������

#define RETURN_KEY 96    //�س�������ʾ�������������ȴ�ˢ����ɨ������
#define CLEAR_KEY 111    //�˸������ʾɾ��ԭ����
#define FIXED_KEY 69     //�̶���
#define CASH_KEY  78     //�ֽ����+��������

#define POINT_KEY 32     //.����D������
#define ADD_KEY   78     //+�����ֽ������
#define SUB_KEY   18     //-����E������

#define UNUM1_KEY 2     //�û���1��
#define UNUM2_KEY 3     //�û���2��
#define UNUM3_KEY 4     //�û���3��
#define UNUM4_KEY 5     //�û���4��
#define UNUM5_KEY 6     //�û���5��
#define UNUM6_KEY 7     //�û���6��
#define UNUM7_KEY 8     //�û���7��
#define UNUM8_KEY 9     //�û���8��ls
#define UNUM9_KEY 10     //�û���9��
#define UNUM0_KEY 11     //�û���0��

#define UCLEAR_KEY 14    //�û����˸��
#define UENTER_KEY 28    //�û���س���

#define NO_KEY    0xFF     //δ������ֵ
#define EXIT_KEY  0xFE     //�˳�

#define MAX_KEYBUFFER_LEN 0x500 //�����̻���
#define MAX_UKEYBUFFER_LEN 0x500//����û�����̻���
//����������������������������������ض���end������������������������///////

#define OPER_SIDE 0         //����Ա�����
#define USER_SIDE 1         //�û������

#define KEY_MYUPPER 0xFD    //��ʾ�������һ����д��ĸ

//��ʼ�������߳�
int fn_StartKeyboardThread();
//���������̺���
unsigned char fn_ReadKey();
unsigned char fn_ReadUKey();
int fn_ClearKeyBuff();
int fn_ClearUKeyBuff();
//���������߳�
int fn_EndKeyboardThread();
#endif
