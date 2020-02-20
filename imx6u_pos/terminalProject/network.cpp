#include "network.h"
#include "ui_network.h"
#include "afunix_udp.h"
#include "qmessagebox.h"
#include <QStringList>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QScrollBar>
#include <QDateTime>


#include "getnetsettingthread.h"

#include "cglobal.h"
#include "getnetsetting.h"

network::network(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::network)
{
    ui->setupUi(this);
    networkFlag=0;//有线连接标志，0：关，1：开
    wifiFlag=0;//无线连接标志，0：关，1：开
    dhcpFlag=0;//DHCP标志，0：关，1：开
    gprsFlag=0;//GPRS标志，0：关，1：开
    //定时获取页面信息
    //获取wifi--SSID
    timerwifi =new QTimer(this);
    model=new QStringListModel;
    ssids_model =new QStandardItemModel();

    process=new QProcess;

    timerGetServer =new QTimer(this);
    timerGetNetwork =new QTimer(this);
    timerGetWifi =new QTimer(this);
    timerGprs =new QTimer(this);

    //扫描二维码
    //ui->pushButton_scavenging->hide();
    timerScanqrcode = new QTimer(this);

    init();//整体--初始化函数
    //全屏
    setWindowFlags(Qt::FramelessWindowHint);
    showFullScreen();
}

