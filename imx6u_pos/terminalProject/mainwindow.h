#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "sendcmd.h"
#include "flow.h"
#include <QTimer>
#include <QDateTime>
#include <QMediaPlayer>
#include <QKeyEvent>
#include "common.h"
#include "login.h"
#include <QJsonArray>
#include "consumecallbackthread.h"
#include <QStandardItemModel>
static char *ATTRIB1_FILE = "/usr/local/nkty/attrib1.dat";
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void timerUpdate();//日期

    void queConsumeRet(QJsonObject jsonrecv);//查询消费结果--工作模式：消费

    void hideConsumeMsg();//隐藏消费面板--恢复初始页面

    void queAccoInfoRet(QJsonObject jsonrecv);//查询卡账户信息结果--工作模式：出纳

    void waitForInitDate();//欢迎光临预启动界面

    void showPic();

    void showAll(QString strtemp,int PosX,int PosY,int FontSize);

    void appendtxt(QString strtemp,int PosX,int PosY,int FontSize);

    void on_pushButton_menu_clicked();

    void on_pushButton_gotoFlowlist_clicked();

    void on_pushButton_termMenu_clicked();

    void on_pushButton_submits_clicked();

    void on_pushButton_cancelConsume_clicked();

    void on_pushButton_1_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_point_clicked();

    void on_pushButton_0_clicked();

    void on_pushButton_back_clicked();

    void on_pushButton_delete_clicked();

    void on_pushButton_clear_clicked();

    void on_pushButton_keyboard_clicked();

    void on_pushButton_examination_clicked();

    void on_pushButton_cashier_clicked();

    void on_pushButton_cancel_clicked();

    void returnConsume();//恢复请求消费初始状态

    void returnAccoInfo();//恢复查询机初始状态

    void showScanqrser();//交易成功后，副屏显示信息

    void recoveryAccoInfo();//恢复自检初始状态

    void on_pushButton_login_clicked();

    void on_pushButton_add_clicked();

    void on_pushButton_backs_clicked();

    void reqConsume(int reqNum);//请求消费

    void on_pushButton_consume_clicked();

    void on_pushButton_consume_cancel_clicked();

    void consumePlayer();

    void clearPannel();

    void getOffLineFlowNum();//获取尚未上传的脱机流水条数

    void on_pushButton_termEnter_clicked();

    //void on_pushButton_upload_clicked();

    void on_pushButton_enter_clicked();

    void on_pushButton_confirm_cancel_clicked();

    void on_pushButton_confirm_upload_clicked();

    void clearUploadPanel();//清空上传面板



