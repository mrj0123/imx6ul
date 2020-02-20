#ifndef NKQRSACN_H
#define NKQRSACN_H

#define MAX_QRBUFFER_LEN 0x400 //最大键盘缓存
#define MAX_QRCODE_LEN 0x101   //最大二维码串
#define KEY_MYUPPER 0xFD    //表示后面跟随一个大写字母

#define QR_DEV_PATH "/dev/input/event"   //difference is possible
//——————————————键盘相关定义end————————————///////

//初始化扫描线程
int fn_StartQRScanThread();
//结束扫描线程
int fn_EndQRScanThread();
//开始扫描函数
//unsigned char fn_QRScan();
//获取设备编号（内部调用）
//int find_event();

int fn_getLastTime();

#endif