network::~network()
{
    delete ui;
    delete model;
    delete ssids_model;
    delete timerwifi;   
    delete process;
    delete timerScanqrcode;
    delete timerGetServer;
    delete timerGetNetwork;
    delete timerGetWifi;
    delete timerGprs;
}
void network::init()
{
    //界面标签显示状态
    ui->label_tips->hide();//提示语
    ui->tableView_SSID->hide();//SSID下拉框
    //ui->lineEdit_password->setEchoMode(QLineEdit::Password);//设置输入密码框

    ui->lineEdit_ipaddress->installEventFilter(this);//在窗体上为其安装过滤器
    ui->lineEdit_netmask->installEventFilter(this);//在窗体上为其安装过滤器
    ui->lineEdit_gateway->installEventFilter(this);//在窗体上为其安装过滤器
    ui->lineEdit_DNS1->installEventFilter(this);//在窗体上为其安装过滤器
    ui->lineEdit_DNS2->installEventFilter(this);//在窗体上为其安装过滤器
    ui->lineEdit_SSID->installEventFilter(this);//在窗体上为其安装过滤器
    //ui->lineEdit_password->installEventFilter(this);//在窗体上为其安装过滤器
    ui->lineEdit_ip->installEventFilter(this);//在窗体上为其安装过滤器
    ui->lineEdit_port->installEventFilter(this);//在窗体上为其安装过滤器
    ui->lineEdit_spareIp->installEventFilter(this);//在窗体上为其安装过滤器
    ui->lineEdit_sparePort->installEventFilter(this);//在窗体上为其安装过滤器
    //ui->lineEdit_ip->setFocus();//设置焦点
    ui->pushButton_network_open->hide();
    ui->pushButton_wifi_close->hide();
    ui->pushButton_wifi_open->hide();
    ui->pushButton_gprs_open->hide();
    ui->groupBox_mask->hide();
    ui->groupBox_cashierBg->hide();
    maskFlag=0;//遮罩标志

    //隐藏
    //ui->groupBox->hide();
    ui->pushButton_search_wifi->hide();
    ui->pushButton_submit2->hide();
    ui->label_wifi_open->hide();
    //ui->groupBox_2->hide();
    ui->pushButton_submit3->hide();
    /*ui->pushButton_submit4->hide();
    ui->pushButton_cancel4->hide();

    ui->pushButton_submit1->hide();
    ui->pushButton_cancel1->hide();*/

    //隐藏MAC地址
    ui->label_MAC->hide();
    ui->lineEdit_MAC->hide();

    //信息初始化
    QTimer::singleShot(100,this,SLOT(initDate()));
    //获取SSID
    connect(timerwifi,SIGNAL(timeout()),this,SLOT(getWifiSsid()));
    ui->lineEdit_MAC->setEnabled(false);//Mac地址禁止编辑
    //扫描二维码
    connect(timerScanqrcode,SIGNAL(timeout()),this,SLOT(queScanqrcodeRet()));

    //获取服务器参数
    connect(timerGetServer,SIGNAL(timeout()),this,SLOT(getServerInfo()));
    //获取有线网络参数
    connect(timerGetNetwork,SIGNAL(timeout()),this,SLOT(getNetworkInfo()));
    //获取无线网络参数
    connect(timerGetWifi,SIGNAL(timeout()),this,SLOT(getWifiInfo()));
    //获取GPRS参数
    connect(timerGprs,SIGNAL(timeout()),this,SLOT(getGprsInfo()));
}
bool network::eventFilter(QObject *watched, QEvent *event)
{
    if( watched==ui->lineEdit_ipaddress)       //首先判断控件(这里指 lineEdit_ipaddress)
    {
        if (event->type()==QEvent::KeyPress)     //判断控件的具体事件 (这里指获得点击事件)
        {
           QKeyEvent *keyevent=static_cast<QKeyEvent *>(event);
           if(keyevent->isAutoRepeat()){
               ui->lineEdit_password->setText(ui->lineEdit_ipaddress->text());
               return true;
           }
           int keyValue = keyevent->key();
           if(keyValue==16777264)
           {//16777264,定义为小数点
               ui->lineEdit_ipaddress->setText(ui->lineEdit_ipaddress->text().trimmed()+".");
               return true;
           }
           else if(keyValue==42)
           {//42,返回键，定义为退格键
               ui->lineEdit_ipaddress->backspace();
               return true;
           }
           else if(keyValue==16777265 || keyValue==16777216)
           {//16777265,菜单键;//16777216,取消键
               ui->lineEdit_ipaddress->setText(ui->lineEdit_ipaddress->text());
               return true;
           }
           else if(keyValue==16777220)
           {//16777220,确定键
               setNetworkInfomation();
               return true;
           }
           else
           {
               return QWidget::eventFilter(watched,event);     // 最后将事件交给上层对话框
           }
        }
    }
    else if( watched==ui->lineEdit_netmask)       //首先判断控件(这里指 lineEdit_netmask)
    {
        if (event->type()==QEvent::KeyPress)     //判断控件的具体事件 (这里指获得点击事件)
        {
           QKeyEvent *keyevent=static_cast<QKeyEvent *>(event);
           if(keyevent->isAutoRepeat()){
               ui->lineEdit_password->setText(ui->lineEdit_netmask->text());
               return true;
           }
           int keyValue = keyevent->key();
           if(keyValue==16777264)
           {//16777264,定义为小数点
               ui->lineEdit_netmask->setText(ui->lineEdit_netmask->text().trimmed()+".");
               return true;
           }
           else if(keyValue==42)
           {//42,返回键，定义为退格键
               ui->lineEdit_netmask->backspace();
               return true;
           }
           else if(keyValue==16777265 || keyValue==16777216)
           {//16777265,菜单键;//16777216,取消键
               ui->lineEdit_netmask->setText(ui->lineEdit_netmask->text());
               return true;
           }
           else if(keyValue==16777220)
           {//16777220,确定键
               setNetworkInfomation();
               return true;
           }
           else
           {
               return QWidget::eventFilter(watched,event);     // 最后将事件交给上层对话框
           }
        }
    }
    else if( watched==ui->lineEdit_gateway)       //首先判断控件(这里指 lineEdit_gateway)
    {
        if (event->type()==QEvent::KeyPress)     //判断控件的具体事件 (这里指获得点击事件)
        {
           QKeyEvent *keyevent=static_cast<QKeyEvent *>(event);
           if(keyevent->isAutoRepeat()){
               ui->lineEdit_password->setText(ui->lineEdit_gateway->text());
               return true;
           }
           int keyValue = keyevent->key();
           if(keyValue==16777264)
           {//16777264,定义为小数点
               ui->lineEdit_gateway->setText(ui->lineEdit_gateway->text().trimmed()+".");
               return true;
           }
           else if(keyValue==42)
           {//42,返回键，定义为退格键
               ui->lineEdit_gateway->backspace();
               return true;
           }
           else if(keyValue==16777265 || keyValue==16777216)
           {//16777265,菜单键;//16777216,取消键
               ui->lineEdit_gateway->setText(ui->lineEdit_gateway->text());
               return true;
           }
           else if(keyValue==16777220)
           {//16777220,确定键
               setNetworkInfomation();
               return true;
           }
           else
           {
               return QWidget::eventFilter(watched,event);     // 最后将事件交给上层对话框
           }
        }
    }
    else if( watched==ui->lineEdit_DNS1)       //首先判断控件(这里指 lineEdit_DNS1)
    {
        if (event->type()==QEvent::KeyPress)     //判断控件的具体事件 (这里指获得点击事件)
        {
           QKeyEvent *keyevent=static_cast<QKeyEvent *>(event);
           if(keyevent->isAutoRepeat()){
               ui->lineEdit_password->setText(ui->lineEdit_DNS1->text());
               return true;
           }
           int keyValue = keyevent->key();
           if(keyValue==16777264)
           {//16777264,定义为小数点
               ui->lineEdit_DNS1->setText(ui->lineEdit_DNS1->text().trimmed()+".");
               return true;
           }
           else if(keyValue==42)
           {//42,返回键，定义为退格键
               ui->lineEdit_DNS1->backspace();
               return true;
           }
           else if(keyValue==16777265 || keyValue==16777216)
           {//16777265,菜单键;//16777216,取消键
               ui->lineEdit_DNS1->setText(ui->lineEdit_DNS1->text());
               return true;
           }
           else if(keyValue==16777220)
           {//16777220,确定键
               setNetworkInfomation();
               return true;
           }
           else
           {
               return QWidget::eventFilter(watched,event);     // 最后将事件交给上层对话框
           }
        }
    }
    else if( watched==ui->lineEdit_DNS2)       //首先判断控件(这里指 lineEdit_DNS2)
    {
        if (event->type()==QEvent::KeyPress)     //判断控件的具体事件 (这里指获得点击事件)
        {
           QKeyEvent *keyevent=static_cast<QKeyEvent *>(event);
           if(keyevent->isAutoRepeat()){
               ui->lineEdit_password->setText(ui->lineEdit_DNS1->text());
               return true;
           }
           int keyValue = keyevent->key();
           if(keyValue==16777264)
           {//16777264,定义为小数点
               ui->lineEdit_DNS2->setText(ui->lineEdit_DNS2->text().trimmed()+".");
               return true;
           }
           else if(keyValue==42)
           {//42,返回键，定义为退格键
               ui->lineEdit_DNS2->backspace();
               return true;
           }
           else if(keyValue==16777265 || keyValue==16777216)
           {//16777265,菜单键;//16777216,取消键
               ui->lineEdit_DNS2->setText(ui->lineEdit_DNS2->text());
               return true;
           }
           else if(keyValue==16777220)
           {//16777220,确定键
               setNetworkInfomation();
               return true;
           }
           else
           {
               return QWidget::eventFilter(watched,event);     // 最后将事件交给上层对话框
           }
        }
    }
    else if( watched==ui->lineEdit_SSID)       //首先判断控件(这里指 lineEdit_SSID)
    {
        if (event->type()==QEvent::KeyPress)     //判断控件的具体事件 (这里指获得点击事件)
        {
           QKeyEvent *keyevent=static_cast<QKeyEvent *>(event);
           if(keyevent->isAutoRepeat()){
               ui->lineEdit_password->setText(ui->lineEdit_SSID->text());
               return true;
           }
           int keyValue = keyevent->key();
           if(keyValue==16777264)
           {//16777264,定义为小数点
               ui->lineEdit_SSID->setText(ui->lineEdit_SSID->text().trimmed()+".");
               return true;
           }
           else if(keyValue==42)
           {//42,返回键，定义为退格键
               ui->lineEdit_SSID->backspace();
               return true;
           }
           else if(keyValue==16777265 || keyValue==16777216)
           {//16777265,菜单键;//16777216,取消键
               ui->lineEdit_SSID->setText(ui->lineEdit_SSID->text());
               return true;
           }
           else if(keyValue==16777220)
           {//16777220,确定键
               setWifiSetting();
               return true;
           }
           else
           {
               return QWidget::eventFilter(watched,event);     // 最后将事件交给上层对话框
           }
        }
    }
    else if( watched==ui->lineEdit_password)       //首先判断控件(这里指 lineEdit_password)
    {
        if (event->type()==QEvent::KeyPress)     //判断控件的具体事件 (这里指获得点击事件)
        {
           QKeyEvent *keyevent=static_cast<QKeyEvent *>(event);
           if(keyevent->isAutoRepeat()){
               ui->lineEdit_password->setText(ui->lineEdit_password->text());
               return true;
           }
           int keyValue = keyevent->key();           
           if(keyValue==16777264)
           {//16777264,定义为小数点
               ui->lineEdit_password->setText(ui->lineEdit_password->text().trimmed()+".");
               return true;
           }
           else if(keyValue==42)
           {//42,返回键，定义为退格键
               ui->lineEdit_password->backspace();
               return true;
           }
           else if(keyValue==16777265 || keyValue==16777216)
           {//16777265,菜单键;//16777216,取消键
               ui->lineEdit_password->setText(ui->lineEdit_password->text());
               return true;
           }
           else if(keyValue==16777220)
           {//16777220,确定键
               setWifiSetting();
               return true;
           }
           else
           {
               return QWidget::eventFilter(watched,event);     // 最后将事件交给上层对话框
           }
        }
    }    
    else if( watched==ui->lineEdit_ip)       //首先判断控件(这里指 lineEdit_password)
    {
        if (event->type()==QEvent::KeyPress)     //判断控件的具体事件 (这里指获得点击事件)
        {
           QKeyEvent *keyevent=static_cast<QKeyEvent *>(event);
           if(keyevent->isAutoRepeat()){
               ui->lineEdit_password->setText(ui->lineEdit_ip->text());
               return true;
           }
           int keyValue = keyevent->key();
           if(keyValue==16777264)
           {//16777264,定义为小数点
               ui->lineEdit_ip->setText(ui->lineEdit_ip->text().trimmed()+".");
               return true;
           }
           else if(keyValue==42)
           {//42,返回键，定义为退格键
               ui->lineEdit_ip->backspace();
               return true;
           }
           else if(keyValue==16777265 || keyValue==16777216)
           {//16777265,菜单键;//16777216,取消键
               ui->lineEdit_ip->setText(ui->lineEdit_ip->text());
               return true;
           }
           else if(keyValue==16777220)
           {//16777220,确定键
               setServerSetting();
               return true;
           }
           else
           {
               return QWidget::eventFilter(watched,event);     // 最后将事件交给上层对话框
           }
        }
    }
    else if( watched==ui->lineEdit_port)       //首先判断控件(这里指 lineEdit_port)
    {
        if (event->type()==QEvent::KeyPress)     //判断控件的具体事件 (这里指获得点击事件)
        {
           QKeyEvent *keyevent=static_cast<QKeyEvent *>(event);
           if(keyevent->isAutoRepeat()){
               ui->lineEdit_password->setText(ui->lineEdit_port->text());
               return true;
           }
           int keyValue = keyevent->key();
           if(keyValue==16777264)
           {//16777264,定义为小数点
               ui->lineEdit_port->setText(ui->lineEdit_port->text().trimmed()+".");
               return true;
           }
           else if(keyValue==42)
           {//42,返回键，定义为退格键
               ui->lineEdit_port->backspace();
               return true;
           }
           else if(keyValue==16777265 || keyValue==16777216)
           {//16777265,菜单键;//16777216,取消键
               ui->lineEdit_port->setText(ui->lineEdit_port->text());
               return true;
           }
           else if(keyValue==16777220)
           {//16777220,确定键
               setServerSetting();
               return true;
           }
           else
           {
               return QWidget::eventFilter(watched,event);     // 最后将事件交给上层对话框
           }
        }
    }
    else if( watched==ui->lineEdit_spareIp)       //首先判断控件(这里指 lineEdit_spareIp)
    {
        if (event->type()==QEvent::KeyPress)     //判断控件的具体事件 (这里指获得点击事件)
        {
           QKeyEvent *keyevent=static_cast<QKeyEvent *>(event);
           if(keyevent->isAutoRepeat()){
               ui->lineEdit_password->setText(ui->lineEdit_spareIp->text());
               return true;
           }
           int keyValue = keyevent->key();
           if(keyValue==16777264)
           {//16777264,定义为小数点
               ui->lineEdit_spareIp->setText(ui->lineEdit_spareIp->text().trimmed()+".");
               return true;
           }
           else if(keyValue==42)
           {//42,返回键，定义为退格键
               ui->lineEdit_spareIp->backspace();
               return true;
           }
           else if(keyValue==16777265 || keyValue==16777216)
           {//16777265,菜单键;//16777216,取消键
               ui->lineEdit_spareIp->setText(ui->lineEdit_spareIp->text());
               return true;
           }
           else if(keyValue==16777220)
           {//16777220,确定键
               setServerSetting();
               return true;
           }
           else
           {
               return QWidget::eventFilter(watched,event);     // 最后将事件交给上层对话框
           }
        }
    }
    else if( watched==ui->lineEdit_sparePort)       //首先判断控件(这里指 lineEdit_sparePort)
    {
        if (event->type()==QEvent::KeyPress)     //判断控件的具体事件 (这里指获得点击事件)
        {
           QKeyEvent *keyevent=static_cast<QKeyEvent *>(event);
           if(keyevent->isAutoRepeat()){
               ui->lineEdit_password->setText(ui->lineEdit_sparePort->text());
               return true;
           }
           int keyValue = keyevent->key();
           if(keyValue==16777264)
           {//16777264,定义为小数点
               ui->lineEdit_sparePort->setText(ui->lineEdit_sparePort->text().trimmed()+".");
               return true;
           }
           else if(keyValue==42)
           {//42,返回键，定义为退格键
               ui->lineEdit_sparePort->backspace();
               return true;
           }
           else if(keyValue==16777265 || keyValue==16777216)
           {//16777265,菜单键;//16777216,取消键
               ui->lineEdit_sparePort->setText(ui->lineEdit_sparePort->text());
               return true;
           }
           else if(keyValue==16777220)
           {//16777220,确定键
               setServerSetting();
               return true;
           }
           else
           {

               return QWidget::eventFilter(watched,event);     // 最后将事件交给上层对话框
           }
        }
    }
    return QWidget::eventFilter(watched,event);
}
void network::initDate()
{
    QDateTime time = QDateTime::currentDateTime();
    myPtf("start initDate! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());

    //获取UI版本号
    versions();
    //获取终端唯一识别码二维码
    getQcode();
    time = QDateTime::currentDateTime();
    myPtf("end initDate! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
}
void network::on_tabWidget_tabBarClicked(int index)
{
    if(index==0){
        //获取UI版本号
        versions();
        //获取终端唯一识别码二维码
        getQcode();

        timerGetServer->stop();
        timerGetNetwork->stop();
        timerGetWifi->stop();
        timerGprs->stop();
    }else if(index==1){
        //获取服务器参数--初始化函数
        getServerSetting();
        timerGetNetwork->stop();
        timerGetWifi->stop();
        timerGprs->stop();
    }else if(index==2){
        //获取有线网络参数--初始化函数
        getNetworkInfomation();
        timerGetServer->stop();
        timerGetWifi->stop();
        timerGprs->stop();
    }else if(index==3){
        //获取无线网络参数--初始化函数
        getWifiSetting();
        timerGetServer->stop();
        timerGetNetwork->stop();
        timerGprs->stop();
    }else if(index==4){
        //获取GPRS状态--初始化函数
        getGprsStatus();
        timerGetServer->stop();
        timerGetNetwork->stop();
        timerGetWifi->stop();
    }
}
void network::loadShow()
{
    ui->groupBox_cashierBg->show();
    ui->label_info->setText("信息加载中...");
    ui->pushButton_cancels->hide();
}
void network::loadHide()
{
    ui->groupBox_cashierBg->hide();
    ui->label_info->setText("");
    ui->pushButton_cancels->show();
}
void network::versions()
{
    //获取UI版本号
    ui->label_uiVersion->setText(uiVersion_network);
    //获取consumeser服务版本号
    ui->label_consumeserVersion->setText(consumeserVersion_network);
    //获取scanqrser服务版本号
    ui->label_scanqrserVersion->setText(scanqrserVersion_network);
    //获取secscreen服务版本号
    ui->label_secscreenVersion->setText(secscreenVersion_network);
    //获取network服务版本号
    ui->label_networkVersion->setText(networkVersion_network);
    //获取终端唯一识别号
    ui->label_SSIDCode->setText(termCode_network);
}
void network::getServerSetting()
{
    //获取服务器参数--初始化函数
    /*QJsonObject jsonsend;
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = SERVER_SETTING_GET_CMD;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_network_network->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+SERVER_SETTING_GET_CMD){
        //数据获取失败，可以尝试重连
        //QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
        /message.exec();
        ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else{
        //QString strShow = "";
        //strShow.append(retJson);
        //QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        //message.exec();
        //解析json字符串
        ui->lineEdit_ip->setText(jsonrecv.value("ipaddress").toString());
        ui->lineEdit_port->setText(jsonrecv.value("port").toString());
        ui->lineEdit_spareIp->setText(jsonrecv.value("ipaddress2").toString());
        ui->lineEdit_sparePort->setText(jsonrecv.value("port2").toString());
    }*/

    fn_StartNetInfoThread(GETSERVERSETTING);
    if(qs_ipaddress1==""){
        loadShow();
    }
    timerGetServer->start(2000);
}
void network::getServerInfo()
{
    //获取服务器参数
    loadHide();
    timerGetServer->stop();
    std::string ipaddress=std::string(CGlobal::g_server_setting[0].serverip);
    int port=CGlobal::g_server_setting[0].serverport;
    std::string ipaddress2=std::string(CGlobal::g_server_setting[1].serverip);
    int port2=CGlobal::g_server_setting[1].serverport;

    qs_ipaddress1=QString::fromStdString(ipaddress);
    QString qs_port=QString::number(port);
    QString qs_ipaddress2=QString::fromStdString(ipaddress2);
    QString qs_port2=QString::number(port2);

    if(qs_ipaddress1==""){
        timerGetServer->start(500);
    }else{
        ui->lineEdit_ip->setText(qs_ipaddress1);
        ui->lineEdit_port->setText(qs_port);
        ui->lineEdit_spareIp->setText(qs_ipaddress2);
        ui->lineEdit_sparePort->setText(qs_port2);
    }
}
void network::getNetworkInfomation()
{    
    //获取有线网络参数--初始化函数
    /*QJsonObject jsonsend;
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = NETWORK_INFOMATION_GET_CMD;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_network_network->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+NETWORK_INFOMATION_GET_CMD){
        //数据获取失败，可以尝试重连
        //QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是aa"+QString::number(Cmd));
        //message.exec();
        ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else{
        //QString strShow = "";
        //strShow.append(retJson);
        //QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        //message.exec();
        //解析json字符串
        ui->lineEdit_ipaddress->setText(jsonrecv.value("ipaddress").toString());
        ui->lineEdit_netmask->setText(jsonrecv.value("netmask").toString());
        ui->lineEdit_gateway->setText(jsonrecv.value("gateway").toString());
        ui->lineEdit_DNS1->setText(jsonrecv.value("dns1").toString());
        ui->lineEdit_DNS2->setText(jsonrecv.value("dns2").toString());
        //ui->lineEdit_MAC->setText(jsonrecv.value("mac").toString());
        QString flag=jsonrecv.value("flag").toString();

        if(flag=="0" || flag=="2"){
            //有线连接--开
            networkFlag=1;
            ui->pushButton_network_open->show();
            ui->pushButton_network_close->hide();
        }else if(flag=="1" || flag=="3"){
            //有线连接--关
            networkFlag=0;
            ui->pushButton_network_open->hide();
            ui->pushButton_network_close->show();
        }
        QString usedhcp=jsonrecv.value("usedhcp").toString();
        if(usedhcp=="0"){
            //DHCP关闭
            dhcpFlag=0;
            ui->pushButton_DHCP_on->hide();
            ui->pushButton_DHCP_off->show();
            ui->lineEdit_ipaddress->setEnabled(true);
            ui->lineEdit_netmask->setEnabled(true);
            ui->lineEdit_gateway->setEnabled(true);
            ui->lineEdit_DNS1->setEnabled(true);
            ui->lineEdit_DNS2->setEnabled(true);
        }else{
            //DHCP开启
            dhcpFlag=1;
            ui->pushButton_DHCP_on->show();
            ui->pushButton_DHCP_off->hide();
            ui->lineEdit_ipaddress->setEnabled(false);
            ui->lineEdit_netmask->setEnabled(false);
            ui->lineEdit_gateway->setEnabled(false);
            ui->lineEdit_DNS1->setEnabled(false);
            ui->lineEdit_DNS2->setEnabled(false);
        }
    }*/
    fn_StartNetInfoThread(GETWIREDSETTING);
    if(qs_ipaddress2==""){
        loadShow();
    }
    timerGetNetwork->start(2000);
}
void network::getNetworkInfo()
{
    //获取有线网络参数
    loadHide();
    timerGetNetwork->stop();
    std::string ipaddress=std::string(CGlobal::g_wired_setting.ipaddress);
    std::string netmask=std::string(CGlobal::g_wired_setting.netmask);
    std::string gateway=std::string(CGlobal::g_wired_setting.gateway);
    std::string mac=std::string(CGlobal::g_wired_setting.mac);
    int netstatus=CGlobal::g_wired_setting.netstatus;
    int dhcpflag=CGlobal::g_wired_setting.dhcpflag;
    std::string dns1=std::string(CGlobal::g_wired_setting.dns1);
    std::string dns2=std::string(CGlobal::g_wired_setting.dns2);

    qs_ipaddress2=QString::fromStdString(ipaddress);
    QString qs_netmask=QString::fromStdString(netmask);
    QString qs_gateway=QString::fromStdString(gateway);
    QString qs_mac=QString::fromStdString(mac);
    QString qs_dns1=QString::fromStdString(dns1);
    QString qs_dns2=QString::fromStdString(dns2);

    if(qs_ipaddress2==""){
        timerGetNetwork->start(500);
    }else{
        ui->lineEdit_ipaddress->setText(qs_ipaddress2);
        ui->lineEdit_netmask->setText(qs_netmask);
        ui->lineEdit_gateway->setText(qs_gateway);
        ui->lineEdit_DNS1->setText(qs_dns1);
        ui->lineEdit_DNS2->setText(qs_dns2);

        if(netstatus==0 || netstatus==2){
            //有线连接--开
            networkFlag=1;
            ui->pushButton_network_open->show();
            ui->pushButton_network_close->hide();
        }else if(netstatus==1 || netstatus==3){
            //有线连接--关
            networkFlag=0;
            ui->pushButton_network_open->hide();
            ui->pushButton_network_close->show();
        }

        if(dhcpflag==0){
            //DHCP关闭
            dhcpFlag=0;
            ui->pushButton_DHCP_on->hide();
            ui->pushButton_DHCP_off->show();
            ui->lineEdit_ipaddress->setEnabled(true);
            ui->lineEdit_netmask->setEnabled(true);
            ui->lineEdit_gateway->setEnabled(true);
            ui->lineEdit_DNS1->setEnabled(true);
            ui->lineEdit_DNS2->setEnabled(true);
        }else{
            //DHCP开启
            dhcpFlag=1;
            ui->pushButton_DHCP_on->show();
            ui->pushButton_DHCP_off->hide();
            ui->lineEdit_ipaddress->setEnabled(false);
            ui->lineEdit_netmask->setEnabled(false);
            ui->lineEdit_gateway->setEnabled(false);
            ui->lineEdit_DNS1->setEnabled(false);
            ui->lineEdit_DNS2->setEnabled(false);
        }
    }
}
void network::getWifiSetting()
{
    //获取无线网络参数--初始化函数
    /*QJsonObject jsonsend;
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = WIFI_STATUS_GET;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_network_network->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+WIFI_STATUS_GET){
        //数据获取失败，可以尝试重连
        //QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
        //message.exec();
        ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
        ui->pushButton_wifi_close->show();
        ui->pushButton_wifi_open->hide();
    }else{
        //QString strShow = "";
        //strShow.append(retJson3);
        //QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        //message.exec();
        //解析json字符串
        ui->lineEdit_SSID->setText(jsonrecv.value("essid").toString());
        ui->lineEdit_password->setText(jsonrecv.value("password").toString());
        QString flag=jsonrecv.value("flag").toString();
        if(flag=="1" || flag=="2"){
            //无线开启
            ui->label_wifi_close->setText("开");
        }else if(flag=="0" || flag=="3"){
            //无线关闭
            ui->label_wifi_close->setText("关");
        }
    }*/
    fn_StartNetInfoThread(GETWIRELESSSETTING);
    if(qs_essidname==""){
        loadShow();
    }
    timerGetWifi->start(2000);
}
void network::getWifiInfo()
{
    //获取无线网络参数
    loadHide();
    timerGetWifi->stop();
    std::string essidname=std::string(CGlobal::g_wireless_setting.essidname);
    std::string password=std::string(CGlobal::g_wireless_setting.password);
    int netstatus=CGlobal::g_wireless_setting.netstatus;

    qs_essidname=QString::fromStdString(essidname);
    QString qs_password=QString::fromStdString(password);

    if(qs_essidname==""){
        timerGetWifi->start(500);
    }else{
        ui->lineEdit_SSID->setText(qs_essidname);
        ui->lineEdit_password->setText(qs_password);

        if(netstatus==1 || netstatus==2){
            //无线开启
            ui->label_wifi_close->setText("开");
        }else if(netstatus==0 || netstatus==3){
            //无线关闭
            ui->label_wifi_close->setText("关");
        }
    }
}
void network::getWifiSsid()
{
    timerwifi->stop();
    //获取SSID--初始化函数
    QJsonObject jsonsend;
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = WIFI_SETTING_GET_CMD;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_network_network->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+WIFI_SETTING_GET_CMD){
        //数据获取失败，可以尝试重连
        /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败bb!,ret是"+QString::number(Cmd));
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else{
        QString strShow = "";
        strShow.append(retJson);
        QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        message.exec();
        //解析json字符串
        //下拉菜单
        QString ssids=jsonrecv.value("ssids").toString();
        QStringList ssids_list = ssids.split(",");
        //model=new QStringListModel;
        model->setStringList(ssids_list);

        //ssids_model =new QStandardItemModel();
        ssids_model->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr("SSID")));
        ui->tableView_SSID->setModel(ssids_model);
        ui->tableView_SSID->setColumnWidth(0,220);
        ui->tableView_SSID->verticalHeader()->setDefaultSectionSize(50);
        ui->tableView_SSID->verticalScrollBar()->setStyleSheet("QScrollBar:vertical{width: 40px;}");
        ui->tableView_SSID->verticalHeader()->hide();
        ui->tableView_SSID->setEditTriggers(QAbstractItemView::NoEditTriggers);
        for(int i=0;i<ssids_list.size();i++){
            ssids_model->setItem(i,0,new QStandardItem(ssids_list.at(i)));
        }
    }
}
void network::getGprsStatus()
{
    /*//获取GPRS状态--初始化函数
    QJsonObject jsonsend;
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = GPRS_STATUS_GET_CMD;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_network_network->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+GPRS_STATUS_GET_CMD){
        //数据获取失败，可以尝试重连
        //QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
        //message.exec();
        ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
        QTimer::singleShot(2000,ui->label_tips,SLOT(hide()));
        ui->pushButton_gprs_open->show();
        ui->pushButton_gprs_close->hide();
    }else{
        //QString strShow = "";
        //strShow.append(retJson);
        //QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        //message.exec();
        //解析json字符串
        QString status=jsonrecv.value("status").toString();
        if(status=="1"){
            //GPRS开启
            gprsFlag=1;
            ui->pushButton_gprs_open->show();
            ui->pushButton_gprs_close->hide();
        }else{
            //GPRS关闭
            gprsFlag=0;
            ui->pushButton_gprs_open->hide();
            ui->pushButton_gprs_close->show();
        }
    }*/
    fn_StartNetInfoThread(GETGPRSSETTING);
    if(qs_ipaddress3==""){
        loadShow();
    }
    timerGprs->start(2000);
}
void network::getGprsInfo()
{
    //获取GPRS参数
    loadHide();
    timerGprs->stop();
    int nettype=CGlobal::g_gprs_setting.nettype;
    int status=CGlobal::g_gprs_setting.status;
    std::string ipaddress=std::string(CGlobal::g_gprs_setting.ipaddress);

    qs_ipaddress3=QString::fromStdString(ipaddress);
    if(status==1){
        //GPRS开启
        gprsFlag=1;
        ui->pushButton_gprs_open->show();
        ui->pushButton_gprs_close->hide();
    }else{
        //GPRS关闭
        gprsFlag=0;
        ui->pushButton_gprs_open->hide();
        ui->pushButton_gprs_close->show();
    }
    myPtf("====4G Setting\n");
    myPtf("nettype:%d\n",CGlobal::g_gprs_setting.nettype);
    myPtf("netstatus:%d\n",CGlobal::g_gprs_setting.status);
    myPtf("ip address:%s\n",CGlobal::g_gprs_setting.ipaddress);
}
void network::setNetworkInfomation()
{
    //设置有线网络参数
    //组织数据
    QJsonObject jsonsend;
    QString ipaddress = ui->lineEdit_ipaddress->text();
    QString netmask = ui->lineEdit_netmask->text();
    QString gateway = ui->lineEdit_gateway->text();
    QString dns1 = ui->lineEdit_DNS1->text();
    QString dns2 = ui->lineEdit_DNS2->text();
    jsonsend.insert("ipaddress",ipaddress);
    jsonsend.insert("netmask",netmask);
    jsonsend.insert("gateway",gateway);
    jsonsend.insert("dns1",dns1);
    jsonsend.insert("dns2",dns2);
    jsonsend.insert("flag",networkFlag);
    jsonsend.insert("usedhcp",dhcpFlag);

    //IP地址、掩码、网关、DNS正则
    QRegExp ipRegExp = QRegExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
    QRegExpValidator * ipRegExpValidator = new QRegExpValidator(ipRegExp,this);
    int pos=0;
    QValidator::State result_ipaddress = ipRegExpValidator->validate(ipaddress,pos);
    QValidator::State result_netmask = ipRegExpValidator->validate(netmask,pos);
    QValidator::State result_gateway = ipRegExpValidator->validate(gateway,pos);
    QValidator::State result_dns1 = ipRegExpValidator->validate(dns1,pos);
    QValidator::State result_dns2 = ipRegExpValidator->validate(dns2,pos);

    if(ipaddress == ""){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入IP地址!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入IP地址");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(result_ipaddress != QValidator::Acceptable){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入正确格式的IP地址!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入正确格式的IP地址");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(netmask == ""){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入掩码!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入掩码");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(result_netmask != QValidator::Acceptable){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入正确格式的掩码!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入正确格式的掩码");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(gateway == ""){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入网关!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入网关");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(result_gateway != QValidator::Acceptable){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入正确格式的网关!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入正确格式的网关");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(dns1 == ""){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入DNS1!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入DNS1");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(result_dns1 != QValidator::Acceptable){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入正确格式的DNS1!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入正确格式的DNS1");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(dns2 == ""){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入DNS2!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入DNS2");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(result_dns2 != QValidator::Acceptable){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入正确格式的DNS2!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入正确格式的dns2");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else{
        //发送数据
        QJsonObject jsonrecv;
        int Cmd = NETWORK_INFOMATION_SET_CMD;
        char retJson[1024];
        memset(retJson,0,1024);
        jsonrecv = scmd_network_network->send_Command(&Cmd,jsonsend,retJson);
        if(Cmd != 10000+NETWORK_INFOMATION_SET_CMD){
            //数据发送失败，可以尝试重连
            /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
            message.exec();*/
            ui->label_tips->show();
            ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
            QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
        }else{
            QString strShow = "";
            strShow.append(retJson);
            /*QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
            message.exec();*/

            //解析json字符串
            if(strShow.toInt()==3001){
                ui->label_tips->show();
                ui->label_tips->setText("设置成功!");
                QTimer::singleShot(1000,ui->label_tips,SLOT(hide()));
            }else{
                ui->label_tips->show();
                ui->label_tips->setText("设置失败!");
                QTimer::singleShot(1000,ui->label_tips,SLOT(hide()));
            }
        }
    }
}
void network::setWifiSetting()
{
    //设置无线网络参数
    //组织数据
    QJsonObject jsonsend;
    QString password = ui->lineEdit_password->text();
    QString ssid =ui->lineEdit_SSID->text();
    jsonsend.insert("wifiswitch",wifiFlag);
    jsonsend.insert("ssid",ssid);
    jsonsend.insert("password",password);

    if(ssid==""){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请选择SSID!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请选择SSID");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(password==""){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入密码!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入密码");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else{
        //发送数据
        QJsonObject jsonrecv;
        int Cmd = WIFI_SETTING_SET_CMD;
        char retJson[1024];
        memset(retJson,0,1024);
        jsonrecv = scmd_network_network->send_Command(&Cmd,jsonsend,retJson);
        if(Cmd != 10000+WIFI_SETTING_SET_CMD){
            //数据发送失败，可以尝试重连
            /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
            message.exec();*/
            ui->label_tips->show();
            ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
            QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
        }else{
            QString strShow = "";
            strShow.append(retJson);
            /*QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
            message.exec();*/
            //解析json字符串
            if(strShow.toInt()==WIFI_SETTING_SET_CMD){
                ui->label_tips->show();
                ui->label_tips->setText("设置成功!");
                QTimer::singleShot(1000,ui->label_tips,SLOT(hide()));
            }else{
                ui->label_tips->show();
                ui->label_tips->setText("设置失败!");
                QTimer::singleShot(1000,ui->label_tips,SLOT(hide()));
            }
        }
    }
}
void network::setGprsSwitch()
{
    //设置GPRS参数
    //组织数据
    QJsonObject jsonsend;
    jsonsend.insert("flag",gprsFlag);
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = GPRS_SWITCH_SET_CMD;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_network_network->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+GPRS_SWITCH_SET_CMD){
        //数据发送失败，可以尝试重连
        /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
        QTimer::singleShot(5000,ui->label_tips,SLOT(hide()));
    }else{
        QString strShow = "";
        strShow.append(retJson);
        /*QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        message.exec();*/
        if(strShow.toInt()==GPRS_SWITCH_SET_CMD){
            ui->label_tips->show();
            ui->label_tips->setText("设置成功!");
            QTimer::singleShot(1000,ui->label_tips,SLOT(hide()));
        }else{
            ui->label_tips->show();
            ui->label_tips->setText("设置失败!");
            QTimer::singleShot(1000,ui->label_tips,SLOT(hide()));
        }
    }
}
void network::setServerSetting()
{
    //设置服务器参数
    //组织数据
    QJsonObject jsonsend;
    QString serverIp = ui->lineEdit_ip->text();
    QString port = ui->lineEdit_port->text();
    QString serverIps = ui->lineEdit_spareIp->text();
    QString ports = ui->lineEdit_sparePort->text();
    jsonsend.insert("ipaddress",serverIp);
    jsonsend.insert("port",port);
    jsonsend.insert("ipaddress2",serverIps);
    jsonsend.insert("port2",ports);

    //IP地址、掩码、网关、DNS正则
    QRegExp ipRegExp = QRegExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
    QRegExpValidator * ipRegExpValidator = new QRegExpValidator(ipRegExp,this);
    //端口(正整数)
    QRegExp portRegExp = QRegExp("[0-9]+$");
    QRegExpValidator * portValidator = new QRegExpValidator(portRegExp,this);

    int pos=0;
    QValidator::State result_serverIp = ipRegExpValidator->validate(serverIp,pos);
    QValidator::State result_serverIps = ipRegExpValidator->validate(serverIps,pos);
    QValidator::State result_port = portValidator->validate(port,pos);
    QValidator::State result_ports = portValidator->validate(ports,pos);
    if(serverIp == ""){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入IP地址!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入IP地址");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(result_serverIp != QValidator::Acceptable){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入正确格式的IP地址!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入正确格式的IP地址");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(port == ""){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入端口!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入端口");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(result_port != QValidator::Acceptable){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入正确格式的端口!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入正确格式的端口");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(serverIps == ""){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入备用IP地址!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入备用IP地址");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(result_serverIps != QValidator::Acceptable){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入正确格式的备用IP地址!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入正确格式的备用IP地址");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(ports == ""){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入备用端口!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入备用端口");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(result_ports != QValidator::Acceptable){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入正确格式的备用端口!");
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("请输入正确格式的备用端口");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else{
        //发送数据
        QJsonObject jsonrecv;
        int Cmd = SERVER_SETTING_SET_CMD;
        char retJson[1024];
        memset(retJson,0,1024);
        jsonrecv = scmd_network_network->send_Command(&Cmd,jsonsend,retJson);
        if(Cmd != 10000+SERVER_SETTING_SET_CMD){
            //数据发送失败，可以尝试重连
            /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
            message.exec();*/
            ui->label_tips->show();
            ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
            QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
        }else{
            QString strShow = "";
            strShow.append(retJson);
            /*QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
            message.exec();*/
            //解析json字符串
            if(strShow.toInt()==1025){
                ui->label_tips->show();
                ui->label_tips->setText("设置成功!");
                QTimer::singleShot(1000,ui->label_tips,SLOT(hide()));
            }else{
                ui->label_tips->show();
                ui->label_tips->setText("设置失败!");
                QTimer::singleShot(1000,ui->label_tips,SLOT(hide()));
            }
        }
    }
    delete ipRegExpValidator;
    delete portValidator;
}
void network::getQcode()
{
    //获取终端唯一识别码二维码
    QString  deviceno(termCode_network);
    QString program  = "/usr/local/nkty/CreateQR" ;
    QStringList arguments ;
    arguments << deviceno;
    process->execute(program,arguments);

    QPixmap image; //定义一张图片
    image.load("device.png");//加载
    ui->label_picture->clear();//清空
    ui->label_picture->setPixmap(image);//加载到Label标签
    ui->label_picture->show();//显示
}
void network::on_pushButton_back_clicked()
{
    //返回主页面
    this->close();
}