private:
    Ui::MainWindow *ui;

    void player(QString url);//音频
    void versions();//获取运行版本号--所有服务
    void consumeserVersions();//运行版本号--consumeser服务
    void scanqrserVersions();//运行版本号--scanqrser服务
    void secscreenVersions();//运行版本号--secscreen服务
    void networkVersions();//运行版本号--network服务
    //int processStatus();//更新各个进程状态
    void getNetworkConnectStatus();//获取网络连接状态
    void getHardwareSerialno();//获取硬件唯一号--初始化函数
    int getTermInfo();//获取初始化界面信息 成功返回1，失败返回0
    void getTotal();//获取本餐次消费金额、本餐次消费总笔数
    int inputReg();//验证输入框金额(或票数)
    void keyPressEvent(QKeyEvent *ev);//监听键盘事件

    void showConsumeResult(int result);//得到查询的消费结果后显示
    void showConsumeReady();//请求消费信息前显示
    void showConsumeMsg(QString noteMes);//交易成功后,显示消费信息界面
    void statePannel(QString noteTip);//查询消费结果，根据相应状态,显示不同提示界面
    void errorPannel(int ret,QString noteMes,QString videoUrl);//消费中错误信息提示界面
    void passwordPannel();//消费时，如果需要输入密码，弹出该对话框的函数
    void cancelConsume();//取消消费

    void reqAccoInfo();//请求刷卡自检
    void showAccoInfoResult(int result);//得到查询的卡账户结果后显示
    void showAccoInfoReady();//请求卡自检前显示
    void showAccoInfoMsg();//显示卡账户信息
    void hideAccoInfoMsg();//隐藏卡账户信息
    void cancelAccoInfo();//取消卡自检

    void getSegs();//获取当前餐次信息
    void serverError();//调用云服务时错误,界面显示情况

    void clearScreen(int waitTime,int screenID);//清理主-副屏的函数

    //void getWifiSetting();//获取无线网络参数--初始化函数

    void offLineConsume();//脱机消费

    void uploadFlow(int num);//上传脱机流水

    void showOfflineConsumeMsg(QString noteMes);//脱机消费成功候，显示交易成功

    void getFlowNum();//获取脱机流水

    int getAttrib1();//获取工作模式
    int setAttrib1(int attr);//写入工作模式，暂存本地，然后再读取

    int attrib1New;


    QString termCode;//获得硬件唯一号
    int termID;//终端ID
    int termNo;//终端号
    int areaID;//分区号
    int attrib1;//终端工作模式
    int consumeType;//交易类型
    int keyFlag;//按键标志,0:在主界面可点击,1:在主界面不可点击
    int keyFlag_cashier;//按键标志(出纳--自检),0:在主界面可点击,1:在主界面不可点击,备注：只有在出纳的时候才可使用

    sendCmd * scmd_network;//网络启动
    sendCmd * scmd_main;//主屏启动
    sendCmd * scmd_sec;//副屏启动

    QTimer * timerMain;//定时获取终端信息、时间、版本号
    //QTimer * timerSearch;//查询消费结果
    //QTimer * timerCheckCard;//卡自检
    QTimer * timerclearScreen;//清屏定时器
    //int timeMainFlag;//主定时器标志
    QTimer * timerFlowNum;//获取尚未上传的脱机流水条数定时器


    QString uiVersion;//UI版本号
    QString consumeserVersion;//获取消费服务版本号，consumeser服务
    QString scanqrserVersion;//获取版本信息,scanqrser服务
    QString secscreenVersion;//获取版本号,secscreen服务
    QString networkVersion;//获取版本号,network服务

    //更新各个进程状态
    QString consumeserStatus;//consumeser服务的获取时间
    QString terminalProjectStatus;//terminalProject服务的获取时间
    QString secscreenStatus;//secscreen服务的获取时间
    QString scanqrserStatus;//scanqrser服务的获取时间

    int rets;//服务连接状态
    int workType;//终端工作模式
    int transactionType;//交易类型
    QMediaPlayer * players;
    //common * myCommon;//新建获取网络状态的类


    QString inputTxt;//输入的金额或者张数
    int point;//0：无小数点，1：有小数点，2：小数点后1位，3：小数点后2位,不能再输入。

    QString cashier;//出纳参数，1:存款,2:取款
    int cashierFlag;//出纳标志，0：取款，1：存款
    int keyboardFlag;//键盘标志
    //消费模式
    int ret;//账户状态10：交易成功(正常账户)，30：账户余额不足，40：账户不存在或已注销，60：冻结或挂失
    QString totalstr;//累加公式
    QString dateStamp;//返回时间戳(消费)
    char moneySum[20];
    double moneyFloat;//消费金额--输入金额
    int money;//消费金额--参数
    int timeTcur;//当前时间戳(消费)
    int addFlag;//加号标志,0:没点击过加号,1:点击过加号
    double totalMoney;//总金额--工作模式为消费
    char remainMoney2[20];//个人账户余额--工作模式为消费
    char flowMoney2[20];//消费金额
    char secAccoMoney[20];//补贴账户余额
    int consumeTimes;//第几次消费
    char manageFee[20];//附加费
    int accountID;//卡账户
    QString msg;//错误信息
    int keyOtherFlag;//在消费模式下，读卡或扫码时，只有取消按键可用，其他按键不可用
    QString pwd;//消费模式时，需要输入密码
    int pwdFlag;//消费模式时，需要输入密码的标志,0:正常,1:再次请求消费的时候，就不用提示请刷卡或扫码了
    //洗衣模式
    int remainMoney6;//个人账户余额--工作模式为洗衣
    int flowMoney6;//消费金额--工作模式为洗衣
    int totalCount;//总金额--工作模式为洗衣
    //出纳模式
    QString termPassWord;//出纳时的登录密码
    int loginFlag;//出纳时的登录标志
    QString dateStamp_check;//返回时间戳(出纳)
    int timeTcur_check;//当前时间戳(出纳)
    double totalInCount;//当日充值总金额
    double totalOutCount;//当日取钱总金额
    int cardId;//卡号
    int realUseFlag;//账户状态
    QString accoName;//账户名称
    QString deptName;//部门名称
    QString pidName;//身份名称
    char remainMoney1[20];//个人账户余额--工作模式为出纳
    int intRemainMoney;//个人账户余额--工作模式为出纳--用于和输入框的值进行比较
    char secAccoMoney1[20];//补贴账户余额--工作模式为出纳
    int accoInfoFlag;//获取卡账户信息标志
    int keyMenuFlag;//当处于出纳时，还没获取到个人信息时，除了菜单按键可点，其他按键不可点
    int consumeSuccessFlag;//交易成功后的标志
    //定额消费
    int timeSegId;//时段ID
    QJsonArray arraySegs;//定义一个存放餐次信息的数组
    QString timeSegStart;//时段开始时间(用于当前属于哪个时段)
    QString timeSegEnd;//时段结束时间
    QString timeSegName;//时段名称
    QString consumeMoney;//时段金额
    QString timeMorning;//早餐时间
    int consumeState;//启动固定消费标志,1:启动,0:不启动
    int newConsumeState;
    int oldConsumeState;

    QString timeEnd_str;//本餐次的结束时间

    //累加金额消费
    double totalnum[100];//累加金额数量
    int arrnum;//数组下标

    int fScreenClearTime;//清副屏的时间戳
    int sScreenClearTime;//清主屏的时间戳
    int clearFlag;//清屏标志，0：正常，1：在交易信息还没消失前，当clearFlag=1时，要进行清屏操作

    SignalSender * sig;
    char strMsg[MSGBUFFLEN];
    int cmdType;//命令类型，现在包括
                //3：UI_REQ_CONSUME 请求消费
                //10:UI_REQ_ACCOUNTINFO 获取账户信息
                //11:UI_REQ_SCANQRCODE 申请扫描二维码
                //13:UI_REQ_OFFLINECONSUME 脱机消费


    //脱机操作功能
    int flowCount;//尚未上传的脱机流水条数
    int offlineFlag;//脱机标志，当为1时就不用在弹出框提示了
    int normalFlag;//正常标志，当为1时就不用在弹出框提示了
    int flowFlag;//开机时是否显示上传流水的弹出框，为1时就不显示了
    int iNum;
    int uploadFlag;//点击“上传”按钮标志，1:开启
    int connectFlag;//联网标志，1：连接，0：未连接
    //int getTotalFlag;//上传完脱机流水后，再刷新流水总数标志，0--未开启，1--开启

public slots:
    void showConsumeRet(char * strvalue);
};

#endif // MAINWINDOW_H
