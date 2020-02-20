#ifndef SECSCREEN_H
#define SECSCREEN_H
#include "sendcmd.h"

class secscreen
{
public:
    secscreen();
    void showPic();// 显示底图（只有底图，没有文字）
    void showAll(QString strtemp,int PosX,int PosY,int FontSize);// 显示背景和文字（有底图，也有文字）
    void showAppendTxt(QString strtemp,int PosX,int PosY,int FontSize);// 在原来显示的内容上追加文字
private:
    int isConnected;//是否启动
    sendCmd * scmd_secscreen;//副屏启动
};

#endif // SECSCREEN_H