void network::on_pushButton_submit1_clicked()
{
    setNetworkInfomation();
}

void network::on_pushButton_submit2_clicked()
{
    setWifiSetting();
}

void network::on_pushButton_submit3_clicked()
{
    //setGprsSwitch();
}

void network::on_pushButton_submit4_clicked()
{
    setServerSetting();
}

void network::on_pushButton_wifi_open_clicked()
{
    //无线关闭
    wifiFlag=0;
    ui->pushButton_wifi_open->hide();
    ui->pushButton_wifi_close->show();
    ui->tableView_SSID->hide();
    ui->label_wifiIp->setText("");
    ui->label_wifiMac->setText("");
}

void network::on_pushButton_wifi_close_clicked()
{
    /*//无线开启
    wifiFlag=1;
    ui->pushButton_wifi_open->show();
    ui->pushButton_wifi_close->hide();*/
}

void network::on_pushButton_network_close_clicked()
{
    //有线连接--开
    networkFlag=1;
    ui->pushButton_network_open->show();
    ui->pushButton_network_close->hide();
}

void network::on_pushButton_network_open_clicked()
{
    //有线连接--关
    networkFlag=0;
    ui->pushButton_network_open->hide();
    ui->pushButton_network_close->show();
}

void network::on_pushButton_DHCP_off_clicked()
{
    //DHCP开启
    dhcpFlag=1;
    ui->pushButton_DHCP_on->show();
    ui->pushButton_DHCP_off->hide();
    ui->lineEdit_ipaddress->setEnabled(false);
    ui->lineEdit_netmask->setEnabled(false);
    ui->lineEdit_gateway->setEnabled(false);
    ui->lineEdit_DNS1->setEnabled(false);
    ui->lineEdit_DNS2->setEnabled(false);
}

