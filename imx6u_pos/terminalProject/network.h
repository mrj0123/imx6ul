#ifndef NETWORK_H
#define NETWORK_H

#include <QDialog>
#include "sendcmd.h"
#include <QTimer>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QKeyEvent>
#include <QEvent>
#include <QObject>
#include <QProcess>
#include <QMediaPlayer>



namespace Ui {
class network;
}

class network : public QDialog
{
    Q_OBJECT

public:
    explicit network(QWidget *parent = 0);
    ~network();
    sendCmd * scmd_network_network;
    sendCmd * scmd_main_network;
    sendCmd * scmd_sec_network;

    QString termCode_network;//终端唯一识别号
    QMediaPlayer * players_network;

    QString uiVersion_network;//UI版本号
    QString consumeserVersion_network;//获取消费服务版本号，consumeser服务
    QString scanqrserVersion_network;//获取版本信息,scanqrser服务
    QString secscreenVersion_network;//获取版本号,secscreen服务
    QString networkVersion_network;//获取版本号,network服务


private slots:

    void initDate();//整体--初始化函数

    void getWifiSsid();//获取SSID--初始化函数

    void on_pushButton_back_clicked();

    void on_pushButton_submit1_clicked();

    void on_pushButton_submit2_clicked();

    void on_pushButton_submit3_clicked();

    void on_pushButton_submit4_clicked();

    bool eventFilter(QObject *watched, QEvent *event);

    void on_pushButton_wifi_open_clicked();

    void on_pushButton_wifi_close_clicked();

    void on_pushButton_network_close_clicked();

    void on_pushButton_network_open_clicked();

    void on_pushButton_DHCP_off_clicked();

    void on_pushButton_DHCP_on_clicked();

    void on_tableView_SSID_clicked(const QModelIndex &index);

    void on_pushButton_search_wifi_clicked();

    void on_pushButton_gprs_open_clicked();

    void on_pushButton_gprs_close_clicked();

    void on_pushButton_exit_clicked();

    void on_pushButton_recovery_clicked();

    void on_pushButton_submit_clicked();

    void on_pushButton_cancel_clicked();

    void showPic();

    void showAll(QString strtemp,int PosX,int PosY,int FontSize);

    void appendtxt(QString strtemp,int PosX,int PosY,int FontSize);

    void queScanqrcodeRet();//查询扫描二维码结果

    void on_pushButton_scavenging_clicked();

    void on_pushButton_cancel4_clicked();

    void on_pushButton_cancel1_clicked();

    void on_pushButton_cancels_clicked();


    void on_tabWidget_tabBarClicked(int index);
    void getServerInfo();
    void getNetworkInfo();
    void getWifiInfo();
    void getGprsInfo();
private:
    Ui::network *ui;
    QTimer * timerwifi;//ssid初始化
    QTimer * timerGetServer;//获取服务器参数定时器
    QTimer * timerGetNetwork;//获取有线网络参数定时器
    QTimer * timerGetWifi;//获取无线网络参数定时器
    QTimer * timerGprs;//获取GPRS参数定时器
    void init();
    void getNetworkInfomation();//获取有线网络参数--初始化函数
    void getWifiSetting();//获取无线网络参数--初始化函数
    void getGprsStatus();//获取GPRS状态--初始化函数
    void getServerSetting();//获取服务器参数--初始化函数

    void setNetworkInfomation();//设置有线网络参数
    void setWifiSetting();//设置无线网络参数
    void setGprsSwitch();//设置GPRS参数
    void setServerSetting();//设置服务器参数
    void getQcode();//获取终端唯一识别码二维码

    void player(QString url);//音频
    void reqScanqrcode();//请求扫码查询设置
    void showScanqrcodeReady();//请求扫码查询设置前显示
    void showScanqrcodeResult(int result);//得到查询的扫码设置结果后显示
    void cancelScanqrcode();//取消扫码查询设置
    void statePannel(QString noteTip);//查询结果，根据相应状态,显示不同提示界面
    void setSaveall();//保存全部设置参数
    void versions();//获取版本号


    void loadShow();
    void loadHide();

    QStringListModel * model;//建立模型
    QStandardItemModel * ssids_model;//建立保存数据的类

    int networkFlag;//有线连接标志，0：关，1：开
    int wifiFlag;//无线连接标志，0：关，1：开
    int dhcpFlag;//DHCP标志，0：关，1：开
    int gprsFlag;//GPRS标志，0：关，1：开
    QString ssids;
    QProcess *process;

    //扫码查询设置
    QString msg;//错误信息
    QTimer * timerScanqrcode;//扫描二维码
    QString dateStamp_scanqrcode;//返回时间戳(扫码查询设置)
    int timeTcur_scanqrcode;//当前时间戳
    QString qrstr;//获取的二维码信息
    int maskFlag;//遮罩标志,0:默认,1:扫码设置,2:恢复设置

    QString qs_ipaddress1;
    QString qs_ipaddress2;
    QString qs_essidname;
    QString qs_ipaddress3;
};

#endif // NETWORK_H
