#include "secscreen.h"

secscreen::secscreen()
{
    char sersec_path[]=SECSCREEN_PATH_S;
    char clisec_path[]=SECSCREEN_PATH_C;
    scmd_secscreen = new sendCmd(sersec_path,clisec_path);
    isConnected = 0;
}
void secscreen::showPic()
{
    if (isConnected==0)
    {
        scmd_secscreen->conn_afunix();//启动连接
        isConnected=1;
    }
    QJsonObject jsonsend;
    jsonsend.insert("BGPicture","/usr/local/nkty/images/bg.jpg");
    //jsonsend.insert("BGPicture","/home/root/display/000.jpg");
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = SECSCREEN_SHOWPIC_CMD;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_secscreen->send_Command(&Cmd,jsonsend,retJson);
    /*if(Cmd != 10000+SECSCREEN_SHOWPIC_CMD){
        return 1;
    }else{
        return 0;
    }*/
}
void secscreen::showAll(QString strtemp,int PosX,int PosY,int FontSize)
{
    if (isConnected==0)
    {
        scmd_secscreen->conn_afunix();//启动连接
        isConnected=1;
    }
    QJsonObject jsonsend;
    QByteArray utf8String = strtemp.toUtf8();
    QLatin1String lstr=QLatin1String(utf8String);
    jsonsend.insert("Text",lstr);
    jsonsend.insert("PosX",PosX);
    jsonsend.insert("PosY",PosY);
    jsonsend.insert("FontSize",FontSize);
    jsonsend.insert("BGPicture","/usr/local/nkty/images/bg.jpg");
    //jsonsend.insert("BGPicture","/home/root/display/000.jpg");
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = SECSCREEN_SHOWALL_CMD;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_secscreen->send_Command(&Cmd,jsonsend,retJson);
    /*if(Cmd != 10000+SECSCREEN_SHOWALL_CMD){
        return 1;
     }else{
        return 0;
    }*/
}
void secscreen::showAppendTxt(QString strtemp,int PosX,int PosY,int FontSize)
{
    if (isConnected==0)
    {
        scmd_secscreen->conn_afunix();//启动连接
        isConnected=1;
    }
    QJsonObject jsonsend;
    QByteArray utf8String = strtemp.toUtf8();
    QLatin1String lstr=QLatin1String(utf8String);
    jsonsend.insert("Text",lstr);
    jsonsend.insert("PosX",PosX);
    jsonsend.insert("PosY",PosY);
    jsonsend.insert("FontSize",FontSize);
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = SECSCREEN_APPENDTXT_CMD;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_secscreen->send_Command(&Cmd,jsonsend,retJson);
    /*if(Cmd != 10000+SECSCREEN_APPENDTXT_CMD){
        return 1;
    }else{
        return 0;
    }*/
}