void network::on_pushButton_DHCP_on_clicked()
{
    //DHCP关闭
    dhcpFlag=0;
    ui->pushButton_DHCP_on->hide();
    ui->pushButton_DHCP_off->show();
    ui->lineEdit_ipaddress->setEnabled(true);
    ui->lineEdit_netmask->setEnabled(true);
    ui->lineEdit_gateway->setEnabled(true);
    ui->lineEdit_DNS1->setEnabled(true);
    ui->lineEdit_DNS2->setEnabled(true);
}

void network::on_tableView_SSID_clicked(const QModelIndex &index)
{
    //选择wifi
    ui->lineEdit_SSID->setText(index.data().toString());
    ui->lineEdit_password->setText("");
    ui->tableView_SSID->hide();
}

void network::on_pushButton_search_wifi_clicked()
{
    if(ssids==""){
        timerwifi->start(5000);
    }else{
        ui->tableView_SSID->show();
    }
}

void network::on_pushButton_gprs_open_clicked()
{
    /*//GPRS开
    gprsFlag=0;
    ui->pushButton_gprs_close->show();
    ui->pushButton_gprs_open->hide();*/
}

void network::on_pushButton_gprs_close_clicked()
{
    /*//GPRS关
    gprsFlag=1;
    ui->pushButton_gprs_close->hide();
    ui->pushButton_gprs_open->show();*/
}

