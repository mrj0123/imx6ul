#ifndef NKQRSACN_H
#define NKQRSACN_H

#define MAX_QRBUFFER_LEN 0x400 //�����̻���
#define MAX_QRCODE_LEN 0x101   //����ά�봮
#define KEY_MYUPPER 0xFD    //��ʾ�������һ����д��ĸ

#define QR_DEV_PATH "/dev/input/event"   //difference is possible
//����������������������������������ض���end������������������������///////

//��ʼ��ɨ���߳�
int fn_StartQRScanThread();
//����ɨ���߳�
int fn_EndQRScanThread();
//��ʼɨ�躯��
//unsigned char fn_QRScan();
//���ɨ�軺��
int fn_ClearQRBuff();
//ɨ���ά�룬�����̵߳���
int fn_GetQRCode(char * qrCodeBuf);

//��ȡ�豸��ţ��ڲ����ã�
int find_event();
char fn_ConvertBase64Key(int orgkey,int isupper);

#endif