void network::on_pushButton_exit_clicked()
{
    system("/usr/local/nkty/killprocess.sh");
}

void network::on_pushButton_recovery_clicked()
{
    maskFlag=2;
    ui->groupBox_mask->show();
    ui->label_erroNote->setText("提示恢复默认设置后，系统将恢复成IP：192.168.1.4，\n服务器连接恢复成10.1.70.13，9093，重启系统");
}

void network::on_pushButton_submit_clicked()
{
    if(maskFlag==1){
        reqScanqrcode();
    }else if(maskFlag==2){
        system("/usr/local/nkty/resetip.sh");
        ui->label_erroNote->setText("");
    }
    maskFlag=0;
    ui->groupBox_mask->hide();
}

void network::on_pushButton_cancel_clicked()
{
    ui->groupBox_mask->hide();
    ui->label_erroNote->setText("");
}


void network::showPic()
{
    //只显示背景图
    QJsonObject jsonsend;
    jsonsend.insert("BGPicture","/usr/local/nkty/images/bg.jpg");

    //发送数据
    QJsonObject jsonrecv;
    int Cmd = SECSCREEN_SHOWPIC_CMD;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_sec_network->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+SECSCREEN_SHOWPIC_CMD){
        //数据获取失败，可以尝试重连
        /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
        message.exec();*/
        myPtf("send_cmd SECSCREEN_SHOWPIC_CMD error\n");
        ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
        QTimer::singleShot(5000,ui->label_tips,SLOT(hide()));
    }else{
    }
}
void network::showAll(QString strtemp,int PosX,int PosY,int FontSize)
{
    //显示全部
    QJsonObject jsonsend;
    QByteArray utf8String = strtemp.toUtf8();
    QLatin1String lstr=QLatin1String(utf8String);
    jsonsend.insert("Text",lstr);
    jsonsend.insert("PosX",PosX);
    jsonsend.insert("PosY",PosY);
    jsonsend.insert("FontSize",FontSize);

    jsonsend.insert("BGPicture","/usr/local/nkty/images/bg.jpg");
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = SECSCREEN_SHOWALL_CMD;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_sec_network->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+SECSCREEN_SHOWALL_CMD){
        //数据获取失败，可以尝试重连
        /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
        message.exec();*/
        myPtf("send_cmd SECSCREEN_SHOWALL_CMD error\n");
        ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
        QTimer::singleShot(5000,ui->label_tips,SLOT(hide()));
    }
}
void network::appendtxt(QString strtemp,int PosX,int PosY,int FontSize)
{
    //追加文字
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
    jsonrecv = scmd_sec_network->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+SECSCREEN_APPENDTXT_CMD){
        //数据获取失败，可以尝试重连
        /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
        message.exec();*/
        myPtf("send_cmd SECSCREEN_APPENDTXT_CMD error\n");
        ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
        QTimer::singleShot(5000,ui->label_tips,SLOT(hide()));
    }else{
    }
}
void network::player(QString url)
{
    /*QMessageBox message(QMessageBox::NoIcon,"提示","url:"+url);
    message.exec();*/
    players_network->setMedia(QMediaContent(QUrl::fromLocalFile(url)));
    players_network->play();
    myPtf("start play!\n");
}

void network::statePannel(QString noteTip)
{
    //查询消费结果，根据相应状态,显示不同提示界面
    //主屏提示
    ui->label_tips->show();
    ui->label_tips->setText(noteTip);
    QTimer::singleShot(1000,ui->label_tips,SLOT(hide()));
    ui->label_tips->setText("");
    ui->groupBox_cashierBg->hide();//取消扫描遮罩隐藏
    //副屏提示
    showAll(noteTip,80,200,40);
}
void network::reqScanqrcode()
{
    //请求扫码查询设置
    //组织数据
    QJsonObject jsonsend;
    QDateTime time = QDateTime::currentDateTime();
    timeTcur_scanqrcode = time.toTime_t();//当前时间戳
    jsonsend.insert("timeStamp",QString::number(timeTcur_scanqrcode));
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = UI_REQ_SCANQRCODE;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_main_network->send_Command(&Cmd,jsonsend,retJson);

    if(Cmd != 10000+UI_REQ_SCANQRCODE){
        //数据发送失败，可以尝试重连
        ui->label_tips->show();
        //ui->label_tips->setText("发送请求消费命令失败!,ret="+QString::number(Cmd));
        ui->label_tips->setText("扫描二维码服务断开，请重新连接服务！");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
        myPtf("recv UI_REQ_SCANQRCODE ret error!\n");
    }else{
        myPtf("recv UI_REQ_SCANQRCODE ret ok!\n");
        //ui->label_tips->show();
        /*QString strShow = "";
        strShow.append(retJson);
        QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        message.exec();*/
        //解析json字符串
        int state =jsonrecv.value("state").toInt();
        myPtf("state----------------------------:%d\n"+state);
        if(state==1){
            dateStamp_scanqrcode= jsonrecv.value("timeStamp").toString();//返回时间戳

            if(dateStamp_scanqrcode.toInt()!= timeTcur_scanqrcode)
            {
                ui->label_tips->setText("返回时间戳错误!");
                QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
                myPtf("dateStamp error! timeTcur_scanqrcode=%d\n",timeTcur_scanqrcode);
            }
            else
            {
                showScanqrcodeReady();
                timerScanqrcode->start(200);//启动查询消费
                myPtf("start to queScanqrcode\n");
            }
        }else{
            ui->label_tips->setText("接收失败");
            QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
        }
    }
}
void network::showScanqrcodeReady()
{
    ui->label_info->setText("请扫描二维码，获取设置信息");
    ui->label_tips->setText("");
    ui->groupBox_cashierBg->show();
    ui->pushButton_cancels->show();
    showAll("请扫码!",130,150,40);
    player("/usr/local/nkty/players/video14.wav");//请扫码
}
void network::showScanqrcodeResult(int result)
{
    //得到查询结果后显示
    QDateTime time = QDateTime::currentDateTime();
    myPtf("start showScanqrcodeResult! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
    switch (result) {
    case CONSUME_SUCCESS:
        myPtf("start showScanqrcodeResult 1! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());

        timeTcur_scanqrcode=0;//取消成功后，将当前时间戳归0
        //得到返回结果--调用函数给农哥
        ui->groupBox_cashierBg->hide();
        ui->label_info->setText("");
        setSaveall();

        time = QDateTime::currentDateTime();
        myPtf("start showScanqrcodeResult 2! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
        break;
    default:
        //查询失败
        statePannel(msg);//查询消费结果，根据相应状态,显示不同提示界面
        break;
    }
    QTimer::singleShot(3000,this,SLOT(showPic()));
    myPtf("finish showScanqrcodeResult!\n");
}
void network::queScanqrcodeRet()
{
    //查询扫描二维码结果
    myPtf("stop queScanqrcodeRet timer!\n");
    timerScanqrcode->stop();
    myPtf("start queScanqrcodeRet function!\n");
    //组织数据
    QJsonObject jsonsend;
    jsonsend.insert("timeStamp",dateStamp_scanqrcode);
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = UI_QUE_CONSUMERET;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_main_network->send_Command(&Cmd,jsonsend,retJson);
    //如果未返回想要的结果，需要继续查询，则继续启动定时器,继续查询结果
    //可能上级服务死掉，需重试（暂未考虑重试次数）
    if(Cmd != 10000+UI_QUE_CONSUMERET){
        myPtf("error,but have to continue to queScanqrcodeRet\n");
        timerScanqrcode->start(1000);
        return;
    }
    //已收到返回，查看当前进度状态
    int state=jsonrecv.value("state").toInt();
    int ret;
    //如果已经处理完毕返回结果，应解析最终结果
    switch (state) {
    case 1:
        ret = jsonrecv.value("ret").toInt();
        msg=jsonrecv.value("msg").toString();
        if(ret == CONSUME_SUCCESS)
        {
            //获取json串
            qrstr=jsonrecv.value("qrstr").toString();
        }
        myPtf("showScanqrcodeResult ret=%d\n",ret);
        showScanqrcodeResult(ret);
        //如果是解码失败、云服务失败等原因，应在报错等待后重新发起下一轮消费请求
        if(ret<0)
        {
            reqScanqrcode();
        }
        break;
    //各种错误，一般不可能发生，可在显示错误后结束本次操作
    case 2:
    case 3:
        //显示失败信息，结束
        myPtf("showScanqrcodeResult state=%d\n",state);
        showScanqrcodeResult(state);
        break;
    case 4:
    case 5:
        //不显示消息，重新发起请求
        myPtf("continue to queScanqrcodeRet\n");
        timerScanqrcode->start(200);
        break;
    default:
        break;
    }
}
void network::cancelScanqrcode()
{
    //取消扫码查询设置
    QJsonObject jsonsend;
    jsonsend.insert("timeStamp",dateStamp_scanqrcode);
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = UI_CANCEL_CONSUME;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_main_network->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+UI_CANCEL_CONSUME){
        //数据获取失败，可以尝试重连
        /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else{
        /*QString strShow = "";
        strShow.append(retJson);
        QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        message.exec();*/
        //解析json字符串
        ui->pushButton_cancels->hide();
        if(jsonrecv.value("state").toInt()==1){
            ui->label_info->setText("取消成功!");
            showAll("本次扫码已取消",150,200,40);
            QTimer::singleShot(2000,this,SLOT(showPic()));
            myPtf("stop queConsumeRet timer(outer)!\n");
            timerScanqrcode->stop();
            timeTcur_scanqrcode=0;//取消成功后，将当前时间戳归0
        }else{
            ui->label_info->setText("取消失败!");
        }
        QTimer::singleShot(2000,ui->groupBox_cashierBg,SLOT(hide()));        
    }
}

void network::on_pushButton_scavenging_clicked()
{
    maskFlag=1;
    ui->groupBox_mask->show();
    ui->label_erroNote->setText("设置完成后会重启系统，您确定要扫码设置？");
}

void network::on_pushButton_cancel4_clicked()
{
    //获取服务器参数(恢复)
    getServerSetting();
}

void network::on_pushButton_cancel1_clicked()
{
    //获取有线网络设置(恢复)
    getNetworkInfomation();
}

void network::on_pushButton_cancels_clicked()
{
    //取消扫码查询设置
    cancelScanqrcode();
}
void network::setSaveall()
{
    //保存全部设置参数
    QJsonObject jsonsend;
    jsonsend.insert("qrstr",qrstr);
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = SETTING_SAVEALL_SET;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_network_network->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+SETTING_SAVEALL_SET){
        //数据获取失败，可以尝试重连
        /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else{
        QString strShow = "";
        strShow.append(retJson);
        /*QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        message.exec();*/
        myPtf("strShow:%d\n",strShow.toInt());
        //解析json字符串

        if(strShow.toInt()==1){
            ui->label_tips->show();
            ui->label_tips->setText("有线网络设置成功!");
            //获取有线网络参数--初始化函数
            getNetworkInfomation();
        }else if(strShow.toInt()==2){
            ui->label_tips->show();
            ui->label_tips->setText("无线网络设置成功!");
            //获取无线网络参数--初始化函数
            getWifiSetting();
        }else if(strShow.toInt()==3){
            ui->label_tips->show();
            ui->label_tips->setText("服务器参数设置成功!");
            //获取服务器参数--初始化函数
            getServerSetting();
        }else if(strShow.toInt()==4){
            ui->label_tips->show();
            ui->label_tips->setText("GPRS参数设置成功!");
            //获取服务器参数--初始化函数
            getGprsStatus();
        }else{
            ui->label_tips->show();
            ui->label_tips->setText("设置失败!");            
        }
        QTimer::singleShot(2000,ui->label_tips,SLOT(hide()));
        ui->groupBox_mask->hide();
    }
}
