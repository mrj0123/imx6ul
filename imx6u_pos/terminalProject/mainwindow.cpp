#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qmessagebox.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include "hardware.h"
#include "exqrscan.h"
#include "signalsender.h"
#include <stdio.h>
#include <stdlib.h>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    uiVersion=UIVERSION;//UI程序版本号1
    keyFlag=0;//按键标志
    keyFlag_cashier=1;//按键标志(出纳)
    players =  new QMediaPlayer;
    termCode.clear();
    timeTcur=0;//当前时间戳(消费)
    ret=0;
    timeTcur_check=0;//当前时间戳(出纳)
    point=0;//0：无小数点，1：有小数点，2：小数点后1位，3：小数点后2位,不能再输入。
    cashierFlag=0;//出纳标志，0：取款，1：存款
    cashier="1";//出纳参数，1:存款,2:取款
    termID = 0;//终端ID
    areaID = 0;//分区号
    consumeState = 0;//启动固定消费标志,1:启动,0:不启动
    newConsumeState = 0;
    oldConsumeState = 0;


    accoInfoFlag=0;//获取卡账户信息标志
    keyOtherFlag=0;
    pwdFlag=0;//消费模式时，需要输入密码的标志
    //将累加金额，清空还原
    memset(totalnum,0, sizeof(totalnum));//累加金额数量
    arrnum=0;//数组下标
    addFlag=0;

    //清空主副屏参数
    fScreenClearTime=0;
    sScreenClearTime=0;
    clearFlag=0;

    consumeSuccessFlag=0;

    //timeMainFlag=0;//主定时器标志

    //出纳时的登录
    ui->lineEdit_password->setEchoMode(QLineEdit::Password);//设置输入密码框
    loginFlag=0;//出纳时的登录标志
    keyboardFlag=0;//键盘标志
    //界面标签显示状态
    ui->label_tips->hide();//提示隐藏
    ui->label_note_remainMoney->hide();//个人账户余额--隐藏域
    ui->label_note_secAccoMoney->hide();//补贴账户余额--隐藏域
    ui->label_note_manageFee->hide();//附加费--隐藏域
    ui->label_note_consumeLimitMoney->hide();//单笔消费上限--隐藏域
    ui->label_note_flowMoney->hide();//当前消费金额--隐藏域
    ui->groupBox_mask->hide();//取消交易遮罩
    ui->groupBox_keyboardBg->hide();//软键盘
    ui->groupBox_termInfoBg->hide();//终端唯一识别码错误信息遮罩
    ui->groupBox_consumeMesBg->hide();//左侧消费信息
    ui->groupBox_cashierBg->hide();//出纳提示遮罩
    ui->groupBox_totalCashierBg->hide();//出纳当日充值-取钱信息栏
    //ui->label_sum->hide();//输入总金额
    ui->groupBox_searchBg->hide();//查询机面板
    ui->groupBox_login->hide();//查询机--登录面板

    ui->pushButton_backs->hide();
    ui->label_remainMoneys_title->hide();

    ui->groupBox_pwdBg->hide();//消费时，如果需要输入密码，弹出该对话框
    ui->lineEdit_password_consume->setEchoMode(QLineEdit::Password);//设置输入密码框
    //网络状态、服务器状态
    ui->label_unconnect->hide();
    ui->label_connectName->hide();
    ui->label_connect01->hide();
    ui->label_connect02->hide();
    ui->label_connect->hide();
    ui->label_network->hide();
    ui->label_4G->hide();
    ui->label_link->hide();

    //脱机界面隐藏元素
    ui->groupBox_flowBg->hide();
    ui->pushButton_confirm_cancel->hide();
    ui->pushButton_confirm_upload->hide();

    ui->pushButton_upload->hide();
    flowCount=0;
    ui->pushButton_termMenu->hide();
    ui->pushButton_termEnter->hide();
    offlineFlag=0;
    ui->groupBox_normalBg->hide();
    normalFlag=0;
    flowFlag=0;
    iNum=0;
    uploadFlag=0;
    ui->label_offlineFlowCount_title->hide();
    ui->label_offlineFlowCount->hide();
    ui->label_workType_offline->hide();
    connectFlag=1;


    QTimer::singleShot(100,this,SLOT(waitForInitDate()));
    myPtf("1\n");
    showFullScreen();//全屏
    myPtf("2\n");

    //初始化扫描线程
    fn_StartQRScanThread();

}

MainWindow::~MainWindow()
{
    delete ui;
    delete scmd_network;
    delete scmd_main;
    delete scmd_sec;
    delete timerMain;
    //delete timerSearch;
    //delete timerCheckCard;
    delete timerclearScreen;
    delete timerFlowNum;
    fn_EndQRScanThread();//结束扫描线程
    //delete myCommon;
}
int getCurrentTimeMain()
{
   struct timeval tv;
   gettimeofday(&tv,NULL);
   //myPtf("tv.tv_sec=%ld,tv.tv_usec=%ld\n",tv.tv_sec,tv.tv_usec);
   //return ((long long int)tv.tv_sec) * 1000 + ((long long int)tv.tv_usec) / 1000;
   return (tv.tv_sec%86400)*10000 + tv.tv_usec/100;
}


void MainWindow::waitForInitDate()
{

    //主屏
    player("/usr/local/nkty/players/video1.wav");//欢迎光临
    QTimer::singleShot(4000,ui->groupBox_centerMask,SLOT(hide()));//隐藏欢迎光临界面

    //网络启动
    char sers_path[]=GATE_PATH_S;
    char clis_path[]=GATE_PATH_C;
    scmd_network = new sendCmd(sers_path,clis_path);
    scmd_network->conn_afunix();//启动连接

    //主屏启动
    char ser_path[]=CONSUMEBIZ_PATH_S;
    char cli_path[]=CONSUMEBIZ_PATH_C;
    scmd_main = new sendCmd(ser_path,cli_path);
    scmd_main->conn_afunix();//启动连接

    //副屏启动
    char sersec_path[]=SECSCREEN_PATH_S;
    char clisec_path[]=SECSCREEN_PATH_C;
    scmd_sec = new sendCmd(sersec_path,clisec_path);
    scmd_sec->conn_afunix();//启动连接

    //定时获取网络信息、时间
    timerMain = new QTimer(this);
    //查询消费结果
    //timerSearch = new QTimer(this);
    //卡自检
    //timerCheckCard = new QTimer(this);
    //清屏
    timerclearScreen = new QTimer(this);
    //获取尚未上传的脱机流水条数
    timerFlowNum = new QTimer(this);

    //获取网络连接状态
    //myCommon = new common();

    //信息初始化
    if(termCode.isEmpty()){
        //获取终端唯一识别号
        getHardwareSerialno();
    }
    //获取版本号
    versions();

    //修改副屏
    showAll("欢迎光临",150,200,40);
    QTimer::singleShot(2000,this,SLOT(showPic()));//显示完错误信息就清空

    //定时获取网络信息、时间、信息初始化
    connect(timerMain,SIGNAL(timeout()),this,SLOT(timerUpdate()));
    //查询消费结果
    //connect(timerSearch,SIGNAL(timeout()),this,SLOT(queConsumeRet()));
    //卡自检
    //connect(timerCheckCard,SIGNAL(timeout()),this,SLOT(queAccoInfoRet()));
    //清屏
    connect(timerclearScreen,SIGNAL(timeout()),this,SLOT(clearPannel()));
    //获取尚未上传的脱机流水条数
    connect(timerFlowNum,SIGNAL(timeout()),this,SLOT(getOffLineFlowNum()));

    sig = new SignalSender();
    //回调信号
    connect(sig,SIGNAL(newMsgSignal(char *)),this,SLOT(showConsumeRet(char *)),Qt::UniqueConnection);

    fn_StartCallbackThread(sig);



    timerMain->start(100);//启动获取网络信息、时间、信息初始化的定时器
    getFlowNum();//获取脱机流水
    timerclearScreen->start(100);


}

void MainWindow::showConsumeRet(char * strvalue)
{
    if(strcmp(strMsg,strvalue) == 0)
    {
        myPtf("newMsgSignal get the same message!%s\n",strvalue);
        return;
    }
    memcpy(strMsg,strvalue,MSGBUFFLEN);
    myPtf("signal newMsgSignal is here %s",strvalue);
    QByteArray jsonString(strMsg);
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonString);
    QJsonObject jsonrecv = jsonDocument.object();
    if(jsonrecv.isEmpty())
    {
        myPtf("newMsgSignal message is empty~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~!\n");
        return;
    }
    //调用查询结果
    //已收到返回，查看当前进度状态
    cmdType=jsonrecv.value("cmdType").toInt();
    //myPtf("cmdTypeqq%d\n",cmdType);
    if(cmdType==3 || cmdType==13){
        queConsumeRet(jsonrecv);
    }else if(cmdType==10){
        queAccoInfoRet(jsonrecv);
    }

}
void MainWindow::showPic()
{
    //只显示背景图
    QJsonObject jsonsend;
    jsonsend.insert("BGPicture","/usr/local/nkty/images/bg.jpg");

    //发送数据
    QJsonObject jsonrecv;
    int Cmd = SECSCREEN_SHOWPIC_CMD;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_sec->send_Command(&Cmd,jsonsend,retJson);
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
void MainWindow::showAll(QString strtemp,int PosX,int PosY,int FontSize)
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
    jsonrecv = scmd_sec->send_Command(&Cmd,jsonsend,retJson);
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
void MainWindow::appendtxt(QString strtemp,int PosX,int PosY,int FontSize)
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
    jsonrecv = scmd_sec->send_Command(&Cmd,jsonsend,retJson);
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
void MainWindow::timerUpdate()
{
    timerMain->stop();
    QDateTime time = QDateTime::currentDateTime();
    QString str = time.toString("yyyy-MM-dd hh:mm:ss");
    int timeStamp = time.toTime_t();//当前的时间戳
    ui->label_dateTime->setText(str);

    if(time.time().second()%10==0){
        //每10秒获取版本信息
        //获取consumeser服务版本号
        consumeserVersions();
    }
    /*if(essid==""){
        getWifiSetting();
    }*/
    if(rets !=10){
        if(offlineFlag==0){
            if(attrib1==1 || attrib1==7){
                ui->groupBox_termInfoBg->show();
                ui->label_termNote->setText("当前终端服务未链接，请检查网络连接状态并重启机器,\n点击'+'进入网络设置！");
                //return;
            }else{
                ui->groupBox_termInfoBg->show();
                //无脱机模式 modified by yangtao 2019-05-05
                if(AllowedOfflineConsume == 1)
                    ui->label_termNote->setText("当前终端服务未链接，请检查网络连接状态并重启机器,\n点击'+'进入网络设置，点击'取消'进入脱机模式！");
                else
                    ui->label_termNote->setText("当前终端服务未链接，请检查网络连接状态并重启机器,\n点击'+'进入网络设置！");
            }

            ui->groupBox_totalPannelBg->hide();//脱机模式下，右侧信息栏隐藏
            ui->pushButton_gotoFlowlist->hide();//脱机模式下，右侧'查看流水'按钮隐藏
            ui->label_consumeLimitMoney->hide();//脱机模式下，单笔消费上限隐藏
            ui->label_timeSegName->hide();//脱机模式下，当前餐次隐藏

            ui->groupBox_normalBg->hide();
            ui->groupBox_flowBg->hide();
        }
    }else{
        if(uploadFlag==1){
            timerFlowNum->start(500);//继续查询尚未上传的脱机流水条数
        }
        //myPtf("connectFlag%d\n",connectFlag);
        if(connectFlag==1){

            if(offlineFlag==0){

                if(attrib1 ==2 || attrib1==3 || attrib1==6){
                    ui->groupBox_totalPannelBg->show();//正常模式下，右侧信息栏显示
                    ui->pushButton_gotoFlowlist->show();//正常模式下，右侧'查看流水'按钮显示
                    ui->pushButton_keyboard->show();//正常模式下，右侧'按键'按钮显示
                    ui->label_consumeLimitMoney->show();//正常模式下，单笔消费上限显示
                    ui->label_timeSegName->show();//正常模式下，当前餐次显示
                    ui->groupBox_keyboardBg->hide();
                    myPtf("hide groupBox_keyboardBg \n");
                    offlineFlag=1;
                }

            }


            if(flowCount==0){
            //无流水再查看终端

                if (termID==0){
                    //getNetworkConnectStatus();//初始化网络状态
                    //获取终端信息
                    if(getTermInfo()==0)
                    {
                        ui->groupBox_termInfoBg->show();
                        ui->label_termNote->setText("请确定该终端是否在系统注册或查看网络设置是否正常！");

                        //timerMain->start(500);
                        /*if(timeMainFlag==0){
                            timerMain->start(500);
                        }*/
                        return;
                    }else{
                        offlineFlag=0;
                        //normalFlag=0;
                        //timeMainFlag=1;
                        //获取当前餐次信息
                        myPtf("daquploadFlag%d\n",uploadFlag);
                        if(uploadFlag==0){
                            getSegs();
                            oldConsumeState =consumeState;

                            if(consumeState==1){
                                //reqConsume();
                                //脱机新建
                                if(rets==10){
                                    reqConsume(8);
                                }else{
                                    offLineConsume();
                                }
                            }
                            //获取本餐次消费金额--本餐次刷卡次数
                            getTotal();
                            ui->groupBox_termInfoBg->hide();
                            ui->label_termNote->setText("");
                        }
                    }
                }else{
                    //当跨餐次时，需要将新餐次信息显示
                    if(timeSegEnd!=""){
                        QDateTime timeEnd;

                        //将字符串转换成时间
                        timeEnd=QDateTime::fromString(timeEnd_str,"yyyy-MM-dd hh:mm:ss");
                        /*QMessageBox messageq(QMessageBox::NoIcon,"提示","22--timeEnd:"+timeEnd.toString("yyyy-MM-dd hh:mm:ss"));
                        messageq.exec();*/
                        //将时间转成时间戳
                        int timeSegEndStamp=timeEnd.toTime_t();

                        /*QMessageBox messagea(QMessageBox::NoIcon,"提示","33--timeStamp:"+QString::number(timeStamp));
                        messagea.exec();

                        QMessageBox messages(QMessageBox::NoIcon,"提示","44--timeSegEndStamp:"+QString::number(timeSegEndStamp));
                        messages.exec();*/
                        if(timeStamp > timeSegEndStamp){
                            //跨时段操作了

                            getSegs();//当当前时间大于等于上次餐次的结束时间时，再调一次该函数，来获取新餐次的信息，并将本餐次刷卡次数清空为0,本餐次消费金额清空为0.00
                            newConsumeState=consumeState;

                            /*QMessageBox messages(QMessageBox::NoIcon,"提示","oldConsumeState:"+QString::number(oldConsumeState));
                            messages.exec();

                            QMessageBox messagesd(QMessageBox::NoIcon,"提示","newConsumeState:"+QString::number(newConsumeState));
                            messagesd.exec();*/
                            if(oldConsumeState==1 && newConsumeState==0){
                                //定额消费--普通消费
                                keyFlag=0;
                                cancelConsume();//定额消费变成普通消费时，需调用一下取消交易
                            }else if(oldConsumeState==0 && newConsumeState==1){
                                //普通消费--定额消费
                                keyFlag=1;
                                //reqConsume();
                                //脱机新建
                                if(rets==10){
                                    reqConsume(9);
                                }else{
                                    offLineConsume();
                                }
                            }

                            ui->label_totalMoney->setText("0.00");
                            ui->label_totalFrequency->setText("0");
                        }
                    }
                }
            }
        }else{
            myPtf("qwertyu");
            //若有流水，先取消消费
            cancelConsume();//关闭消费
            showPic();
        }
    }


    //如果到0点，则清空当日消费总金额
    if(QString::number(time.time().hour())=="0" || QString::number(time.time().hour())=="00" || time.time().hour()==0){
        if(attrib1==6){
            ui->label_totalMoney->setText("0.00");
        }else if(attrib1==1){
            ui->label_totalInCount->setText("0.00");
            ui->label_totalOutCount->setText("0.00");
        }

    }


    timerMain->start(500);
}
void MainWindow::player(QString url)
{
    /*QMessageBox message(QMessageBox::NoIcon,"提示","url:"+url);
    message.exec();*/
    players->setMedia(QMediaContent(QUrl::fromLocalFile(url)));
    players->play();
    myPtf("start play!\n");
}
void MainWindow::versions()
{
    consumeserVersions();//运行版本号--consumeser服务
    scanqrserVersions();//运行版本号--scanqrser服务
    secscreenVersions();//运行版本号--secscreen服务
    networkVersions();//运行版本号--network服务
}
void MainWindow::consumeserVersions()
{
    //运行版本号--consumeser服务
    QJsonObject jsonsend;
    jsonsend.insert("termCode",termCode);
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = UI_GET_VERSION;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_main->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+UI_GET_VERSION){
        //数据获取失败，可以尝试重连
        //QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
        //message.exec();
        /*ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,con-ret是"+QString::number(Cmd));
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));*/
        ui->groupBox_termInfoBg->show();
        //无脱机模式 modified by yangtao 2019-05-05
        if(AllowedOfflineConsume == 1)
            ui->label_termNote->setText("consumeser服务已断开，请检查网络连接状态并重启机器,\n点击'+'进入网络设置，点击'取消'进入脱机模式！");
        else
            ui->label_termNote->setText("consumeser服务已断开，请检查网络连接状态并重启机器,\n点击'+'进入网络设置！");

        consumeserVersion="";//获取不到时，清空版本号
    }else if(Cmd == CLOUD_SENDCMD_ERROR){
        serverError();
    }else{
        QString strShow = "";
        strShow.append(retJson);
        /*QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        message.exec();*/
        //解析json字符串
        //记录consumeser服务的获取时间
        //myPtf("strShow-------------------------%s:\n",strShow);
        QDateTime time = QDateTime::currentDateTime();
        consumeserStatus = time.toString("yyyy-MM-dd hh:mm:ss");

        consumeserVersion=jsonrecv.value("consumeser_version").toString();

        rets = jsonrecv.value("ret").toInt();
        //获取服务器连接状态
        int state=0;
        myPtf("rets-------------------------%d:\n",rets);

        if(rets==10){
            state=0;            
            myPtf("state1111-------------------------:\n");
            if(normalFlag==1){
                ui->groupBox_normalBg->show();
                ui->label_info_normal->setText("当前终端服务已链接,\n点击'进入联网模式'！");
                normalFlag=0;
                totalstr="";//还原输入框累加为空

                ui->groupBox_termInfoBg->hide();

            }
            ui->label_link->show();
            ui->label_unlink->hide();
            ui->label_linkName->setText("服务器已连接");
            //ui->label_workType->setText("消费");


            //给工作模式的Label设置背景色
            //ui->label_workType->setStyleSheet("QLabel{background:transparent;color: rgb(255, 255, 255);font: 500 13pt 'Ubuntu';}");//设置成透明
            ui->label_workType_offline->hide();
            ui->label_workType->show();

            //ui->pushButton_upload->show();//脱机模式时，“上传”按钮隐藏
            if(attrib1==2 || attrib1==3 || attrib1==6){
                 ui->pushButton_backs->show();//正常模式时，“合计”按钮(大)显示
            }


            ui->label_offlineFlowCount_title->hide();
            ui->label_offlineFlowCount->hide();
            ui->label_offlineFlowCount->setText("");
            flowFlag=0;
            if(consumeState==1){
                keyFlag=1;
            }
        }else{
            state=1;
            myPtf("state222222-------------------------:\n");
            myPtf("zzznormalFlag:%d\n",normalFlag);
            getAttrib1();
            attrib1=attrib1New;
            myPtf("qqAttr1:%d\n",attrib1);

            if(normalFlag==0){
                if(attrib1==1 || attrib1==7){
                    ui->groupBox_termInfoBg->show();
                    ui->label_termNote->setText("当前终端服务未链接，请检查网络连接状态并重启机器,\n点击'+'进入网络设置！");
                    return;
                }
                ui->groupBox_termInfoBg->show();
                //无脱机模式 modified by yangtao 2019-05-05
                if(AllowedOfflineConsume == 1)
                    ui->label_termNote->setText("当前终端服务未链接，请检查网络连接状态并重启机器,\n点击'菜单'进入网络设置，点击'取消'进入脱机模式！");
                else
                    ui->label_termNote->setText("当前终端服务未链接，请检查网络连接状态并重启机器,\n点击'菜单'进入网络设置！");
                //定额时，变换参数
                keyOtherFlag=0;
                cancelConsume();
                showPic();
                termID=0;//断网后，要清除termID
                getFlowNum();
                normalFlag=1;
                uploadFlag=0;
                keyFlag=0;
                consumeState=0;//还原启动固定消费标志
                totalstr="";//还原输入框累加为空
                ui->label_offlineFlowCount_title->show();
                ui->label_offlineFlowCount->show();


            }
            ui->label_unlink->show();
            ui->label_link->hide();
            ui->label_linkName->setText("服务器未连接");
            //ui->label_workType->setText("脱机");
            ui->groupBox_pannel->show();
            ui->groupBox_teller->hide();


            //给工作模式的Label设置背景色
            //ui->label_workType->setStyleSheet("QLabel{background:#f52600;color: rgb(255, 255, 255);font: 500 13pt 'Ubuntu';}");//设置成橙色
            ui->label_workType_offline->show();



            if(attrib1==2 || attrib1==3){
                ui->label_workType_offline->setText("脱机消费");
            }else if(attrib1==6){
                ui->label_workType_offline->setText("脱机洗衣");
            }

            ui->label_workType->hide();

            //ui->pushButton_upload->show();//脱机模式时，“上传”按钮显示
            ui->pushButton_backs->show();//正常模式时，“合计”按钮(大)隐藏
            ui->pushButton_examination->hide();
            ui->pushButton_cashier->hide();
            ui->pushButton_back->hide();
            ui->groupBox_totalPannelBg->hide();//脱机模式下，右侧信息栏隐藏
            ui->pushButton_gotoFlowlist->hide();//脱机模式下，右侧'查看流水'按钮隐藏
            ui->label_consumeLimitMoney->hide();//脱机模式下，单笔消费上限隐藏
            ui->label_timeSegName->hide();//脱机模式下，当前餐次隐藏
            termID=0;


        }
        if(state==1){
            myPtf("state33333-------------------------:\n");
            /*if(rets < ERRORNUM){
                ui->groupBox_termInfoBg->show();
                ui->label_termNote->setText("服务器连接失败，可尝试重启终端或咨询维护工程师！");
            }*/
        }else{
            myPtf("state44444-------------------------:\n");
            ui->groupBox_termInfoBg->hide();
            ui->label_termNote->setText("");
        }
    }
}
void MainWindow::serverError()
{
    keyFlag=1;
    ui->label_unlink->show();
    ui->label_link->hide();
    ui->label_linkName->setText("服务器未连接");

    ui->groupBox_termInfoBg->show();
    ui->label_termNote->setText("服务器连接失败，可尝试重启终端或咨询维护工程师！");
}
void MainWindow::scanqrserVersions()
{
    //运行版本号--scanqrser服务
    QJsonObject jsonsend;
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = UI_GET_SCANQRVERSION;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_main->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+UI_GET_SCANQRVERSION){
        //数据获取失败，可以尝试重连
        /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
        message.exec();*/
        /*ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,sca-ret是"+QString::number(Cmd));
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));*/
        ui->groupBox_termInfoBg->show();
        ui->label_termNote->setText("scanqrser服务已断开，请检查网络连接状态并重启机器！");
        scanqrserVersion="";//获取不到时，清空版本号
    }else if(Cmd == CLOUD_SENDCMD_ERROR){
	//TODO logic error,will never run
        serverError();
    }else{
        /*QString strShow = "";
        strShow.append(retJson);
        QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        message.exec();*/
        //解析json字符串
        //记录scanqrser服务的获取时间
        QDateTime time = QDateTime::currentDateTime();
        scanqrserStatus = time.toString("yyyy-MM-dd hh:mm:ss");

        //scanqrserVersion=jsonrecv.value("scanqrser_version").toString();
        //TODO scanqrser do not support this cmd ,do later
		scanqrserVersion = "2.0.1";
    }
}
void MainWindow::secscreenVersions(){
    //运行版本号--secscreen服务
    QJsonObject jsonsend;
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = SECSCREEN_GETVERSION_CMD;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_sec->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+SECSCREEN_GETVERSION_CMD){
        //数据获取失败，可以尝试重连
        //QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
        //message.exec();
        /*ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,sec-ret是"+QString::number(Cmd));
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));*/
        ui->groupBox_termInfoBg->show();
        ui->label_termNote->setText("secscreen服务已断开，请检查网络连接状态并重启机器！");
        secscreenVersion="";//获取不到时，清空版本号
    }else if(Cmd == CLOUD_SENDCMD_ERROR){
        serverError();
    }else{
        /*QString strShow = "";
        strShow.append(retJson);
        QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        message.exec();*/
        //解析json字符串
        //记录secscreen服务的获取时间
        QDateTime time = QDateTime::currentDateTime();
        secscreenStatus = time.toString("yyyy-MM-dd hh:mm:ss");

        secscreenVersion=jsonrecv.value("secscreen_version").toString();
    }
}
void MainWindow::networkVersions()
{
    //运行版本号--network服务
    QJsonObject jsonsend;
    jsonsend.insert("ui_version",uiVersion);
    jsonsend.insert("consume_version",consumeserVersion);
    jsonsend.insert("scanqr_version",scanqrserVersion);
    jsonsend.insert("secscreen_version",secscreenVersion);
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = RUNNING_VERSION_GET;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_network->send_Command(&Cmd,jsonsend,retJson);


    if(Cmd != 10000+RUNNING_VERSION_GET){
        //数据获取失败，可以尝试重连
        //QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd4));
        //message.exec();
        /*ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,net-ret是"+QString::number(Cmd));
        QTimer::singleShot(5000,ui->label_tips,SLOT(hide()));*/
        ui->groupBox_termInfoBg->show();
        ui->label_termNote->setText("network服务已断开，请检查网络连接状态并重启机器！");
        networkVersion="";//获取不到时，清空版本号
    }else if(Cmd == CLOUD_SENDCMD_ERROR){
        serverError();
    }else{
        /*QString strShow = "";
        strShow.append(retJson);
        QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        message.exec();*/
        //解析json字符串
        networkVersion=jsonrecv.value("version").toString();
    }
}
/*int MainWindow::processStatus()
{
    //更新各个进程状态
    QDateTime time = QDateTime::currentDateTime();
    terminalProjectStatus = time.toString("yyyy-MM-dd hh:mm:ss");
    QJsonObject jsonsend;
    jsonsend.insert("terminalProject",terminalProjectStatus);
    jsonsend.insert("consumeser",consumeserStatus);
    jsonsend.insert("secscreen",secscreenStatus);
    jsonsend.insert("scanqrser",scanqrserStatus);
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = ALL_PROCESS_STATUS_UPDATE;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_network->send_Command(&Cmd,jsonsend,retJson);


    if(Cmd != 10000+ALL_PROCESS_STATUS_UPDATE){
        //数据获取失败，可以尝试重连
        //QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd4));
        //message.exec();
        ui->label_tips->show();
        //ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
        ui->label_tips->setText("获取进程信息失败!");
        QTimer::singleShot(5000,ui->label_tips,SLOT(hide()));
        return 0;
    }else{
        //QString strShow = "";
        //strShow.append(retJson);
        //QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        //message.exec();
        //解析json字符串
        return 1;
    }
}*/
/*void MainWindow::getNetworkConnectStatus()
{
    //获取网络连接状态
    myCommon=new common();
    myCommon->scmd_network_common=scmd_network;
    myCommon->getNetworkConnet();

    QString netState= myCommon->networkStruct_common->netState;
    QString networkType = myCommon->networkStruct_common->networkType;
    QString powerrange = myCommon->networkStruct_common->powerrange;
    QString serverState = myCommon->networkStruct_common->serverState;
    if(netState=="1"){
        if(networkType=="1"){
            //有线网络
            ui->label_network->show();
            ui->label_connect01->hide();
            ui->label_connect02->hide();
            ui->label_connect->hide();
            ui->label_unconnect->hide();
            ui->label_4G->hide();
            ui->label_connectName->setText("有线连接");
        }else if(networkType=="2"){
            //无线网络
            if(powerrange=="15"){
                ui->label_connect01->show();
                ui->label_unconnect->hide();
                ui->label_connect02->hide();
                ui->label_connect->hide();
                ui->label_network->hide();
                ui->label_4G->hide();
                ui->label_connectName->setText("无线连接");
            }else if(powerrange=="20"){
                ui->label_connect02->show();
                ui->label_unconnect->hide();
                ui->label_connect01->hide();
                ui->label_connect->hide();
                ui->label_network->hide();
                ui->label_4G->hide();
                ui->label_connectName->setText("无线连接");
            }else{
                ui->label_connect->show();
                ui->label_unconnect->hide();
                ui->label_connect02->hide();
                ui->label_connect01->hide();
                ui->label_network->hide();
                ui->label_4G->hide();
                ui->label_connectName->setText("无线连接");
            }
        }else{
            //4G
            ui->label_4G->show();
            ui->label_connect->hide();
            ui->label_unconnect->hide();
            ui->label_connect02->hide();
            ui->label_connect01->hide();
            ui->label_network->hide();
            ui->label_connectName->setText("4G连接");
        }
    }else{
        //无网络
        ui->label_unconnect->show();
        ui->label_connect01->hide();
        ui->label_connect02->hide();
        ui->label_connect->hide();
        ui->label_network->hide();
        ui->label_4G->hide();
        ui->label_connectName->setText("未连接");
    }
}*/
/*void MainWindow::getWifiSetting()
{
    //获取无线网络参数--初始化函数
    QJsonObject jsonsend;
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = WIFI_STATUS_GET;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_network->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+WIFI_STATUS_GET){
        //数据获取失败，可以尝试重连
        //QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
        //message.exec();/
        ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
        QTimer::singleShot(5000,ui->label_tips,SLOT(hide()));
    }else{
        //QString strShow = "";
        //strShow.append(retJson3);
        //QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        //message.exec();/
        //解析json字符串
        essid=jsonrecv.value("essid").toString();
        password=jsonrecv.value("password").toString();
        networkFlag=jsonrecv.value("flag").toString();
    }
}*/
void MainWindow::getHardwareSerialno()
{
    //获取硬件唯一号--初始化函数
    int ilen=0;
    char *str=NULL;
    str=(char *)GetHardwareSerialNo(&ilen);
    myPtf("终端唯一识别码:%s\n", str);
    //char *转qstring
    termCode = QString(QLatin1String(str));

    /*QMessageBox message(QMessageBox::NoIcon,"提示","终端唯一识别码:"+termCode);
    message.exec();*/

}
int MainWindow::getTermInfo()
{
    //获取初始化界面信息
    QJsonObject jsonsend;
    jsonsend.insert("termCode",termCode);
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = UI_GET_TERMINFO;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_main->send_Command(&Cmd,jsonsend,retJson);

    if(Cmd != 10000+UI_GET_TERMINFO){
        //数据获取失败，可以尝试重连
        /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败www!,ret是"+QString::number(Cmd));
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
        QTimer::singleShot(5000,ui->label_tips,SLOT(hide()));
        return 0;
    }else if(Cmd == CLOUD_SENDCMD_ERROR){
        serverError();
        return 0;
    }else{
        /*QString strShow = "";
        strShow.append(retJson);
        QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        message.exec();*/
        //解析json字符串
        if(jsonrecv.value("ret").toInt()==10){

            if(jsonrecv.contains("term")){
                QJsonValue termValue = jsonrecv.value("term");
                if(termValue.isObject()){
                    QJsonObject termObject = termValue.toObject();
                    termNo = termObject.value("termNo").toInt();
                    termPassWord=termObject.value("termPassWord").toString();
                    QString schoolName = termObject.value("schoolName").toString();
                    QString areaName = termObject.value("areaName").toString();
                    QString unitName = termObject.value("unitName").toString();
                    QString groupName = termObject.value("groupName").toString();
                    attrib1 = termObject.value("attrib1").toInt();
                    setAttrib1(attrib1);

                    consumeType = termObject.value("consumeType").toInt();//交易类型
                    myPtf("consumeType:%d\n",consumeType);
                    termID=termObject.value("termID").toInt();
                    areaID=termObject.value("areaID").toInt();
                    char consumeLimitMoney[20];
                    sprintf(consumeLimitMoney,"%.2f",((float)termObject.value("consumeLimitMoney").toInt())/100);//单笔消费上限
                    ui->label_note_consumeLimitMoney->setText(consumeLimitMoney);
                    ui->label_consumeLimitMoney->setText("单笔消费上限:" + ui->label_note_consumeLimitMoney->text() + "元");//单笔消费上限
                    ui->label_terminalNumeber->setText(QString::number(termNo));
                    ui->label_schoolName->setText(schoolName);
                    ui->label_otherName->setText(areaName+"  "+unitName+"  "+groupName);
                    if(attrib1==1){
                        loginFlag=1;
                        ui->groupBox_login->show();//出纳时，用户需要登录
                        ui->groupBox_totalCashierBg->show();//出纳:当日充值-取钱信息栏(显示)
                        myPtf("asdf\n");
                        ui->groupBox_totalPannelBg->hide();//消费:当日消费-本餐次刷卡信息栏(隐藏)
                        ui->label_consumeTimes->hide();
                        ui->label_consumeTimes_title1->hide();
                        ui->label_consumeTimes_title2->hide();
                        //ui->groupBox_pannel->hide();//消费、洗衣右侧信息面板
                        ui->label_timeSegName->hide();//餐次名称
                        ui->label_consumeLimitMoney->hide();//单笔消费上限

                        ui->label_flowMoney_title->setText("出纳金额(元)：");


                        ui->label_remainMoneys_title->show();
                        //ui->label_remainMoneys->show();

                        ui->label_secAccoMoney_title->hide();
                        ui->label_secAccoMoney->hide();
                        ui->label_manageFee_title->hide();
                        ui->label_manageFee->hide();

                        ui->pushButton_backs->hide();
                        ui->pushButton_examination->show();
                        ui->pushButton_back->show();
                        ui->pushButton_cashier->show();

                        ui->groupBox_pannel->hide();
                        ui->groupBox_teller->show();


                        ui->label_workType->setText("存款");
                        ui->label_noteMes->setGeometry(24,380,410,50);

                        keyFlag_cashier=0;


                    }else if(attrib1==2 || attrib1==3){
                        cardId=0;
                        ui->label_workType->setText("消费");
                        ui->groupBox_totalCashierBg->hide();//出纳:当日充值-取钱信息栏(隐藏)
                        ui->groupBox_totalPannelBg->show();//消费:当日消费-本餐次刷卡信息栏(显示)
                        ui->label_consumeTimes->show();
                        ui->label_secAccoMoney_title->show();
                        ui->label_secAccoMoney->show();
                        ui->label_manageFee_title->show();
                        ui->label_manageFee->show();
                        ui->label_totalMoney_title->setText("本餐次消费金额(元):");
                        ui->label_consumeTimes_title1->show();
                        ui->label_consumeTimes_title2->show();
                        ui->groupBox_pannel->show();//消费、洗衣右侧信息面板
                        ui->label_timeSegName->show();//餐次名称
                        ui->label_consumeLimitMoney->show();//单笔消费上限

                        ui->label_remainMoneys_title->hide();

                        ui->label_flowMoney_title->setText("消费金额(元)：");

                        ui->pushButton_backs->show();
                        ui->pushButton_examination->hide();
                        ui->pushButton_back->hide();
                        ui->pushButton_cashier->hide();

                        ui->label_noteMes->setGeometry(24,160,410,50);

                        ui->groupBox_pannel->show();
                        ui->groupBox_teller->hide();
                        keyMenuFlag=0;
                        keyFlag_cashier=1;
                        //脱机模式转到正常模式
                        ui->groupBox_keyboardBg->hide();
                        ui->pushButton_gotoFlowlist->show();
                        ui->pushButton_keyboard->show();
                    }else if(attrib1==6){
                        ui->label_workType->setText("洗衣");
                        ui->groupBox_totalCashierBg->hide();//出纳:当日充值-取钱信息栏(隐藏)
                        ui->groupBox_totalPannelBg->show();//消费:当日消费-本餐次刷卡信息栏(显示)
                        ui->label_totalFrequency_title->hide();//洗衣：无本餐次刷卡(隐藏)
                        ui->label_totalFrequency->hide();
                        ui->label_consumeTimes->hide();
                        ui->label_secAccoMoney_title->hide();
                        ui->label_secAccoMoney->hide();
                        ui->label_manageFee_title->hide();
                        ui->label_manageFee->hide();
                        ui->label_totalMoney_title->setText("当日消费总券数(张):");
                        ui->label_consumeTimes_title1->hide();
                        ui->label_consumeTimes_title2->hide();
                        ui->groupBox_pannel->show();//消费、洗衣右侧信息面板
                        ui->label_timeSegName->hide();//餐次名称
                        ui->label_consumeLimitMoney->hide();//单笔消费上限
                        ui->label_flowMoney_title->setText("洗衣券(张):");
                        ui->label_remainMoney_title->setText("洗衣券剩余数量(张):");

                        ui->label_remainMoneys_title->hide();

                        ui->pushButton_backs->show();
                        ui->pushButton_examination->hide();
                        ui->pushButton_back->hide();
                        ui->pushButton_cashier->hide();

                        ui->groupBox_pannel->show();
                        ui->groupBox_teller->hide();
                        keyMenuFlag=0;
                        keyFlag_cashier=1;
                    }else if(attrib1==7){
                        ui->pushButton_gotoFlowlist->hide();
                        ui->label_workType->setText("查询");
                        ui->groupBox_totalCashierBg->hide();//出纳:当日充值-取钱信息栏(隐藏)
                        ui->groupBox_totalPannelBg->hide();//消费:当日消费-本餐次刷卡信息栏(显示)
                        ui->label_consumeTimes->hide();
                        ui->label_consumeTimes_title1->hide();
                        ui->label_consumeTimes_title2->hide();
                        ui->label_totalFrequency_title->hide();//查询：无本餐次刷卡(隐藏)
                        ui->groupBox_teller->hide();//出纳右侧信息面板
                        ui->groupBox_pannel->hide();//消费-洗衣右侧信息面板
                        ui->label_timeSegName->hide();//餐次名称
                        ui->label_consumeLimitMoney->hide();//单笔消费上限

                        ui->label_remainMoneys_title->hide();

                        ui->pushButton_backs->show();
                        ui->pushButton_examination->hide();
                        ui->pushButton_back->hide();
                        ui->pushButton_cashier->hide();

                        ui->label_flowMoney_title->hide();
                        keyFlag_cashier=0;
                        keyMenuFlag=1;
                        reqAccoInfo();
                    }
                }
            }
            //定额消费--(备注：在消费的模式下才有固定消费)
            if(attrib1==2 || attrib1==3){
                if(jsonrecv.contains("segs")){
                    QJsonValue valueArray=jsonrecv.value("segs");
                    if(valueArray.isArray()){
                        arraySegs = valueArray.toArray();
                    }
                }
            }
            return 1;
        }else{
            myPtf("error!!!------------------------------------");
            ui->groupBox_termInfoBg->show();
            ui->label_termNote->setText(jsonrecv.value("msg").toString());
            return 0;
        }
    }
}
void MainWindow::getSegs()
{
    //获取当前餐次信息
    //正式的数据，需要改数据类型
    int child_timeSegId;//时段ID
    QString child_timeSegStart;//时段开始时间(原值)
    QString child_timeSegStart_str;//时段开始时间(用于转换)
    QString child_timeSegName;//时段名称
    char child_consumeMoney[20];//时段金额
    int child_consumeState;//启动固定消费标志,1:启动,0:不启动
    int k= -1;

    //当前的时间
    QDateTime timeNow = QDateTime::currentDateTime();//获取当前系统时间
    QString timeStr = timeNow.toString("yyyy-MM-dd"); //设置显示格式
    QString beforeDaystr=timeNow.addDays(-1).toString("yyyy-MM-dd");//获取前一天时间
    QString afterDaystr=timeNow.addDays(1).toString("yyyy-MM-dd");//获取后一天时间
    int timeNowStamp = timeNow.toTime_t();//当前的时间戳

    //获取的时间
    QDateTime timeGet;
    for(int i=0;i<arraySegs.count();i++){
        QJsonValue childValue = arraySegs[i];

        if(childValue.isObject()){
            //选择相应时段的固定金额进行消费
            QJsonObject childObject =childValue.toObject();
            if(childObject.contains("timeSegStart")){
                //获取时段开始时间
                QJsonValue valueJson = childObject.value("timeSegStart");
                child_timeSegStart_str =valueJson.toString();
            }
            //将获得的时分秒，拼成年月日时分秒
            child_timeSegStart_str = timeStr +' '+child_timeSegStart_str;
            timeGet=QDateTime::fromString(child_timeSegStart_str,"yyyy-MM-dd hh:mm:ss");//将字符串转换成时间
            int timeGetStamp = timeGet.toTime_t();//获取的时间戳

            if(timeNowStamp >= timeGetStamp){
                k=i;

            }else{
                break;
            }
        }
    }
    QJsonValue childValue;
    QJsonValue nextChildValue;//下一个餐次开始时间
    if(k>=0 && k<3){
        //当天的早餐--午餐--晚餐
        childValue = arraySegs[k];
        nextChildValue = arraySegs[k+1];
    }
    else if(k==3){
        //当天的夜宵
        childValue = arraySegs[k];
        nextChildValue = arraySegs[0];
    }
    else
    {
        //跨天的夜宵(前一天)
        childValue = arraySegs[3];
        nextChildValue = arraySegs[0];
    }


    if(nextChildValue.isObject()){
        //下一个餐次信息
        QJsonObject childObject =nextChildValue.toObject();
        if(childObject.contains("timeSegStart")){
            //获取时段开始时间(下一个餐次,即：本餐次结束时间)
            QJsonValue valueJson = childObject.value("timeSegStart");
            child_timeSegStart=valueJson.toString();
            timeSegEnd=child_timeSegStart;
            /*QMessageBox messagea(QMessageBox::NoIcon,"提示","ww--timeSegEnd:"+timeSegEnd);
            messagea.exec();*/
            if(k>=0 && k<3){
                //当天的早餐--午餐--晚餐
                timeEnd_str=timeStr+' '+timeSegEnd;
                /*QMessageBox messagea(QMessageBox::NoIcon,"提示","num11--timeStamp:"+timeEnd_str);
                messagea.exec();*/
            }else if(k==3){
                //当天的夜宵
                timeEnd_str=afterDaystr+' '+timeSegEnd;
                /*QMessageBox messagea(QMessageBox::NoIcon,"提示","num22--timeStamp:"+timeEnd_str);
                messagea.exec();*/
            }else{
                //跨天的夜宵(前一天)
                timeEnd_str=beforeDaystr+' '+timeSegEnd;
                /*QMessageBox messagea(QMessageBox::NoIcon,"提示","num33--timeStamp:"+timeEnd_str);
                messagea.exec();*/
            }
        }
    }

    if(childValue.isObject()){
        //本餐次信息
        QJsonObject childObject =childValue.toObject();
        if(childObject.contains("timeSegId")){
            //获取时段ID
            QJsonValue valueJson = childObject.value("timeSegId");
            child_timeSegId =valueJson.toInt();
            timeSegId=child_timeSegId;
        }
        if(childObject.contains("timeSegStart")){
            //获取时段开始时间(本餐次)
            QJsonValue valueJson = childObject.value("timeSegStart");
            child_timeSegStart=valueJson.toString();
            timeSegStart=child_timeSegStart;
        }
        if(childObject.contains("timeSegName")){
            //获取时段名称
            QJsonValue valueJson = childObject.value("timeSegName");
            child_timeSegName =valueJson.toString();
            timeSegName=child_timeSegName;
        }
        if(childObject.contains("consumeMoney")){
            //获取时段金额
            QJsonValue valueJson = childObject.value("consumeMoney");
            sprintf(child_consumeMoney,"%.2f",((float)valueJson.toInt())/100);
            ui->label_note_secAccoMoney->setText(child_consumeMoney);
            consumeMoney=ui->label_note_secAccoMoney->text();
        }
        if(childObject.contains("consumeState")){
            //获取启动固定消费标志,1:启动,0:不启动
            QJsonValue valueJson = childObject.value("consumeState");
            child_consumeState =valueJson.toInt();
            consumeState=child_consumeState;
        }
    }

    if(attrib1==2 || attrib1==3){
        if(consumeState==1){
            cancelConsume();//切换时段的时候，把上一笔请求取消掉
            ui->label_timeSegName->setText("当前餐次:"+timeSegName+",消费金额:"+consumeMoney);//餐次名称
            ui->label_workType->setText("定额");
            ui->label_input->setText(consumeMoney);
            moneyFloat=consumeMoney.toDouble();
            showAll("需支付:"+QString::number(moneyFloat)+"元",140,200,40);

            keyFlag=1;//让终端键盘在定额消费模式时不可点击
            myPtf("dinge  start reqConsume-----------------\n");

            //reqConsume();//再重新请求
            //脱机新建
            if(rets==10){
                reqConsume(10);
            }else{
                offLineConsume();
            }
        }else{
            ui->label_timeSegName->setText("当前餐次:"+timeSegName);//餐次名称
            ui->label_workType->setText("消费");
            ui->label_input->setText("");
            keyFlag=0;//在普通消费模式时可以点击
        }
    }



    /*QMessageBox message(QMessageBox::NoIcon,"提示","timeSegId:"+QString::number(timeSegId)+"--timeSegName:"+timeSegName+"--consumeMoney:"+consumeMoney+"--consumeState:"+QString::number(consumeState)+"--timeSegStart:"+timeSegStart+"--timeSegEnd:"+timeSegEnd+"--timeEnd_str"+timeEnd_str);
    message.exec();*/
}

void MainWindow::getTotal()
{
    //获取本餐次消费金额、本餐次消费总笔数
    //消费信息--初始化
    QJsonObject jsonsend;
    QDateTime dateTime = QDateTime::currentDateTime();
    QString time = dateTime.toString("yyyy-MM-dd");
    if(attrib1==1 || attrib1==6){
        jsonsend.insert("startTime",time+" 00:00:00");
        jsonsend.insert("endTime",time+" 23:59:59");
    }else{
        /*QMessageBox messagea(QMessageBox::NoIcon,"提示","qqtimeSegStart:"+timeSegStart+"qqtimeSegEnd:"+timeSegEnd);
        messagea.exec();*/
        jsonsend.insert("startTime",time+" "+timeSegStart);//当前时段开始时间
        jsonsend.insert("endTime",timeEnd_str);//下一时段开始时间
        //jsonsend.insert("endTime",time+" "+timeSegEnd);//下一时段开始时间
    }

    jsonsend.insert("termCode",termCode);
    jsonsend.insert("termID",termID);
    jsonsend.insert("pageNum","0");
    jsonsend.insert("pageSize","0");

    //发送数据
    QJsonObject jsonrecv;
    int Cmd = UI_GET_FLOWTOTAL;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_main->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+UI_GET_FLOWTOTAL){
       //数据获取失败，可以尝试重连
       /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败ee!,ret是"+QString::number(Cmd));
       message.exec();*/
       ui->label_tips->show();
       ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
       QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(Cmd == CLOUD_SENDCMD_ERROR){
        serverError();
    }else{
       /*QString strShow = "";
       strShow.append(retJson);
       QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
       message.exec();*/
       //解析json字符串
       if(jsonrecv.value("ret").toInt()==10){
           if(attrib1==1){
               char inSum[20];
               sprintf(inSum,"%.2f",((float)jsonrecv.value("inSum").toInt())/100);
               ui->label_totalInCount->setText(inSum);

               char outSum[20];
               sprintf(outSum,"%.2f",((float)jsonrecv.value("outSum").toInt())/100);
               ui->label_totalOutCount->setText(outSum);
           }else if(attrib1==2 || attrib1==3){
               //本餐次消费金额
               char sum[20];
               sprintf(sum,"%.2f",((float)jsonrecv.value("sum").toInt())/100);
               ui->label_totalMoney->setText(sum);
               //本餐次消费笔数
               ui->label_totalFrequency->setText(QString::number(jsonrecv.value("count").toInt()));
           }else if(attrib1==6){
               ui->label_totalMoney->setText(QString::number(jsonrecv.value("sum").toInt()));
           }
       }
    }
}
int MainWindow::inputReg()
{

    //验证输入框金额(或票数)
    //金额(请输入正整数或保留两位小数)
    QString moneyStr =ui->label_input->text();
    QString consumeLimitMoney =ui->label_note_consumeLimitMoney->text();
    QRegExp numRegExp = QRegExp("[0-9]+(.[0-9]{0,2})?");
    QRegExpValidator * numValidator = new QRegExpValidator(numRegExp,this);
    int pos=0;
    QValidator::State result = numValidator->validate(moneyStr,pos);
    //金额(请输入正整数)
    QRegExp portRegExp = QRegExp("[0-9]+$");
    QRegExpValidator * portValidator = new QRegExpValidator(portRegExp,this);
    int poss=0;
    QValidator::State results = portValidator->validate(moneyStr,poss);

    int moneyInput=moneyStr.toInt() * 100;
    switch(attrib1)
    {
    case 1:
        if(moneyStr == ""){
            ui->label_tips->show();
            ui->label_tips->setText("请输入金额!");
            QTimer::singleShot(2000,ui->label_tips,SLOT(hide()));
            if(attrib1==1){
                cashierFlag=0;
                cashier="1";
                ui->label_workType->setText("存款");
            }
            return -1;
        }else if(moneyStr.toDouble() ==0 || moneyStr.toDouble()==0.0 || moneyStr.toDouble()==0.00){
            ui->label_tips->show();
            ui->label_tips->setText("您输入的金额不能为0!");
            QTimer::singleShot(2000,ui->label_tips,SLOT(hide()));
            ui->label_input->setText("");
            if(attrib1==1){
                cashierFlag=0;
                cashier="1";
                ui->label_workType->setText("存款");
            }
            return -1;
        }else if(moneyInput > intRemainMoney){
            if(cashier=="2"){
                ui->label_tips->show();
                ui->label_tips->setText("取款金额不得超过个人账户余额!");
                QTimer::singleShot(2000,ui->label_tips,SLOT(hide()));
                ui->label_input->setText("");
                if(attrib1==1){
                    cashierFlag=0;
                    cashier="1";
                    ui->label_workType->setText("存款");
                }
                return -1;
            }
        }else if(moneyStr.toDouble() > MAXMONEY){
            if(cashier=="1"){
                ui->label_tips->show();
                ui->label_tips->setText("存款不得超过!"+QString::number(MAXMONEY)+"元");
                QTimer::singleShot(2000,ui->label_tips,SLOT(hide()));
                ui->label_input->setText("");
                return -1;
            }
        }else if(result != QValidator::Acceptable){
            ui->label_tips->show();
            ui->label_tips->setText("请输入正整数或保留两位小数!");
            QTimer::singleShot(1000,ui->label_tips,SLOT(hide()));
            if(attrib1==1){
                cashierFlag=0;
                cashier="1";
                ui->label_workType->setText("存款");
            }
            return -1;
        }
        break;
    case 2:
    case 3:
        if(rets == 10){
            if(moneyStr == ""){
                ui->label_tips->show();
                ui->label_tips->setText("请输入金额!");
                QTimer::singleShot(2000,ui->label_tips,SLOT(hide()));
                return -1;
            }else if(moneyStr.toDouble() > consumeLimitMoney.toDouble()){
                ui->label_tips->show();
                ui->label_tips->setText("请输入小于"+consumeLimitMoney+"的金额!");
                QTimer::singleShot(2000,ui->label_tips,SLOT(hide()));
                ui->label_input->setText("");
                return -1;
            }else if(result != QValidator::Acceptable){
                ui->label_tips->show();
                ui->label_tips->setText("请输入正整数或保留两位小数!");
                QTimer::singleShot(1000,ui->label_tips,SLOT(hide()));
                return -1;
            }
        }else{
            if(moneyStr == ""){
                ui->label_tips->show();
                ui->label_tips->setText("请输入金额!");
                QTimer::singleShot(2000,ui->label_tips,SLOT(hide()));
                return -1;
            }else if(moneyStr.toDouble() > OFFLINEMAXMONEY){
                ui->label_tips->show();
                ui->label_tips->setText("请rr输入小于"+QString::number(OFFLINEMAXMONEY)+"的金额!");
                QTimer::singleShot(2000,ui->label_tips,SLOT(hide()));
                ui->label_input->setText("");
                return -1;
            }else if(result != QValidator::Acceptable){
                ui->label_tips->show();
                ui->label_tips->setText("请输入正整数或保留两位小数!");
                QTimer::singleShot(1000,ui->label_tips,SLOT(hide()));
                return -1;
            }
        }

        break;
    case 6:
        if(moneyStr == ""){
            ui->label_tips->show();
            ui->label_tips->setText("请输入金额!");
            QTimer::singleShot(2000,ui->label_tips,SLOT(hide()));
            return -1;
        }else if(results != QValidator::Acceptable){
            ui->label_tips->show();
            ui->label_tips->setText("请输入正整数!");
            QTimer::singleShot(2000,ui->label_tips,SLOT(hide()));
            return -1;
        }
        break;
    default:
        break;
    }
    return 1;
}
void MainWindow::on_pushButton_gotoFlowlist_clicked()
{
    //跳转至流水列表
    if(consumeState==1){
        cancelConsume();
    }
    flow * myFlow = new flow(this);
    //myFlow->scmd_network_flow=scmd_network;
    myFlow->scmd_main_flow=scmd_main;
    myFlow->termCode_flow=termCode;
    myFlow->termPassWord_flow=termPassWord;
    myFlow->termNo_flow=termNo;
    myFlow->termID_flow=termID;
    myFlow->attrib1_flow=attrib1;
    myFlow->ret_flow=rets;
    myFlow->exec();
    getTotal();
    if(consumeState==1){
       ui->label_input->setText(consumeMoney);
       //reqConsume();
       //脱机新建
       if(rets==10){
           reqConsume(11);
       }else{
           offLineConsume();
       }
    }
    delete myFlow;
}
void MainWindow::on_pushButton_keyboard_clicked()
{
    //软键盘切换
    keyboardFlag=1;
    ui->groupBox_keyboardBg->show();
    ui->pushButton_gotoFlowlist->hide();
    ui->pushButton_keyboard->hide();
    //ui->groupBox_pannel->hide();
    ui->groupBox_totalPannelBg->hide();
    ui->groupBox_totalCashierBg->hide();

    ui->label_consumeLimitMoney->hide();
    ui->label_timeSegName->hide();

    myPtf("daqqqattrib1%d\n",attrib1);

    //脱机新建
    if(rets !=10 ){
        //ui->pushButton_upload->show();//脱机模式时，“上传”按钮显示
        ui->pushButton_cashier->hide();
        ui->pushButton_back->hide();
        ui->pushButton_examination->hide();
        ui->pushButton_cashier->hide();


        ui->label_offlineFlowCount_title->hide();
        ui->label_offlineFlowCount->hide();
    }
    if(attrib1==1){
        ui->pushButton_backs->hide();
    }else{
       ui->pushButton_backs->show();
    }
}
void MainWindow::on_pushButton_termMenu_clicked()
{
    //跳转至登录
    on_pushButton_menu_clicked();
}
void MainWindow::on_pushButton_cancelConsume_clicked()
{
    //取消消费
    //将累加金额，清空还原
    totalstr="";
    memset(totalnum,0, sizeof(totalnum));
    arrnum=0;//数组下标
    ui->label_sum->setText("");

    addFlag=0;
    keyFlag=0;
    cancelConsume();
}
void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    int key=ev->key();
    myPtf("qwert------key:%d\n",key);


    int spanTime = getNowTime() - fn_getLastTime();
    myPtf("spanTime = %d\n",spanTime);
    if (spanTime < 1000 && spanTime > 0)
    {
      return;
    }

    if (ev->isAutoRepeat())
    return;
    if(keyFlag==1){
        myPtf("qqqqqqqqqqqwwwww\n");
        return;
    }else{
        myPtf("dddffffff\n");
    }
    //清屏时，按键处理
    if(clearFlag==1){
        clearScreen(CLEARTIMESTAMP,FSTSCREEN);
        clearScreen(CLEARTIMESTAMP,SECSCREEN);
        hideConsumeMsg();
        ui->label_input->setText("");//输入框清空
        showPic();
        clearFlag=0;
    }


    //int keytext=ev->nativeVirtualKey();
    //myPtf("keytext:%d\n",keytext);

    if(loginFlag==1){
        //出纳输入密码
        inputTxt=ui->lineEdit_password->text();
        if(key==49){
            myPtf("textqqqq\n");
            ui->lineEdit_password->setText(inputTxt+"1");
        }else if(key==50){
            ui->lineEdit_password->setText(inputTxt+"2");
        }else if(key==51){
            ui->lineEdit_password->setText(inputTxt+"3");
        }else if(key==52){
            ui->lineEdit_password->setText(inputTxt+"4");
        }else if(key==53){
            ui->lineEdit_password->setText(inputTxt+"5");
        }else if(key==54){
            ui->lineEdit_password->setText(inputTxt+"6");
        }else if(key==55){
            ui->lineEdit_password->setText(inputTxt+"7");
        }else if(key==56){
            ui->lineEdit_password->setText(inputTxt+"8");
        }else if(key==57){
            ui->lineEdit_password->setText(inputTxt+"9");
        }else if(key==42){
            inputTxt = inputTxt.left(inputTxt.length()-1);
            ui->lineEdit_password->setText(inputTxt);//返回（即：删除一位字符
        }else if(key==48){
            ui->lineEdit_password->setText(inputTxt+"0");
        }else if(key==16777264 || key==46){//46:外接键盘--“.”小数点
            ui->lineEdit_password->setText(inputTxt+".");//小数点
        }else if(key==16777216 || key==16777219){//16777219:外接键盘--“Back Space”
            ui->lineEdit_password->setText("");//取消（即：清空）
        }else if(key==16777265){//菜单
            ui->lineEdit_password->setText(inputTxt);//取消（即：清空）
        }else if(key==16777220 || key==16777221){//确认~~~16777221:外接键盘--“Enter”
            on_pushButton_login_clicked();
        }
    }else if(loginFlag==2){
        //消费输入密码
        inputTxt=ui->lineEdit_password_consume->text();
        if(key==49){
            ui->lineEdit_password_consume->setText(inputTxt+"1");
        }else if(key==50){
            ui->lineEdit_password_consume->setText(inputTxt+"2");
        }else if(key==51){
            ui->lineEdit_password_consume->setText(inputTxt+"3");
        }else if(key==52){
            ui->lineEdit_password_consume->setText(inputTxt+"4");
        }else if(key==53){
            ui->lineEdit_password_consume->setText(inputTxt+"5");
        }else if(key==54){
            ui->lineEdit_password_consume->setText(inputTxt+"6");
        }else if(key==55){
            ui->lineEdit_password_consume->setText(inputTxt+"7");
        }else if(key==56){
            ui->lineEdit_password_consume->setText(inputTxt+"8");
        }else if(key==57){
            ui->lineEdit_password_consume->setText(inputTxt+"9");
        }else if(key==42){
            inputTxt = inputTxt.left(inputTxt.length()-1);
            ui->lineEdit_password_consume->setText(inputTxt);//返回（即：删除一位字符
        }else if(key==48){
            ui->lineEdit_password_consume->setText(inputTxt+"0");
        }else if(key==16777264 || key==46){//46:外接键盘--“.”小数点
            ui->lineEdit_password_consume->setText(inputTxt+".");//小数点
        }else if(key==16777216 || key==16777219){//16777219:外接键盘--“Back Space”
            ui->lineEdit_password_consume->setText("");//取消（即：清空）
        }else if(key==16777265){//菜单
            ui->lineEdit_password_consume->setText(inputTxt);//取消（即：清空）
        }else if(key==16777220 || key==16777221){//确认~~~16777221:外接键盘--“Enter”
            on_pushButton_consume_clicked();
        }
    }else{
        inputTxt=ui->label_input->text();
        if(clearFlag==1){
            inputTxt="";
        }
        myPtf("qq--keyOtherFlag:%d\n",keyOtherFlag);
        myPtf("qq--keyMenuFlag:%d\n",keyMenuFlag);
        if(key==49){
            if(keyOtherFlag==1){
                return;
            }
            if(keyMenuFlag==1){
                return;
            }

            ui->label_input->setText(inputTxt+"1");
            if(point==1 || point==2){
                point = point +1;
            }else if(point==3){
                ui->label_input->setText(inputTxt);
            }
            if(inputTxt=="0"){
                ui->label_input->setText("0");
            }
            if(inputTxt.length()>6){
                ui->label_input->setText(inputTxt);
            }
        }else if(key==50){
            if(keyOtherFlag==1){
                return;
            }
            if(keyMenuFlag==1){
                return;
            }

            ui->label_input->setText(inputTxt+"2");
            if(point==1 || point==2){
                point = point +1;
            }else if(point==3){
                ui->label_input->setText(inputTxt);
            }
            if(inputTxt=="0"){
                ui->label_input->setText("0");
            }
            if(inputTxt.length()>6){
                ui->label_input->setText(inputTxt);
            }
        }else if(key==51){
            if(keyOtherFlag==1){
                return;
            }
            if(keyMenuFlag==1){
                return;
            }

            ui->label_input->setText(inputTxt+"3");
            if(point==1 || point==2){
                point = point +1;
            }else if(point==3){
                ui->label_input->setText(inputTxt);
            }
            if(inputTxt=="0"){
                ui->label_input->setText("0");
            }
            if(inputTxt.length()>6){
                ui->label_input->setText(inputTxt);
            }
        }else if(key==52){
            if(keyOtherFlag==1){
                return;
            }
            if(keyMenuFlag==1){
                return;
            }

            ui->label_input->setText(inputTxt+"4");
            if(point==1 || point==2){
                point = point +1;
            }else if(point==3){
                ui->label_input->setText(inputTxt);
            }
            if(inputTxt=="0"){
                ui->label_input->setText("0");
            }
            if(inputTxt.length()>6){
                ui->label_input->setText(inputTxt);
            }
        }else if(key==53){
            if(keyOtherFlag==1){
                return;
            }
            if(keyMenuFlag==1){
                return;
            }

            ui->label_input->setText(inputTxt+"5");
            if(point==1 || point==2){
                point = point +1;
            }else if(point==3){
                ui->label_input->setText(inputTxt);
            }
            if(inputTxt=="0"){
                ui->label_input->setText("0");
            }
            if(inputTxt.length()>6){
                ui->label_input->setText(inputTxt);
            }
        }else if(key==54){
            if(keyOtherFlag==1){
                return;
            }
            if(keyMenuFlag==1){
                return;
            }

            ui->label_input->setText(inputTxt+"6");
            if(point==1 || point==2){
                point = point +1;
            }else if(point==3){
                ui->label_input->setText(inputTxt);
            }
            if(inputTxt=="0"){
                ui->label_input->setText("0");
            }
            if(inputTxt.length()>6){
                ui->label_input->setText(inputTxt);
            }
        }else if(key==55){
            if(keyOtherFlag==1){
                return;
            }
            if(keyMenuFlag==1){
                return;
            }

            ui->label_input->setText(inputTxt+"7");
            if(point==1 || point==2){
                point = point +1;
            }else if(point==3){
                ui->label_input->setText(inputTxt);
            }
            if(inputTxt=="0"){
                ui->label_input->setText("0");
            }
            if(inputTxt.length()>6){
                ui->label_input->setText(inputTxt);
            }
        }else if(key==56){
            if(keyOtherFlag==1){
                return;
            }
            if(keyMenuFlag==1){
                return;
            }

            ui->label_input->setText(inputTxt+"8");
            if(point==1 || point==2){
                point = point +1;
            }else if(point==3){
                ui->label_input->setText(inputTxt);
            }
            if(inputTxt=="0"){
                ui->label_input->setText("0");
            }
            if(inputTxt.length()>6){
                ui->label_input->setText(inputTxt);
            }
        }else if(key==57){
            if(keyOtherFlag==1){
                return;
            }
            if(keyMenuFlag==1){
                return;
            }

            ui->label_input->setText(inputTxt+"9");
            if(point==1 || point==2){
                point = point +1;
            }else if(point==3){
                ui->label_input->setText(inputTxt);
            }
            if(inputTxt=="0"){
                ui->label_input->setText("0");
            }
            if(inputTxt.length()>6){
                ui->label_input->setText(inputTxt);
            }
        }else if(key==42){
            if(keyOtherFlag==1){
                return;
            }
            if(inputTxt==""){
                ui->label_sum->setText("");
                //将累加金额，清空还原
                memset(totalnum,0, sizeof(totalnum));
                arrnum=0;//数组下标
            }
            if(attrib1==1){
                if(accoInfoFlag==1 && inputTxt ==""){
                    recoveryAccoInfo();
                    accoInfoFlag=0;
                }else{
                    inputTxt = inputTxt.left(inputTxt.length()-1);
                    ui->label_input->setText(inputTxt);//返回（即：删除一位字符）

                    point =point -1;
                    if(point==0){
                        point=0;
                    }
                }
            }else if(attrib1==7){
                returnAccoInfo();
            }else{
                inputTxt = inputTxt.left(inputTxt.length()-1);
                ui->label_input->setText(inputTxt);//返回（即：删除一位字符）
                point =point -1;
                if(point==0){
                    point=0;
                }
            }


        }else if(key==48){
            if(keyOtherFlag==1){
                return;
            }
            if(keyMenuFlag==1){
                return;
            }

            ui->label_input->setText(inputTxt+"0");
            if(point==1 || point==2){
                point = point +1;
            }else if(point==3){
                ui->label_input->setText(inputTxt);
            }
            if(inputTxt=="0"){
                ui->label_input->setText("0");
            }
            if(inputTxt.length()>6){
                ui->label_input->setText(inputTxt);
            }
        }else if(key==16777264 || key==46){//46:外接键盘--“.”小数点
            if(keyOtherFlag==1){
                return;
            }
            if(keyMenuFlag==1){
                return;
            }
            if(attrib1==6){
                return;
            }
            if(ui->label_input->text() != ""){
                myPtf("aaaa\n");
                if(connectFlag==0 && attrib1==6){
                    ui->label_input->setText(inputTxt);//小数点
                }
                ui->label_input->setText(inputTxt+".");//小数点
            }else{
                myPtf("bbbb\n");
                ui->label_input->setText("0.");
            }

            if(point==1 || point==2 || point==3){
                ui->label_input->setText(inputTxt);
            }else{
                point=1;
            }
            if(inputTxt.length()>6){
                ui->label_input->setText(inputTxt);
            }

        }else if(key==16777216 || key==16777219){//16777219:外接键盘--“Back Space”
            //无脱机模式 modified by yangtao 2019-05-05
            //进入脱机模式
            if((rets != 10)&&(AllowedOfflineConsume == 1)){
                myPtf("yyww\n");
                getAttrib1();
                attrib1=attrib1New;
                myPtf("wwAttr1:%d\n",attrib1);

                if(attrib1==2 || attrib1==3 || attrib1==6){
                    //脱机模式--进入脱机模式
                    offlineFlag=1;
                    keyFlag=0;
                    connectFlag=0;

                    keyMenuFlag=0;//在脱机模式下，打开键盘
                    ui->groupBox_termInfoBg->hide();
                    //ui->pushButton_upload->show();//脱机模式时，“上传”按钮显示
                    ui->pushButton_backs->show();//正常模式时，“合计”按钮隐藏
                    ui->groupBox_flowBg->hide();

                    myPtf("yyww1\n");
                }
            }

            if(attrib1==7 || attrib1==1){
                return;
            }
            ui->label_input->setText("");//取消（即：清空）
            ui->label_sum->setText("");
            point=0;
            //将累加金额，清空还原
            memset(totalnum,0, sizeof(totalnum));
            arrnum=0;//数组下标
            totalstr="";

            addFlag=0;
            if(attrib1==2 || attrib1==3){
                if(consumeState==0){
                    on_pushButton_cancelConsume_clicked();
                }
            }else if(attrib1==6){
                on_pushButton_cancelConsume_clicked();
            }else if(attrib1==1){
                cancelAccoInfo();
            }

        }else if(key==16777265){//菜单
            myPtf("ddddaaa%d\n",rets);
            if(rets != 10){
                //跳转至登录
                on_pushButton_menu_clicked();
                return;
            }
            if(keyOtherFlag==1){
                return;
            }

            if(attrib1==1){
                //点击卡自检,刷卡，获取账户信息
                if(accoInfoFlag==0){
                    myPtf("zijian------------------\n");
                    reqAccoInfo();
                }else{
                    if(cashierFlag==1){//切换至存款
                        myPtf("cunkuan------------------\n");
                        cashier="1";
                        ui->label_workType->setText("存款");
                        cashierFlag=0;
                    }else{//切换至取款
                        myPtf("qukuan------------------\n");
                        cashier="2";
                        ui->label_workType->setText("取款");
                        cashierFlag=1;
                    }
                }
            }else{
                if(inputTxt=="" && ui->label_sum->text()==""){
                    return;
                }
                if(inputTxt=="0"){
                    addFlag=0;
                }else{
                    addFlag=1;
                }
                //做"+"号,累加金额
                point=0;

                //临时变量
                double tm = 0;
                //累加
                if(totalnum[0]==0){
                    totalnum[arrnum] = inputTxt.toDouble();
                    tm = inputTxt.toDouble();
                }else{
                    totalnum[arrnum] = inputTxt.toDouble();
                    for(int i=0;i<100;i++){
                        if(totalnum[i]>0){
                            tm = tm + totalnum[i];
                        }
                    }
                }
                arrnum = arrnum+1;
                ui->label_input->setText("");
                totalstr="";
                for(int i=0;i<100;i++){

                    if(totalnum[i]>0){
                        totalstr += QString::number(totalnum[i])+"+";
                    }

                }

                if(totalstr.length()>0){

                    totalstr.left(totalstr.length()-1);
                }
                if(attrib1==2 || attrib1==3 || attrib1==6){
                    ui->label_sum->setText(totalstr);
                }

            }
        }else if(key==16777220 || key==16777221){//确认~~~16777221:外接键盘--“Enter”
            if(keyOtherFlag==1){
                return;
            }
            on_pushButton_submits_clicked();
        }else if(key==43){
            //43:外接键盘--“+”号
            if(keyOtherFlag==1){
                return;
            }

            if(attrib1==1){
                //点击卡自检,刷卡，获取账户信息
                if(accoInfoFlag!=0){
                    if(cashierFlag==1){//切换至存款
                        cashier="1";
                        ui->label_workType->setText("存款");
                        cashierFlag=0;
                    }
                }
            }else{
                //做"+"号,累加金额
                point=0;
                addFlag=1;
                //临时变量
                double tm = 0;
                //累加
                if(totalnum[0]==0){
                    totalnum[arrnum] = inputTxt.toDouble();
                    tm = inputTxt.toDouble();
                }else{
                    totalnum[arrnum] = inputTxt.toDouble();
                    for(int i=0;i<100;i++){
                        if(totalnum[i]>0){
                            tm = tm + totalnum[i];
                        }
                    }
                }
                arrnum = arrnum+1;
                ui->label_input->setText("");

                totalstr="";
                for(int i=0;i<100;i++){

                    if(totalnum[i]>0){
                        totalstr += QString::number(totalnum[i])+"+";
                    }

                }

                if(totalstr.length()>0){

                    totalstr.left(totalstr.length()-1);
                }

                if(attrib1==2 || attrib1==3 || attrib1==6){
                    ui->label_sum->setText(totalstr);
                }
            }
        }else if(key==45){
            //45:外接键盘--“-”号
            if(keyOtherFlag==1){
                return;
            }
            if(attrib1==1){
                //点击卡自检,刷卡，获取账户信息
                if(accoInfoFlag!=0){
                    if(cashierFlag==0){//切换至取款
                        cashier="2";
                        ui->label_workType->setText("取款");
                        cashierFlag=1;
                    }
                }
            }
        }else if(key==47){
            //47:外接键盘--“/”号
            if(keyOtherFlag==1){
                return;
            }
            if(attrib1==1){
                //点击卡自检,刷卡，获取账户信息
                if(accoInfoFlag==0){
                    reqAccoInfo();
                }
            }
        }
    }
}
void MainWindow::on_pushButton_1_clicked()
{
    //按键--1
    if(keyFlag==1){
        return;
    }
    if(keyOtherFlag==1){
        return;
    }
    if(keyMenuFlag==1){
        return;
    }
    inputTxt=ui->label_input->text();
    ui->label_input->setText(inputTxt+"1");
    if(point==1 || point==2){
        point = point +1;
    }else if(point==3){
        ui->label_input->setText(inputTxt);
    }
    if(inputTxt=="0"){
        ui->label_input->setText("0");
    }
    if(inputTxt.length()>6){
        ui->label_input->setText(inputTxt);
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    //按键--2
    if(keyFlag==1){
        return;
    }
    if(keyOtherFlag==1){
        return;
    }
    if(keyMenuFlag==1){
        return;
    }
    inputTxt=ui->label_input->text();
    ui->label_input->setText(inputTxt+"2");
    if(point==1 || point==2){
        point = point +1;
    }else if(point==3){
        ui->label_input->setText(inputTxt);
    }
    if(inputTxt=="0"){
        ui->label_input->setText("0");
    }
    if(inputTxt.length()>6){
        ui->label_input->setText(inputTxt);
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    //按键--3
    if(keyFlag==1){
        return;
    }
    if(keyOtherFlag==1){
        return;
    }
    if(keyMenuFlag==1){
        return;
    }
    inputTxt=ui->label_input->text();
    ui->label_input->setText(inputTxt+"3");
    if(point==1 || point==2){
        point = point +1;
    }else if(point==3){
        ui->label_input->setText(inputTxt);
    }
    if(inputTxt=="0"){
        ui->label_input->setText("0");
    }
    if(inputTxt.length()>6){
        ui->label_input->setText(inputTxt);
    }
}

void MainWindow::on_pushButton_4_clicked()
{
    //按键--4
    if(keyFlag==1){
        return;
    }
    if(keyOtherFlag==1){
        return;
    }
    if(keyMenuFlag==1){
        return;
    }
    inputTxt=ui->label_input->text();
    ui->label_input->setText(inputTxt+"4");
    if(point==1 || point==2){
        point = point +1;
    }else if(point==3){
        ui->label_input->setText(inputTxt);
    }
    if(inputTxt=="0"){
        ui->label_input->setText("0");
    }
    if(inputTxt.length()>6){
        ui->label_input->setText(inputTxt);
    }
}

void MainWindow::on_pushButton_5_clicked()
{
    //按键--5
    if(keyFlag==1){
        return;
    }
    if(keyOtherFlag==1){
        return;
    }
    if(keyMenuFlag==1){
        return;
    }
    inputTxt=ui->label_input->text();
    ui->label_input->setText(inputTxt+"5");
    if(point==1 || point==2){
        point = point +1;
    }else if(point==3){
        ui->label_input->setText(inputTxt);
    }
    if(inputTxt=="0"){
        ui->label_input->setText("0");
    }
    if(inputTxt.length()>6){
        ui->label_input->setText(inputTxt);
    }
}

void MainWindow::on_pushButton_6_clicked()
{
    //按键--6
    if(keyFlag==1){
        return;
    }
    if(keyOtherFlag==1){
        return;
    }
    if(keyMenuFlag==1){
        return;
    }
    inputTxt=ui->label_input->text();
    ui->label_input->setText(inputTxt+"6");
    if(point==1 || point==2){
        point = point +1;
    }else if(point==3){
        ui->label_input->setText(inputTxt);
    }
    if(inputTxt=="0"){
        ui->label_input->setText("0");
    }
    if(inputTxt.length()>6){
        ui->label_input->setText(inputTxt);
    }
}

void MainWindow::on_pushButton_7_clicked()
{
    //按键--7
    if(keyFlag==1){
        return;
    }
    if(keyOtherFlag==1){
        return;
    }
    if(keyMenuFlag==1){
        return;
    }
    inputTxt=ui->label_input->text();
    ui->label_input->setText(inputTxt+"7");
    if(point==1 || point==2){
        point = point +1;
    }else if(point==3){
        ui->label_input->setText(inputTxt);
    }
    if(inputTxt=="0"){
        ui->label_input->setText("0");
    }
    if(inputTxt.length()>6){
        ui->label_input->setText(inputTxt);
    }
}

void MainWindow::on_pushButton_8_clicked()
{
    //按键--8
    if(keyFlag==1){
        return;
    }
    if(keyOtherFlag==1){
        return;
    }
    if(keyMenuFlag==1){
        return;
    }
    inputTxt=ui->label_input->text();
    ui->label_input->setText(inputTxt+"8");
    if(point==1 || point==2){
        point = point +1;
    }else if(point==3){
        ui->label_input->setText(inputTxt);
    }
    if(inputTxt=="0"){
        ui->label_input->setText("0");
    }
    if(inputTxt.length()>6){
        ui->label_input->setText(inputTxt);
    }
}

void MainWindow::on_pushButton_9_clicked()
{
    //按键--9
    if(keyFlag==1){
        return;
    }
    if(keyOtherFlag==1){
        return;
    }
    if(keyMenuFlag==1){
        return;
    }
    inputTxt=ui->label_input->text();
    ui->label_input->setText(inputTxt+"9");
    if(point==1 || point==2){
        point = point +1;
    }else if(point==3){
        ui->label_input->setText(inputTxt);
    }
    if(inputTxt=="0"){
        ui->label_input->setText("0");
    }
    if(inputTxt.length()>6){
        ui->label_input->setText(inputTxt);
    }
}

void MainWindow::on_pushButton_point_clicked()
{
    //按键--点(小数点)
    if(keyFlag==1){
        return;
    }
    if(keyOtherFlag==1){
        return;
    }
    if(keyMenuFlag==1){
        return;
    }
    inputTxt=ui->label_input->text();
    if(ui->label_input->text() != ""){
        ui->label_input->setText(inputTxt+".");
    }else{
        ui->label_input->setText("0.");
    }
    ui->label_input->setText(inputTxt+".");
    if(point==1 || point==2 || point==3){
        ui->label_input->setText(inputTxt);
    }else{
        point=1;
    }

    if(inputTxt.length()==0){
        ui->label_input->setText("");
    }else if(inputTxt.length()>6){
        ui->label_input->setText(inputTxt);
    }
}

void MainWindow::on_pushButton_0_clicked()
{
    //按键--0
    if(keyFlag==1){
        return;
    }
    if(keyOtherFlag==1){
        return;
    }
    if(keyMenuFlag==1){
        return;
    }
    inputTxt=ui->label_input->text();
    ui->label_input->setText(inputTxt+"0");
    if(point==1 || point==2){
        point = point +1;
    }else if(point==3){
        ui->label_input->setText(inputTxt);
    }
    if(inputTxt=="0"){
        ui->label_input->setText("0");
    }

    if(inputTxt.length()>6){
        ui->label_input->setText(inputTxt);
    }
}

void MainWindow::on_pushButton_back_clicked()
{
    //按键--返回
    keyboardFlag=0;
    ui->groupBox_keyboardBg->hide();
    ui->pushButton_keyboard->show();


    if(attrib1==1){
        //ui->groupBox_pannel->hide();
        ui->groupBox_totalCashierBg->show();
        ui->groupBox_totalPannelBg->hide();
        ui->pushButton_gotoFlowlist->show();
    }else if(attrib1==7){
        ui->pushButton_gotoFlowlist->hide();
    }else{
        ui->label_consumeLimitMoney->show();
        ui->label_timeSegName->show();
        ui->pushButton_gotoFlowlist->show();
        ui->groupBox_pannel->show();
        ui->groupBox_totalCashierBg->hide();
        ui->groupBox_totalPannelBg->show();
        if(attrib1==6){
            ui->label_totalFrequency_title->hide();
            ui->label_totalFrequency->hide();
        }
    }
}

void MainWindow::on_pushButton_delete_clicked()
{
    //按键--删除
    if(keyFlag==1){
        return;
    }
    if(inputTxt==""){
       ui->label_sum->setText("");
    }
    inputTxt=ui->label_input->text();
    inputTxt = inputTxt.left(inputTxt.length()-1);
    ui->label_input->setText(inputTxt);
    point =point -1;
    if(point==0){
        point=0;
    }
}

void MainWindow::on_pushButton_clear_clicked()
{
    //按键--清空
    if(keyFlag==1){
        return;
    }
    ui->label_input->setText("");
    ui->label_sum->setText("");
    point=0;
}
void MainWindow::on_pushButton_menu_clicked()
{
    //按键--菜单
    //跳转至登录
    if(consumeState==1){
        cancelConsume();
    }
    if(attrib1==7){
        cancelAccoInfo();
    }
    ui->label_input->setText("");
    keyFlag=1;
    login * myLogin  = new login(this);
    myLogin->scmd_network_login=scmd_network;
    myLogin->scmd_main_login=scmd_main;
    myLogin->scmd_sec_login=scmd_sec;
    myLogin->termCode_login=termCode;
    //版本号
    myLogin->uiVersion_login=uiVersion;
    myLogin->players_login=players;

    myLogin->consumeserVersion_login=consumeserVersion;
    myLogin->scanqrserVersion_login=scanqrserVersion;
    myLogin->secscreenVersion_login=secscreenVersion;
    myLogin->networkVersion_login=networkVersion;

    myLogin->exec();
    if(consumeState==1){
       ui->label_input->setText(consumeMoney);
       //reqConsume();
       //脱机新建
       if(rets==10){
           reqConsume(12);
       }else{
           offLineConsume();
       }
    }
    if(attrib1==7){
        reqAccoInfo();
    }
    keyFlag=0;
    delete myLogin;
}
void MainWindow::on_pushButton_submits_clicked()
{
    //按键--确定
    if(keyFlag==1){
        return;
    }
    if(attrib1==7){
        return;
    }
    myPtf("on_pushButton_submits_clicked\n");
    ui->label_cardNumber->setText("");
    ui->label_manageFee->setText("");

    ui->label_cardNumber->show();
    ui->label_manageFee->show();
    ui->label_noteMes->show();
    ui->label_noteMes->setText("");
    point=0;
    myPtf("attrib1qq%d\n",attrib1);
    if(attrib1==1){
        //出纳
        if(accoInfoFlag==1){
            keyMenuFlag=0;
            if(inputReg()==1)
            {
                myPtf("chuna  start reqConsume-----------------\n");
                reqConsume(1);
            }
        }else{
            ui->label_noteMes->setText("请先卡自检获取信息");
            ui->label_input->setText("");
            QTimer::singleShot(2000,ui->label_noteMes,SLOT(hide()));
        }
    }else{
        totalstr = totalstr + ui->label_input->text();
        if(attrib1==2 || attrib1==3 || attrib1==6){
            //消费
            if(inputReg()==1)
            {
                ui->label_sum->setText(totalstr);
                myPtf("putongxiaofei  start reqConsume-----------------\n");
                //reqConsume();
                //脱机新建
                if(rets==10){
                    reqConsume(2);
                }else{
                    offLineConsume();
                }
            }else{
                myPtf("aaaaaaaaaaaaaa\n");
                totalstr="";
                ui->label_input->setText("");
                memset(totalnum,0, sizeof(totalnum));
                arrnum=0;//数组下标
                ui->label_sum->setText("");
            }
        }

    }
    if(rets!=10){
        ui->label_sum->setText("");
    }
}
void MainWindow::on_pushButton_add_clicked()
{
    if(keyFlag==1){
        return;
    }
    inputTxt=ui->label_input->text();
    if(attrib1!=1){
        //做"+"号,累加金额
        point=0;
        addFlag=1;
        //临时变量
        double tm = 0;
        //累加
        if(totalnum[0]==0){
            totalnum[arrnum] = inputTxt.toDouble();
            tm = inputTxt.toDouble();
        }else{
            totalnum[arrnum] = inputTxt.toDouble();
            for(int i=0;i<100;i++){
                if(totalnum[i]>0){
                    tm = tm + totalnum[i];
                }
            }
        }
        arrnum = arrnum+1;
        ui->label_input->setText("");
        totalstr="";
        for(int i=0;i<100;i++){

            if(totalnum[i]>0){
                totalstr += QString::number(totalnum[i])+"+";
            }

        }

        if(totalstr.length()>0){

            totalstr.left(totalstr.length()-1);
        }

        if(attrib1==2 || attrib1==3 || attrib1==6){
            ui->label_sum->setText(totalstr);
        }
    }
}

void MainWindow::reqConsume(int reqNum)
{
    myPtf("reqNum:%d\n",reqNum);
    //请求消费
    //组织数据
    int cct=getCurrentTimeMain();
    myPtf("reqConsume1 program:%d %02d:%02d:%02d--%d\n",cct,cct/10000/60/60+8,cct/10000/60%60,cct/10000%60,cct%10000);


    QJsonObject jsonsend;
    jsonsend.insert("pwd",pwd);//以后需增加密码键盘
    QDateTime time = QDateTime::currentDateTime();
    timeTcur = time.toTime_t();//当前时间戳
    myPtf("timeTcur%d\n",timeTcur);
    jsonsend.insert("timeStamp",QString::number(timeTcur));
    jsonsend.insert("transactionType",consumeType);
    if(attrib1==1){
        moneyFloat =ui->label_input->text().toDouble();
        money = (int)(moneyFloat * 100 +0.01);
        jsonsend.insert("money",QString::number(money));
        jsonsend.insert("type",cashier);//type:0—消费，1—存款2—取款，3—洗衣
        jsonsend.insert("cardId",cardId);
    }else if(attrib1==2 || attrib1==3){
        if(consumeState==1){//定额消费
            moneyFloat=consumeMoney.toDouble();
        }else{//普通消费
            if(addFlag==1){
                moneyFloat=0;
                for(int i=0;i<100;i++){
                    if(totalnum[i]>0){
                        moneyFloat = moneyFloat + totalnum[i];
                    }
                }
                if(moneyFloat>0){
                    moneyFloat=moneyFloat+ui->label_input->text().toDouble();
                }
            }else{
                moneyFloat=ui->label_input->text().toDouble();
            }


            if(moneyFloat > ui->label_note_consumeLimitMoney->text().toDouble()){
                ui->label_tips->show();
                ui->label_tips->setText("请输入小于"+ui->label_note_consumeLimitMoney->text()+"的金额!");
                QTimer::singleShot(2000,ui->label_tips,SLOT(hide()));
                ui->label_input->setText("");
                //将累加金额，清空还原
                memset(totalnum,0, sizeof(totalnum));
                arrnum=0;//数组下标
                totalstr="";
                ui->label_sum->setText("");
                addFlag=0;
                return;
            }
        }


        money = (int)(moneyFloat * 100 +0.01);
        sprintf(moneySum,"%.2f",((double)money)/100);
        myPtf("addFlag-------------------:%d\n",addFlag);
        if(addFlag==1){
            ui->label_input->setText(moneySum);
        }
        jsonsend.insert("money",QString::number(money));
        jsonsend.insert("type","0");//type:0—消费，1—存款2—取款，3—洗衣
        myPtf("wwCardId:%d\n",cardId);
        jsonsend.insert("cardId",cardId);
    }else if(attrib1==6){
        money = ui->label_input->text().toInt();
        if(addFlag==1){
            money=0;
            for(int i=0;i<100;i++){
                if(totalnum[i]>0){
                    money = money + totalnum[i];
                }
            }
            if(money>0){
                money=money+ui->label_input->text().toInt();
            }
            ui->label_input->setText(QString::number(money));
        }else{
            money=ui->label_input->text().toInt();
        }
        jsonsend.insert("type","3");//type:0—消费，1—存款2—取款，3—洗衣
        jsonsend.insert("money",QString::number(money));
        jsonsend.insert("cardId",0);
    }

    jsonsend.insert("termCode",termCode);
    jsonsend.insert("termID",termID);
    jsonsend.insert("areaID",areaID);


    //发送数据
    cct=getCurrentTimeMain();
    myPtf("reqConsume2 program:%d %02d:%02d:%02d--%d\n",cct,cct/10000/60/60+8,cct/10000/60%60,cct/10000%60,cct%10000);


    QJsonObject jsonrecv;
    int Cmd = UI_REQ_CONSUME;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_main->send_Command(&Cmd,jsonsend,retJson);

    cct=getCurrentTimeMain();
    myPtf("reqConsume3 program:%d %02d:%02d:%02d--%d\n",cct,cct/10000/60/60+8,cct/10000/60%60,cct/10000%60,cct%10000);

    if(Cmd != 10000+UI_REQ_CONSUME){

        //数据发送失败，可以尝试重连
        ui->label_tips->show();
        //ui->label_tips->setText("发送请求消费命令失败!,ret="+QString::number(Cmd));
        ui->label_tips->setText("消费服务断开，请重新连接服务！");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
        myPtf("recv UI_REQ_CONSUME ret error!\n");
    }else if(Cmd == CLOUD_SENDCMD_ERROR){
        serverError();
    }else{
        cct=getCurrentTimeMain();
        myPtf("reqConsume3.1 program:%d %02d:%02d:%02d--%d\n",cct,cct/10000/60/60+8,cct/10000/60%60,cct/10000%60,cct%10000);

        myPtf("recv UI_REQ_CONSUME ret ok!\n");
        ui->label_noteMes->show();
        //解析json字符串

        if(jsonrecv.value("state").toInt()==1){
            cct=getCurrentTimeMain();
            myPtf("reqConsume3.2 program:%d %02d:%02d:%02d--%d\n",cct,cct/10000/60/60+8,cct/10000/60%60,cct/10000%60,cct%10000);

            dateStamp= jsonrecv.value("timeStamp").toString();//返回时间戳
            if(dateStamp.toInt()!=timeTcur)
            {
                ui->label_noteMes->setText("返回时间戳错误!");
                QTimer::singleShot(3000,ui->label_noteMes,SLOT(hide()));
                myPtf("dateStamp error! timeTcur=%d\n",timeTcur);
            }
            else
            {
                cct=getCurrentTimeMain();
                myPtf("reqConsume3.3 program:%d %02d:%02d:%02d--%d\n",cct,cct/10000/60/60+8,cct/10000/60%60,cct/10000%60,cct%10000);
                if(attrib1 !=1){
                    if(pwdFlag==0){
                       showConsumeReady();
                       keyOtherFlag=1;//wang maotan 20190614
                       QTimer::singleShot(100,this,SLOT(consumePlayer()));
                    }
                }
                cct=getCurrentTimeMain();
                myPtf("reqConsume3.4 program:%d %02d:%02d:%02d--%d\n",cct,cct/10000/60/60+8,cct/10000/60%60,cct/10000%60,cct%10000);

                myPtf("start to queConsumeRet\n");
                //timerSearch->start(1000);//启动查询消费
                addFlag=0;
                keyOtherFlag=1;
            }
        }else{
            ui->label_noteMes->setText("接收失败");
            QTimer::singleShot(3000,ui->label_noteMes,SLOT(hide()));
        }
    }
    cct=getCurrentTimeMain();
    myPtf("reqConsume4 program:%d %02d:%02d:%02d--%d\n",cct,cct/10000/60/60+8,cct/10000/60%60,cct/10000%60,cct%10000);
}

void MainWindow::showConsumeReady()
{
    if(consumeState !=1){
        myPtf("qqq-showConsumeReady\n");
        ui->groupBox_consumeMesBg->hide();
        ui->groupBox_mask->show();
        ui->pushButton_cancelConsume->show();
        //读卡--扫码
        if(consumeType==1){
            //请读卡
            ui->label_erroNote->setText("请读卡!");
            showAll("请读卡!",130,200,40);
            //player("/usr/local/nkty/players/video13.wav");//请读卡
        }else if(consumeType==2){
            //请扫码
            ui->label_erroNote->setText("请扫码!");
            showAll("请扫码!",130,200,40);
            //player("/usr/local/nkty/players/video14.wav");//请扫码
        }else{
            //请读卡或扫码
            ui->label_erroNote->setText("请读卡或扫码!");
            showAll("请读卡或扫码!",130,200,40);
            //player("/usr/local/nkty/players/video15.wav");//请读卡或扫码
        }
    }

    if(attrib1==2 || attrib1==3){
        if(consumeState==1){
            showPic();
        }
        QDateTime time = QDateTime::currentDateTime();
        myPtf("time----------pay money start! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());

        showAll("需支付:"+QString::number(moneyFloat)+"元",140,200,40);

        time = QDateTime::currentDateTime();
        myPtf("time----------pay money end! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
    }else if(attrib1==6){
        showAll("需支付:"+QString::number(money)+"张",140,200,40);
    }
}
void MainWindow::consumePlayer()
{
    if(consumeState !=1){
        //读卡--扫码
        if(consumeType==1){
            //请读卡
            player("/usr/local/nkty/players/video13.wav");//请读卡
        }else if(consumeType==2){
            //请扫码
            player("/usr/local/nkty/players/video14.wav");//请扫码
        }else{
            //请读卡或扫码
            player("/usr/local/nkty/players/video15.wav");//请读卡或扫码
        }
    }
}
void MainWindow::showConsumeMsg(QString noteMes)
{
    int  currentTime=getCurrentTimeMain();
    myPtf("qqq---start maskBg! %d\n",currentTime);

    ui->groupBox_mask->hide();//信息提示的遮罩隐藏

    currentTime=getCurrentTimeMain();
    myPtf("qqq---end maskBg! %d\n",currentTime);

    clearFlag=1;
    pwd="";//密码清空
    pwdFlag=0;
    loginFlag=0;
    cardId=0;
    //交易成功后,显示消费信息界面
    std::string str = noteMes.toStdString();
    const char* ch = str.c_str();
    currentTime=getCurrentTimeMain();
    myPtf("qqq---start recoveryPannel! %d\n",currentTime);
    QDateTime time = QDateTime::currentDateTime();
    myPtf("recoveryPannel %s start! %02d:%02d:%02d\n",ch,time.time().hour(), time.time().minute(), time.time().second());
    //恢复初始界面

    if(keyboardFlag==1){
        ui->groupBox_keyboardBg->show();
        ui->pushButton_gotoFlowlist->hide();//右侧查看流水按钮隐藏
        ui->pushButton_keyboard->hide();//右侧软键盘按钮隐藏
        ui->label_consumeLimitMoney->hide();
        ui->label_timeSegName->hide();
        ui->groupBox_totalPannelBg->hide();
    }else{
        ui->groupBox_keyboardBg->hide();
        ui->pushButton_gotoFlowlist->show();//右侧查看流水按钮显示
        ui->pushButton_keyboard->show();//右侧软键盘按钮显示
        if(attrib1==1){
            ui->label_consumeLimitMoney->hide();
            ui->label_timeSegName->hide();
            ui->groupBox_totalPannelBg->hide();
            ui->groupBox_totalCashierBg->show();
        }else if(attrib1==2 || attrib1==3){
            ui->label_consumeLimitMoney->show();
            ui->label_timeSegName->show();
            ui->groupBox_totalPannelBg->show();
            ui->groupBox_totalCashierBg->hide();
        }else if(attrib1==6){
            ui->label_consumeLimitMoney->hide();
            ui->label_timeSegName->hide();
            ui->groupBox_totalCashierBg->hide();
            ui->groupBox_totalPannelBg->show();
            ui->label_totalFrequency_title->hide();
            ui->label_totalFrequency->hide();
        }
    }

    ui->label_noteMes->setText(noteMes);
    ui->groupBox_consumeMesBg->show();
    //QApplication::processEvents();
    //副屏提示
    currentTime=getCurrentTimeMain();
    myPtf("qqq---showAll! %d\n",currentTime);
    time = QDateTime::currentDateTime();
    myPtf("showAll! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());

    showAll(noteMes,150,200,40);

    currentTime=getCurrentTimeMain();
    myPtf("qqq---recoveryPannel finish! %d\n",currentTime);
    time = QDateTime::currentDateTime();

    myPtf("recoveryPannel %s finish! %02d:%02d:%02d\n",ch,time.time().hour(), time.time().minute(), time.time().second());    
}
void MainWindow::statePannel(QString noteTip)
{
    myPtf("23576\n");
    //查询消费结果，根据相应状态,显示不同提示界面
    //主屏提示
    ui->groupBox_mask->show();
    ui->label_erroNote->setText(noteTip);
    ui->pushButton_cancelConsume->hide();//取消交易隐藏
    //ui->label_noteMes->show();
    //ui->label_noteMes->setText(noteTip);
    //QTimer::singleShot(1000,ui->label_noteMes,SLOT(hide()));
    //ui->label_noteMes->setText("");
    ui->label_input->setText("");
    ui->groupBox_consumeMesBg->hide();//左侧消费信息隐藏

    QTimer::singleShot(1000,ui->groupBox_mask,SLOT(hide()));
    //副屏提示
    showAll(noteTip,80,200,40);
    keyFlag=0;
    loginFlag=0;
    cardId=0;
}
void MainWindow::hideConsumeMsg()
{
    myPtf("qqwwee\n");
    //隐藏消费面板--恢复初始页面
    clearFlag=0;
    keyFlag=0;

    ui->groupBox_consumeMesBg->hide();
    ui->label_cardNumber->setText("");
    ui->label_consumeTimes->setText("");
    ui->label_remainMoney->setText("");
    ui->label_secAccoMoney->setText("");
    ui->label_manageFee->setText("");
    ui->label_noteMes->hide();
    ui->label_noteMes->setText("");
    if(consumeSuccessFlag==1){
        ui->groupBox_mask->hide();//取消交易遮罩隐藏
    }


    if(consumeSuccessFlag==1){
        ui->label_input->setText("");//输入框清空
        consumeSuccessFlag=0;
    }else{
        QTimer::singleShot(500,ui->pushButton_cancelConsume,SLOT(show()));//恢复取消按钮显示
        //ui->pushButton_cancelConsume->show();
    }
    if(attrib1==1){
        //ui->groupBox_pannel->hide();//右侧出纳信息隐藏
        ui->label_remainMoneys->setText("");
        cashierFlag=0;
        cashier="1";
        ui->label_workType->setText("存款");
        hideAccoInfoMsg();
    }
    if(consumeState==1){
        ui->label_input->setText(consumeMoney);
    }
    //在消费模式下(正常)，账户不存在、账户挂失或冻结、账户余额不足时再消费问题
    //if(ret>0 && ret!=10)
    if(ret>10 || ret <0)
    {
        myPtf("erro  ret<0 returnConsume start----------------------------------");
        reqConsume(4);
        //QTimer::singleShot(2000,this,SLOT(reqConsume(4)));
        myPtf("erro  ret<0 returnConsume end----------------------------------");
    }
}
void MainWindow::errorPannel(int ret,QString noteMes,QString videoUrl)
{
    keyOtherFlag=1;//禁用键盘 wang maotan 20190614
    //消费中错误信息提示界面
    ui->groupBox_mask->show();//遮罩错误信息显示
    ui->pushButton_cancelConsume->hide();//取消交易隐藏

    ui->label_noteMes->hide();
    ui->label_noteMes->setText("");

    //主屏提示
    ui->label_erroNote->setText(noteMes);
    player(videoUrl);//音频
    if(ret>=0){
        //ui->label_input->setText("");//输入框清空
        if(attrib1==1 || attrib1==7){
            QTimer::singleShot(2000,ui->groupBox_mask,SLOT(hide()));
        }
    }else{
        if(attrib1==1 || attrib1==7){
            QTimer::singleShot(2000,ui->groupBox_mask,SLOT(hide()));
        }
        keyFlag=1;
    }

    //副屏提示
    showAll(noteMes,50,200,40);

    loginFlag=0;
    cardId=0;
    pwdFlag=0;
    msg="";
    pwd="";
    //keyOtherFlag=0;//恢复键盘可点击状态--消费 wang mao tan 20190614

    if(consumeState==1){//固定消费 0启用。1不启用
        ui->label_input->setText(consumeMoney);
    }
}
void MainWindow::passwordPannel()
{
    //消费时，如果需要输入密码，弹出该对话框的函数
    pwdFlag=1;
    ui->groupBox_mask->show();//遮罩错误信息显示
    ui->groupBox_pwdBg->show();//消费时，如果需要输入密码，弹出该对话框显示
    ui->pushButton_cancelConsume->hide();//取消交易隐藏
    loginFlag=2;
    //主屏提示
    ui->label_erroNote->setText("需要输入密码");
    if(consumeState==1){
        keyFlag=0;//让终端键盘在定额消费模式时可点击，输入密码
    }
    player("/usr/local/nkty/players/video7.wav");//音频
}
void MainWindow::showConsumeResult(int result)
{
    //得到查询结果后显示
    int currentTime=getCurrentTimeMain();
    myPtf("qqq---start showConsumeResult! %d\n",currentTime);
    QDateTime time = QDateTime::currentDateTime();
    myPtf("start showConsumeResult! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
    switch(result)
    {
    case CONSUME_SUCCESS:
        currentTime=getCurrentTimeMain();
        myPtf("qqq---start showConsumeResult 1! %d\n",currentTime);
        myPtf("start showConsumeResult 1! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
        showConsumeMsg("交易成功");        
        consumeSuccessFlag=1;
        currentTime=getCurrentTimeMain();
        myPtf("qqq---newtime ! %d\n",currentTime);


        if(attrib1==1){
            //主屏显示
            //个人账户余额

            ui->label_remainMoneys->setText(remainMoney2);
            //补贴账户余额
            //ui->label_secAccoMoney->setText(secAccoMoney);
            //第几次消费
            //ui->label_consumeTimes->setText(QString::number(consumeTimes));
            //消费金额
            ui->label_note_flowMoney->setText(flowMoney2);
            //附加费
            //ui->label_manageFee->setText(manageFee);
            if(cashier=="1"){
                //当日充值总金额
                totalInCount = (ui->label_totalInCount->text().toDouble()) + (ui->label_note_flowMoney->text().toDouble()) ;
                char totalInCounts[20];
                sprintf(totalInCounts,"%.2f",(totalInCount));
                ui->label_totalInCount->setText(totalInCounts);
            }else{
                //当日取钱总金额
                totalOutCount = (ui->label_totalOutCount->text().toDouble()) + (ui->label_note_flowMoney->text().toDouble());
                char totalOutCounts[20];
                sprintf(totalOutCounts,"%.2f",(totalOutCount));
                ui->label_totalOutCount->setText(totalOutCounts);
            }
            accoInfoFlag=0;
        }else if(attrib1==2 || attrib1==3){
            //主屏显示
            //个人账户余额
            ui->label_remainMoney->setText(remainMoney2);
            //补贴账户余额
            ui->label_secAccoMoney->setText(secAccoMoney);
            //第几次消费
            ui->label_consumeTimes->setText(QString::number(consumeTimes));
            //消费金额
            ui->label_note_flowMoney->setText(flowMoney2);
            //附加费
            ui->label_manageFee->setText(manageFee);
            //当日消费总金额要将当前这笔消费金额进行累加
            totalMoney = (ui->label_totalMoney->text().toDouble()) + (ui->label_note_flowMoney->text().toDouble()) + (ui->label_manageFee->text().toDouble());
            char totalMoneys[20];
            sprintf(totalMoneys,"%.2f",(totalMoney));
            ui->label_totalMoney->setText(totalMoneys);
            //本餐次刷卡次数要将当前刷卡累加1
            int totalFrequency = ui->label_totalFrequency->text().toInt();
            totalFrequency=totalFrequency+1;
            ui->label_totalFrequency->setText(QString::number(totalFrequency));

        }else if(attrib1==6){
            //主屏显示
            ui->label_remainMoney->setText(QString::number(remainMoney6));
            ui->label_note_flowMoney->setText(QString::number(flowMoney6));
            //当日消费总券数要将当前这笔消费券数进行累加
            totalCount = ui->label_totalMoney->text().toInt() + flowMoney6;
            ui->label_totalMoney->setText(QString::number(totalCount));
        }
        ui->label_cardNumber->setText(QString::number(accountID));

        QApplication::processEvents();

        currentTime=getCurrentTimeMain();
        myPtf("qqq---start showConsumeResult 2! %d\n",currentTime);
        time = QDateTime::currentDateTime();
        myPtf("start showConsumeResult 2! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());

        //左侧--右侧账户消费信息定时2s后隐藏
        currentTime=getCurrentTimeMain();
        myPtf("qqq---start showConsumeResult 3! %d\n",currentTime);
        time = QDateTime::currentDateTime();
        myPtf("start showConsumeResult 3! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());

        //交易成功后，副屏显示信息
        //showScanqrser();
        QTimer::singleShot(500,this,SLOT(showScanqrser()));

        //显示声音
        currentTime=getCurrentTimeMain();
        myPtf("qqq---start player! %d\n",currentTime);

        player("/usr/local/nkty/players/video18.wav");//音频

        currentTime=getCurrentTimeMain();
        myPtf("qqq---end player! %d\n",currentTime);
        clearScreen(1300,FSTSCREEN);        
        clearScreen(1300,SECSCREEN);
        keyOtherFlag=0;//放开键盘 wang mao tan 20190614
        break;
    case CONSUME_FROZEN_LOSS:
        errorPannel(CONSUME_FROZEN_LOSS,"该账户已挂失或已冻结","/usr/local/nkty/players/video23.wav");
        clearScreen(1300,SECSCREEN);
        break;
    case CONSUME_ERROR:
        errorPannel(CONSUME_ERROR,"消费失败","/usr/local/nkty/players/video4.wav");
        clearScreen(1300,SECSCREEN);//副屏ID
        break;
    case CONSUME_NOT_FUNDS:
        //主屏显示
        ui->groupBox_mask->show();//遮罩错误信息显示
        ui->pushButton_cancelConsume->hide();//取消交易隐藏
        //ui->label_input->setText("");//输入框清空
        ui->label_noteMes->hide();
        ui->label_noteMes->setText("");
        keyFlag=1;
        if(attrib1==1 || attrib1==2 || attrib1==3){
            //个人账户余额
            ui->label_note_remainMoney->setText(remainMoney2);
            //补贴账户余额
            ui->label_note_secAccoMoney->setText(secAccoMoney);
            //附加费
            ui->label_note_manageFee->setText(manageFee);

            ui->label_erroNote->setText("余额不足,个人账户余额:"+ui->label_note_remainMoney->text()+"元,\n补贴账户余额:"+ui->label_note_secAccoMoney->text()+"元,\n应收附加费"+ui->label_note_manageFee->text()+"元,\n第"+QString::number(consumeTimes)+"次消费");
        }else if(attrib1==6){
             //个人账户余额
            ui->label_erroNote->setText("余额不足,个人账户余额:"+QString::number(remainMoney6)+"张");

        }
        //副屏显示
        if(attrib1==1 || attrib1==2 || attrib1==3){
            showAll("余额不足",150,90,40);
            appendtxt("个人账户余额:"+ui->label_note_remainMoney->text()+"元",20,160,35);
            appendtxt("补贴账户余额:"+ui->label_note_secAccoMoney->text()+"元",20,230,35);
            appendtxt("应收附加费:"+ui->label_note_manageFee->text()+"元",20,300,35);
        }else if(attrib1==6){
            showAll("余额不足",150,160,35);
            appendtxt("个人账户余额:"+QString::number(remainMoney6)+"张",20,230,35);
        }
        //还原初始值
        loginFlag=0;
        cardId=0;
        pwdFlag=0;
        msg="";
        pwd="";
        if(consumeState==1){
            ui->label_input->setText(consumeMoney);
        }
        if(attrib1==1){
            recoveryAccoInfo();
            accoInfoFlag=0;
        }
        player("/usr/local/nkty/players/video5.wav");//音频
        //errorPannel(CONSUME_NOT_FUNDS,"余额不足","/usr/local/nkty/players/video5.wav");
        clearScreen(1300,FSTSCREEN);
        clearScreen(1300,SECSCREEN);
        //keyOtherFlag=0;//放开键盘 wang mao tan 20190614
        break;
    case CONSUME_NOT_ACCOUNTS:
        errorPannel(CONSUME_NOT_ACCOUNTS,"账户不存在或已注销","/usr/local/nkty/players/video6.wav");
        clearScreen(1300,SECSCREEN);
        break;
    case CONSUME_NEED_PASSWORD:
        passwordPannel();//消费时，如果需要输入密码，弹出该对话框的函数
        break;
    case QR_DECODE_ERROR:
        errorPannel(QR_DECODE_ERROR,"二维码解码失败","/usr/local/nkty/players/video8.wav");
        clearScreen(1300,SECSCREEN);
        break;
    case QR_DECODE_MERCHANTID:
        errorPannel(QR_DECODE_MERCHANTID,"商户号不一致","");
        clearScreen(1300,SECSCREEN);
        break;
    case QR_DECODE_TIMEOUT:
        errorPannel(QR_DECODE_TIMEOUT,"请求超时","/usr/local/nkty/players/video9.wav");
        clearScreen(1300,SECSCREEN);
        break;
    case QR_DECODE_FACTORYERR:
        errorPannel(QR_DECODE_FACTORYERR,"厂商识别号错误","/usr/local/nkty/players/video10.wav");
        clearScreen(1300,SECSCREEN);
        break;
    case NO_CONSUME_REQ:
        errorPannel(NO_CONSUME_REQ,"消费错误","/usr/local/nkty/players/video19.wav");
        clearScreen(1300,SECSCREEN);
        break;
	case 2:
		keyOtherFlag = 0;	//放开键盘 // bywei
		//查询失败
		if(offlineFlag=1)
		{
			statePannel("账户不存在");//查询消费结果，根据相应状态,显示不同提示界面
			clearScreen(1300,SECSCREEN);//副屏ID，清空副屏 wang mao tan 20190614
			clearScreen(1300,FSTSCREEN);	// bywei
		}
		else
		{
			statePannel("查询失败");//查询消费结果，根据相应状态,显示不同提示界面
			clearScreen(1300,SECSCREEN);//副屏ID，清空副屏 wang mao tan 20190614
			myPtf("1234546\n");
		}
		break;
    case 3:
        //流水号失效
        statePannel("流水号失效");//查询消费结果，根据相应状态,显示不同提示界面
        clearScreen(1300,SECSCREEN);//副屏ID，清空副屏 wang mao tan 20190614
        break;
    case 4:
        //结算中...
        ui->label_erroNote->setText("结算中...");
        ui->groupBox_mask->show();
        QApplication::processEvents();
        showAll("结算中...",80,200,40);
        //statePannel("结算中...");//查询消费结果，根据相应状态,显示不同提示界面
        player("/usr/local/nkty/players/video19.wav");//音频
        break;
    default:
        break;
    }
    if(consumeState==1){
        myPtf("chongxinqingqiuxiaofei  start returnConsume----------------------");
        //定额消费时，如果需要输入密码，则停留在输入密码框界面，其他情况，则再次发起消费请求
        if(pwdFlag==0){
            QTimer::singleShot(3000,this,SLOT(returnConsume()));
        }
    }
    //keyOtherFlag=0;wang mao tan 20190614
    timeTcur=0;//返回结果后，将当前时间戳归0

    if(attrib1==1){
        keyMenuFlag=1;
    }else if(attrib1==2 || attrib1==3 || attrib1==6){
        //将累加金额，清空还原
        ui->label_sum->setText("");
        memset(totalnum,0, sizeof(totalnum));
        arrnum=0;//数组下标
        totalstr="";
    }
    currentTime=getCurrentTimeMain();
    myPtf("qqq---start showConsumeResult 5! %d\n",currentTime);
    time = QDateTime::currentDateTime();
    myPtf("start showConsumeResult 5! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
    myPtf("finish showConsumeResult!\n");
}
void MainWindow::showScanqrser()
{
    QDateTime time = QDateTime::currentDateTime();
    //int currentTime=getCurrentTimeMain();
    //交易成功后，副屏显示信息
    if(rets==10){
        if(attrib1==1){
            //副屏显示
            showAll("出纳金额:"+ui->label_note_flowMoney->text()+"元",20,160,35);
            //appendtxt("附加费:"+ui->label_manageFee->text()+"元",20,170,30);
            appendtxt("个人账户余额:"+ui->label_remainMoneys->text()+"元",20,230,35);
            //appendtxt("补贴账户余额:"+ui->label_secAccoMoney->text()+"元",20,270,30);

            int currentTimes=getCurrentTimeMain();
            myPtf("qqq---start showConsumeResult 4! %d\n",currentTimes);
            time = QDateTime::currentDateTime();
            myPtf("start showConsumeResult 4! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
        }else if(attrib1==2 || attrib1==3){
            //副屏显示
            showAll("消费金额:"+ui->label_note_flowMoney->text()+"元",20,90,35);
            appendtxt("附加费:"+ui->label_manageFee->text()+"元",20,160,35);
            appendtxt("个人账户余额:"+ui->label_remainMoney->text()+"元",20,230,35);
            appendtxt("补贴账户余额:"+ui->label_secAccoMoney->text()+"元",20,300,35);
            time = QDateTime::currentDateTime();
            myPtf("start showConsumeResult 4! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
        }else if(attrib1==6){
            //副屏显示
            showAll("消费金额:"+ui->label_note_flowMoney->text()+"张",20,160,35);
            appendtxt("个人账户余额:"+ui->label_remainMoney->text()+"张",20,230,35);
            time = QDateTime::currentDateTime();
            myPtf("start showConsumeResult 4! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
        }
    }else{
        showAll("消费金额:"+ui->label_note_flowMoney->text()+"元",20,200,35);
    }

}
void MainWindow::returnConsume()
{
    ui->groupBox_mask->hide();
    myPtf("returnConsume start----------------------------------");
    //reqConsume();
    //脱机新建
    if(rets==10){
        reqConsume(3);
    }else{
        offLineConsume();
    }
}
void MainWindow::queConsumeRet(QJsonObject jsonrecv)
{

    //查询消费结果
    //开始查询的时间戳
    QDateTime time = QDateTime::currentDateTime();
    myPtf("time----------start to queConsumeRet! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
    //已收到返回，查看当前进度状态
    int state=jsonrecv.value("state").toInt();

    //如果已经处理完毕返回结果，应解析最终结果
    switch(state)
    {
    //已完成，可以看最后结果，本次操作结束（特殊情况下可能衔接发起下一轮请求）
    case 1:
        if(rets==10){
            ret = jsonrecv.value("ret").toInt();
            //返回的数据
            //返回数据的时间戳(开始)
            time = QDateTime::currentDateTime();
            myPtf("time----------resultStart! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
            if(attrib1==1 || attrib1==2 || attrib1==3)//出纳消费
            {
                sprintf(remainMoney2,"%.2f",((float)jsonrecv.value("remainMoney").toInt())/100);
                sprintf(flowMoney2,"%.2f",((float)jsonrecv.value("flowMoney").toInt())/100);
            }else if(attrib1==6)//洗衣
            {
                remainMoney6=jsonrecv.value("remainMoney").toInt();
                flowMoney6=jsonrecv.value("flowMoney").toInt();
            }
            sprintf(secAccoMoney,"%.2f",((float)jsonrecv.value("secAccoMoney").toInt())/100);
            sprintf(manageFee,"%.2f",((float)jsonrecv.value("manageFee").toInt())/100);
            consumeTimes=jsonrecv.value("consumeTimes").toInt();
            accountID =jsonrecv.value("accountID").toInt();
            cardId =jsonrecv.value("cardID").toInt();
            myPtf("qqCardId:%d\n",cardId);
            msg=jsonrecv.value("msg").toString();

            //返回数据的时间戳(结束)
            time = QDateTime::currentDateTime();
            myPtf("time----------resultEnd! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());

            myPtf("showConsumeResult ret=%d\n",ret);
            showConsumeResult(ret);

            //如果是解码失败、云服务失败等原因，应在报错等待后重新发起下一轮消费请求

        }else{
            showOfflineConsumeMsg("交易成功！");

            clearScreen(1300,FSTSCREEN);
            clearScreen(1300,SECSCREEN);
        }

        break;
    //各种错误，一般不可能发生，可在显示错误后结束本次操作
    case 2:
    case 3:
    case 4:
        //显示失败信息，结束
        myPtf("showConsumeResult state=%d\n",state);
        showConsumeResult(state);
        break;
    case 5:
        //不显示消息，重新发起请求
        myPtf("continue to queConsumeRet\n");
        //timerSearch->start(200);
        break;
    default:
        break;
    }
}
void MainWindow::cancelConsume()
{
    //取消消费
    keyOtherFlag=0;
    QJsonObject jsonsend;
    jsonsend.insert("timeStamp",dateStamp);
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = UI_CANCEL_CONSUME;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_main->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+UI_CANCEL_CONSUME){
        //数据获取失败，可以尝试重连
        /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(Cmd == CLOUD_SENDCMD_ERROR){
        serverError();
    }else{
        /*QString strShow = "";
        strShow.append(retJson);
        QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        message.exec();*/
        //解析json字符串
        //ui->pushButton_cancelConsume->hide();
        if(jsonrecv.value("state").toInt()==1){
            //ui->label_erroNote->setText("取消成功!");
            //showAll("本次消费已取消",110,200,40);
            //QTimer::singleShot(500,this,SLOT(showPic()));
            showPic();
            myPtf("stop queConsumeRet timer(outer)!\n");
            //timerSearch->stop();
            ui->label_input->setText("");
            timeTcur=0;//取消成功后，将当前时间戳归0
        }else{
            ui->label_erroNote->setText("取消失败!");
        }
        QTimer::singleShot(500,ui->groupBox_mask,SLOT(hide()));
    }
}
void MainWindow::clearScreen(int waitTime,int screenID)
{
    //清理主-副屏的函数
    int cleartime = getNowTime() + waitTime*10;
    myPtf("waitTime:%d\n",waitTime);
    myPtf("cleartime:%d\n",cleartime);
    myPtf("screenID:%d\n",screenID);
    if (screenID == FSTSCREEN)
    {
        myPtf("FSTSCREEN\n");
        fScreenClearTime = cleartime;
    }
    else if(screenID == SECSCREEN)
    {
        myPtf("SECSCREEN\n");
        sScreenClearTime = cleartime;
    }
}
void MainWindow::clearPannel()
{

    timerclearScreen->stop();
    //交易成功后，主屏清屏操作
    int nowtime = getNowTime();
    //myPtf("nowtime%d\n",nowtime);
    //myPtf("sScreenClearTime%d\n",sScreenClearTime);
    //myPtf("fScreenClearTime%d\n",fScreenClearTime);
    if(nowtime > fScreenClearTime)
    {
        fScreenClearTime = MAXTIMESTAMP;
        //清空副屏
        showPic();
    }
    if(nowtime > sScreenClearTime)
    {
        sScreenClearTime = MAXTIMESTAMP;
        //清空主屏
        if(attrib1==1){
            recoveryAccoInfo();
        }else{
            myPtf("qazdaq\n");
            hideConsumeMsg();
        }
    }
    timerclearScreen->start(100);
}


void MainWindow::on_pushButton_examination_clicked()
{
    //按键--自检
    if(keyFlag==1){
        return;
    }
    if(keyFlag_cashier==1){
        return;
    }
    //点击卡自检,刷卡，获取账户信息
    if(attrib1==1){
        reqAccoInfo();
    }
}
void MainWindow::on_pushButton_cashier_clicked()
{
    //按键--充/取钱
    if(keyFlag==1){
        return;
    }
    if(keyFlag_cashier==1){
        return;
    }
    if(attrib1==7){
        return;
    }
    if(cashierFlag==1){//切换至存款
        myPtf("cunkuan------------------\n");
        cashier="1";
        ui->label_workType->setText("存款");
        cashierFlag=0;
    }else{//切换至取款
        myPtf("qukuan------------------\n");
        cashier="2";
        ui->label_workType->setText("取款");
        cashierFlag=1;
    }
}
void MainWindow::reqAccoInfo()
{
    //请求刷卡自检
    //组织数据
    QJsonObject jsonsend;
    QDateTime time = QDateTime::currentDateTime();
    timeTcur_check = time.toTime_t();//当前时间戳
    jsonsend.insert("timeStamp",QString::number(timeTcur_check));
    jsonsend.insert("transactionType",consumeType);
    jsonsend.insert("termCode",termCode);
    jsonsend.insert("termID",termID);
    jsonsend.insert("areaID",areaID);
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = UI_REQ_ACCOUNTINFO;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_main->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+UI_REQ_ACCOUNTINFO){
        //数据发送失败，可以尝试重连
        ui->label_tips->show();
        //ui->label_tips->setText("发送请求消费命令失败!,ret="+QString::number(Cmd));
        ui->label_tips->setText("卡自检服务断开，请重新连接服务！");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
        myPtf("recv UI_REQ_ACCOUNTINFO ret error!\n");
    }else if(Cmd == CLOUD_SENDCMD_ERROR){
        serverError();
    }else{
        myPtf("recv UI_REQ_ACCOUNTINFO ret ok!\n");
        ui->label_noteMes->show();
        //解析json字符串
        if(jsonrecv.value("state")==1){
            dateStamp_check= jsonrecv.value("timeStamp").toString();//返回时间戳
            if(dateStamp_check.toInt()!=timeTcur_check)
            {
                ui->label_noteMes->setText("返回时间戳错误!");
                QTimer::singleShot(3000,ui->label_noteMes,SLOT(hide()));
                myPtf("dateStamp error! timeTcur_check=%d\n",timeTcur_check);
            }
            else
            {
                showAccoInfoReady();
                keyOtherFlag=1;
                myPtf("start to queAccoInfoRet\n");
                //timerCheckCard->start(200);//启动查询消费
            }
        }else{
            ui->label_noteMes->setText("接收失败");
            QTimer::singleShot(3000,ui->label_noteMes,SLOT(hide()));
        }
    }
}
void MainWindow::showAccoInfoReady()
{
    if(attrib1 !=7){
        ui->label_info->setText("请刷卡或扫码，获取账户信息");
        ui->label_noteMes->setText("");
        ui->groupBox_cashierBg->show();
        ui->pushButton_cancel->show();
        player("/usr/local/nkty/players/video15.wav");//请读卡
    }
    showAll("请刷卡或扫码!",130,200,40);

}
void MainWindow::showAccoInfoResult(int result)
{
    //得到查询结果后显示
    QDateTime time = QDateTime::currentDateTime();
    myPtf("time----------start showAccoInfoResult! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
    switch (result) {
    case CONSUME_SUCCESS:
        myPtf("time----------start showAccoInfoResult 1! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
        showAccoInfoMsg();
        timeTcur_check=0;//取消成功后，将当前时间戳归0

        //主屏显示
        if(attrib1==1){
            //账户状态
            if(realUseFlag==7){
                accoInfoFlag=1;
                //账户名称
                ui->label_cardNumber_teller->setText(accoName);
                //个人账户余额--工作模式为出纳
                ui->label_remainMoney_teller->setText(remainMoney1);
                //QTimer::singleShot(20000,this,SLOT(recoveryAccoInfo()));
                clearScreen(20000,FSTSCREEN);
            }
            else if(realUseFlag==1){
                errorPannel(CONSUME_SUCCESS,"该账户已挂失","/usr/local/nkty/players/video20.wav");
            }else if(realUseFlag==17){
                errorPannel(CONSUME_SUCCESS,"该账户已挂失冻结","/usr/local/nkty/players/video22.wav");
            }else if(realUseFlag==23){
                errorPannel(CONSUME_SUCCESS,"该账户已冻结","/usr/local/nkty/players/video21.wav");
            }
        }else if(attrib1==7){
            ui->groupBox_searchBg->show();
            //账户名称
            ui->label_accoName_search->setText(accoName);
            //账户状态
            if(realUseFlag==1){
                ui->label_userType_search->setText("挂失");
            }else if(realUseFlag==7){
                ui->label_userType_search->setText("正常");
            }else if(realUseFlag==17){
                ui->label_userType_search->setText("挂失冻结");
            }else if(realUseFlag==23){
                ui->label_userType_search->setText("冻结");
            }
            //部门名称
            ui->label_deptName_search->setText(deptName);
            //身份名称
            ui->label_pidName_search->setText(pidName);
            //个人账户余额--工作模式为出纳
            ui->label_remainMoneys_search->setText(remainMoney1);
            //补贴账户余额--工作模式为出纳
            ui->label_secAccoMoneys_search->setText(secAccoMoney1);
        }
        if(attrib1==1){
            if(realUseFlag==7){
                //副屏显示
                showAll("账户名称:"+ui->label_cardNumber_teller->text(),20,160,35);
                appendtxt("个人账户余额:"+ui->label_remainMoney_teller->text()+"元",20,230,35);
                time = QDateTime::currentDateTime();
                myPtf("start showAccoInfoResult 4! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
                time = QDateTime::currentDateTime();
                myPtf("start showAccoInfoResult 5! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
            }
        }else if(attrib1==7){
            //副屏显示
            showAll("账户名称:"+ui->label_accoName_search->text(),20,90,35);
            appendtxt("账户状态:"+ui->label_userType_search->text(),20,160,35);
            appendtxt("个人账户余额:"+ui->label_remainMoneys_search->text()+"元",20,230,35);
            appendtxt("补贴账户余额:"+ui->label_secAccoMoneys_search->text()+"元",20,300,35);
            time = QDateTime::currentDateTime();
            myPtf("start showConsumeResult 4! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
            time = QDateTime::currentDateTime();
            myPtf("start showAccoInfoResult 5! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
        }
        if(attrib1==7){
            QTimer::singleShot(10000,this,SLOT(returnAccoInfo()));
        }

        time = QDateTime::currentDateTime();
        myPtf("time----------start showAccoInfoResult 2! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
        break;
    case QR_DECODE_ERROR:
        errorPannel(QR_DECODE_ERROR,"二维码解码失败","/usr/local/nkty/players/video8.wav");//二维码解码失败
        ui->groupBox_cashierBg->hide();
        if(attrib1==7){
            QTimer::singleShot(1000,this,SLOT(returnAccoInfo()));
        }else if(attrib1==1){
            QTimer::singleShot(1000,this,SLOT(recoveryAccoInfo()));
        }

        break;
    case QR_DECODE_TIMEOUT:
        errorPannel(QR_DECODE_TIMEOUT,"请求超时","/usr/local/nkty/players/video9.wav");//请求超时
        ui->groupBox_cashierBg->hide();
        if(attrib1==7){
            QTimer::singleShot(1000,this,SLOT(returnAccoInfo()));
        }else if(attrib1==1){
            QTimer::singleShot(1000,this,SLOT(recoveryAccoInfo()));
        }

        break;
    case CONSUME_NOT_ACCOUNTS:
        errorPannel(CONSUME_NOT_ACCOUNTS,"账户不存在或已注销","/usr/local/nkty/players/video6.wav");//账户不存在或已注销
        ui->groupBox_cashierBg->hide();
        if(attrib1==7){
            QTimer::singleShot(1000,this,SLOT(returnAccoInfo()));
        }else if(attrib1==1){
            QTimer::singleShot(1000,this,SLOT(recoveryAccoInfo()));
        }

    default:
        break;
    }
    keyFlag=0;
    keyOtherFlag=0;

    if(attrib1==7){
        keyMenuFlag=1;
    }else if(attrib1==1 ){
        if(accoInfoFlag==0){
            keyMenuFlag=1;
        }else{
            keyMenuFlag=0;
        }
    }else{
        keyMenuFlag=0;
    }

    myPtf("finish showAccoInfoResult!\n");
}
void MainWindow::recoveryAccoInfo()
{
    //恢复自检初始状态
    hideAccoInfoMsg();
    accoInfoFlag=0;
    keyMenuFlag=1;
    ui->label_input->setText("");
    showPic();
    myPtf("showPic001");
}
void MainWindow::returnAccoInfo(){
    //恢复查询机初始状态
    showPic();
    ui->groupBox_searchBg->hide();
    ui->label_accoName_search->setText("");
    ui->label_userType_search->setText("");
    ui->label_deptName_search->setText("");
    ui->label_pidName_search->setText("");
    ui->label_remainMoneys_search->setText("");
    ui->label_secAccoMoneys_search->setText("");
    reqAccoInfo();
}
void MainWindow::queAccoInfoRet(QJsonObject jsonrecv)
{
    //查询卡账户信息结果
    //开始查询的时间戳
    QDateTime time = QDateTime::currentDateTime();
    myPtf("time----------start to queConsumeRet! %02d:%02d:%02d\n",time.time().hour(), time.time().minute(), time.time().second());
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
            if(jsonrecv.contains("user")){
                QJsonValue userValue = jsonrecv.value("user");
                if(userValue.isObject()){
                    QJsonObject userObject = userValue.toObject();
                    realUseFlag=userObject.value("realUseFlag").toInt();
                    cardId=userObject.value("cardId").toInt();
                    accoName=userObject.value("accoName").toString();
                    deptName=userObject.value("deptName").toString();
                    pidName=userObject.value("pidName").toString();
                    sprintf(remainMoney1,"%.2f",((float)userObject.value("remainMoney").toInt())/100);
                    intRemainMoney=userObject.value("remainMoney").toInt();
                    sprintf(secAccoMoney1,"%.2f",((float)userObject.value("secAccoMoney").toInt())/100);
                }
            }
        }
        myPtf("showAccoInfoResult ret=%d\n",ret);
        showAccoInfoResult(ret);

        //如果是解码失败、云服务失败等原因，应在报错等待后重新发起下一轮消费请求
        if(ret<0)
        {
            QTimer::singleShot(2000,this,SLOT(reqAccoInfo()));
        }
        break;
    //各种错误，一般不可能发生，可在显示错误后结束本次操作
    case 2:
    case 3:
    case 4:
        //显示失败信息，结束
        myPtf("showAccoInfoResult state=%d\n",state);
        showAccoInfoResult(state);
        break;
    case 5:
        //不显示消息，重新发起请求
        myPtf("continue to queAccoInfoRet\n");
        //timerCheckCard->start(200);
        break;
    default:
        break;
    }
}
void MainWindow::showAccoInfoMsg()
{
    //显示卡账户信息
    ui->label_noteMes->show();
    ui->label_noteMes->setText("");
    ui->groupBox_cashierBg->hide();

    if(attrib1!=7){
        if(keyboardFlag==1){
            ui->groupBox_keyboardBg->show();
            ui->pushButton_gotoFlowlist->hide();//右侧查看流水按钮隐藏
            ui->pushButton_keyboard->hide();//右侧软键盘按钮隐藏
            ui->groupBox_totalCashierBg->hide();
        }else{
            ui->groupBox_keyboardBg->hide();
            ui->pushButton_gotoFlowlist->show();//右侧查看流水按钮显示
            ui->pushButton_keyboard->show();//右侧软键盘按钮显示
            ui->groupBox_totalCashierBg->show();
        }
    }


}
void MainWindow::hideAccoInfoMsg()
{
    //隐藏卡账户信息
    ui->label_remainMoney_teller->setText("");
    ui->label_cardNumber_teller->setText("");
    if(attrib1==1){
        cashierFlag=0;
        cashier="1";
        ui->label_noteMes->setText("");
        ui->label_remainMoneys->setText("");
        if(rets ==10){
            ui->label_workType->setText("存款");
        }else{
            ui->label_workType->setText("脱机");
        }

    }
}
void MainWindow::cancelAccoInfo()
{
    //取消卡自检
    keyOtherFlag=0;
    QJsonObject jsonsend;
    jsonsend.insert("timeStamp",dateStamp_check);
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = UI_CANCEL_CONSUME;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_main->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+UI_CANCEL_CONSUME){
        //数据获取失败，可以尝试重连
        /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
        message.exec();*/
        ui->label_tips->show();
        ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(Cmd == CLOUD_SENDCMD_ERROR){
        serverError();
    }else{
        /*QString strShow = "";
        strShow.append(retJson);
        QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        message.exec();*/
        //解析json字符串
        ui->pushButton_cancel->hide();
        if(jsonrecv.value("state").toInt()==1){
            ui->label_info->setText("取消成功!");
            showAll("本次自检已取消",120,200,40);
            QTimer::singleShot(2000,this,SLOT(showPic()));
            myPtf("stop queConsumeRet timer(outer)!\n");
            //timerCheckCard->stop();
            ui->label_input->setText("");
            timeTcur_check=0;//取消成功后，将当前时间戳归0
        }else{
            ui->label_info->setText("取消失败!");
        }
        QTimer::singleShot(2000,ui->groupBox_cashierBg,SLOT(hide()));
    }
}
void MainWindow::on_pushButton_cancel_clicked()
{
    //卡自检--取消
    cancelAccoInfo();
}

void MainWindow::on_pushButton_login_clicked()
{
    //登录验证(出纳)
    QString pwd = ui->lineEdit_password->text();
    if(pwd ==""){
        ui->label_erroMes->show();
        ui->label_erroMes->setText("请输入密码!");
        QTimer::singleShot(2000,ui->label_erroNote,SLOT(hide()));
    }else if(pwd !=termPassWord){
        ui->label_erroMes->show();
        ui->label_erroMes->setText("密码错误!");
        QTimer::singleShot(2000,ui->label_erroNote,SLOT(hide()));
    }else{
        loginFlag=0;
        keyMenuFlag=1;
        ui->groupBox_login->hide();
    }
}

void MainWindow::on_pushButton_backs_clicked()
{
    //按键--返回
    keyboardFlag=0;
    if(rets !=10){
        ui->groupBox_totalPannelBg->hide();
        ui->pushButton_gotoFlowlist->hide();

        ui->groupBox_keyboardBg->hide();
        ui->pushButton_keyboard->show();

        ui->label_offlineFlowCount_title->show();
        ui->label_offlineFlowCount->show();

    }else{
        ui->pushButton_keyboard->show();
        ui->pushButton_gotoFlowlist->show();


        ui->groupBox_keyboardBg->hide();
        ui->groupBox_totalPannelBg->show();
        if(attrib1==6){
            ui->label_totalFrequency_title->hide();
            ui->label_totalFrequency->hide();
            ui->label_consumeLimitMoney->hide();
            ui->label_timeSegName->hide();
        }else if(attrib1==2 || attrib1==3){
            ui->label_consumeLimitMoney->show();
            ui->label_timeSegName->show();
        }else if(attrib1==7){
            ui->groupBox_totalPannelBg->hide();
            ui->pushButton_gotoFlowlist->hide();
        }
    }

}

void MainWindow::on_pushButton_consume_clicked()
{
    if(consumeState==1){
        keyFlag=1;//让终端键盘在定额消费模式时不可点击
    }
    ui->groupBox_mask->hide();//信息提示的遮罩隐藏
    ui->groupBox_pwdBg->hide();//消费时，如果需要输入密码，弹出该对话框隐藏
    pwd = ui->lineEdit_password_consume->text();
    //reqConsume();
    //脱机新建
    if(rets==10){
        reqConsume(5);
    }else{
        offLineConsume();
    }
    ui->lineEdit_password_consume->setText("");
}

void MainWindow::on_pushButton_consume_cancel_clicked()
{
    //修改
    cancelConsume();
    if(consumeState==1){
        keyFlag=1;//让终端键盘在定额消费模式时不可点击
    }
    ui->groupBox_mask->hide();//信息提示的遮罩隐藏
    ui->groupBox_pwdBg->hide();//消费时，如果需要输入密码，弹出该对话框隐藏
    ui->lineEdit_password_consume->setText("");
    ui->label_input->setText("");

    //将累加金额，清空还原
    totalstr="";
    memset(totalnum,0, sizeof(totalnum));
    arrnum=0;//数组下标
    ui->label_sum->setText("");

    addFlag=0;
    keyFlag=0;

    loginFlag=0;
    cardId=0;
    pwd="";
    pwdFlag=0;
}

//脱机操作功能
//脱机新建
void MainWindow::showOfflineConsumeMsg(QString noteMes)
{
    //脱机消费成功
    keyOtherFlag=0;
    ui->groupBox_mask->hide();//信息提示的遮罩隐藏
    ui->label_noteMes->setText(noteMes);
    ui->label_input->setText("");
    ui->label_sum->setText("");
    int flowCountNum = ui->label_offlineFlowCount->text().toInt()+ 1;
    ui->label_offlineFlowCount->setText(QString::number(flowCountNum));
    player("/usr/local/nkty/players/video18.wav");//音频
    showAll(noteMes,150,200,40);
}
void MainWindow::offLineConsume()
{
    //脱机消费
    QJsonObject jsonsend;
    jsonsend.insert("termCode",termCode);
    QDateTime time = QDateTime::currentDateTime();
    timeTcur = time.toTime_t();//当前时间戳
    jsonsend.insert("timeStamp",QString::number(timeTcur));

    if(consumeState==1){//定额消费
        moneyFloat=consumeMoney.toDouble();
    }else{//普通消费
        if(addFlag==1){
            moneyFloat=0;
            for(int i=0;i<100;i++){
                if(totalnum[i]>0){
                    moneyFloat = moneyFloat + totalnum[i];
                }
            }
            if(moneyFloat>0){
                moneyFloat=moneyFloat+ui->label_input->text().toDouble();
            }
        }else{
            moneyFloat=ui->label_input->text().toDouble();
        }

        if(rets == 10){
            if(moneyFloat > ui->label_note_consumeLimitMoney->text().toDouble()){
                ui->label_tips->show();
                ui->label_tips->setText("请输入小于"+ui->label_note_consumeLimitMoney->text()+"的金额!");
                QTimer::singleShot(2000,ui->label_tips,SLOT(hide()));
                ui->label_input->setText("");
                //将累加金额，清空还原
                memset(totalnum,0, sizeof(totalnum));
                arrnum=0;//数组下标
                totalstr="";
                ui->label_sum->setText("");
                addFlag=0;
                return;
            }
        }else{
            if(moneyFloat > OFFLINEMAXMONEY){
                ui->label_tips->show();
                ui->label_tips->setText("请输入小于"+QString::number(OFFLINEMAXMONEY)+"的金额!");
                QTimer::singleShot(2000,ui->label_tips,SLOT(hide()));
                ui->label_input->setText("");
                //将累加金额，清空还原
                memset(totalnum,0, sizeof(totalnum));
                arrnum=0;//数组下标
                totalstr="";
                ui->label_sum->setText("");
                addFlag=0;
                return;
            }
        }

    }
    if(attrib1==2 || attrib1==3){
        money = (int)(moneyFloat * 100 +0.01);
    }else if(attrib1==6){
        money = (int)(moneyFloat);
    }

    if(addFlag==1){
        ui->label_input->setText(moneySum);
    }
    jsonsend.insert("money",QString::number(money));

    //发送数据
    QJsonObject jsonrecv;
    int Cmd = UI_REQ_OFFLINECONSUME;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_main->send_Command(&Cmd,jsonsend,retJson);

    if(Cmd != 10000+UI_REQ_OFFLINECONSUME){
        //数据发送失败，可以尝试重连
        ui->label_tips->show();
        //ui->label_tips->setText("发送请求消费命令失败!,ret="+QString::number(Cmd));
        ui->label_tips->setText("消费服务断开，请重新连接服务！");
        QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
        myPtf("recv UI_REQ_CONSUME ret error!\n");
    }else if(Cmd == CLOUD_SENDCMD_ERROR){
        serverError();
    }else{
        ui->label_noteMes->show();
        //解析json字符串
        if(jsonrecv.value("state").toInt()==1){
            dateStamp= jsonrecv.value("timeStamp").toString();//返回时间戳
            if(dateStamp.toInt()!=timeTcur)
            {
                ui->label_noteMes->setText("返回时间戳错误!");
                QTimer::singleShot(3000,ui->label_noteMes,SLOT(hide()));
                myPtf("dateStamp error! timeTcur=%d\n",timeTcur);
            }else{
                showConsumeReady();
                QTimer::singleShot(100,this,SLOT(consumePlayer()));
                addFlag=0;
                keyOtherFlag=1;
            }
        }else{
            ui->label_noteMes->setText("接收失败");
            QTimer::singleShot(3000,ui->label_noteMes,SLOT(hide()));
        }
    }
}
void MainWindow::getFlowNum()
{
    myPtf("iNum:%d\n",iNum);
    iNum=iNum+1;
    QJsonObject jsonsend;
    jsonsend.insert("termCode",termCode);
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = UI_GET_OFFLINEFLOWNUM;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_main->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+UI_GET_OFFLINEFLOWNUM){
       //数据获取失败，可以尝试重连
       /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
       message.exec();*/
       ui->label_tips->show();
       ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
       QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(Cmd == CLOUD_SENDCMD_ERROR){
        serverError();
    }else{
        myPtf("qqstate--:%d\n",jsonrecv.value("state").toInt());
        if(jsonrecv.value("state").toInt()==1){
            flowCount = jsonrecv.value("flowCount").toInt();
            myPtf("qqflowCount--:%d\n",flowCount);
            if(rets ==10){
                //state=1:正在上传

                if(flowCount !=0){
                    myPtf("qqconsumeState:%d\n",consumeState);
                    if(consumeState==1){
                        cancelConsume();//关闭消费
                    }

                    ui->label_process->setText("正在上传，还剩余"+QString::number(flowCount)+"条！");
                    ui->pushButton_confirm_cancel->show();
                    ui->pushButton_confirm_upload->show();
                    //uploadFlag=1;
                    if(flowFlag==0){
                        ui->groupBox_flowBg->show();
                        flowFlag=1;
                    }
                }else{
                    ui->label_process->setText("当前无需上传的流水！");
                    myPtf("qqqaaa");
                    uploadFlag=0;
                    QTimer::singleShot(1000,this,SLOT(clearUploadPanel()));
                    connectFlag=1;
                    timerMain->start(500);
                }
            }else{
                myPtf("ddflowCount--:%d\n",flowCount);
                ui->label_offlineFlowCount->setText(QString::number(flowCount));
            }

        }else{
            //state=0:当前没有上传
            ui->label_process->setText("当前没有上传");
            QTimer::singleShot(2000,this,SLOT(clearUploadPanel()));
        }
    }
}

void MainWindow::getOffLineFlowNum()
{
    //获取尚未上传的脱机流水条数
    timerFlowNum->stop();
    getFlowNum();
}
void MainWindow::uploadFlow(int num)
{
    myPtf("1dddNum%d\n",num);
    //上传脱机流水
    QJsonObject jsonsend;
    jsonsend.insert("termCode",termCode);
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = UI_REQ_UPLOADFLOW;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_main->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+UI_REQ_UPLOADFLOW){
       //数据获取失败，可以尝试重连
       /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
       message.exec();*/
       ui->label_tips->show();
       ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
       QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else if(Cmd == CLOUD_SENDCMD_ERROR){
        serverError();
    }else{
        myPtf("wwstate--:%d\n",jsonrecv.value("state").toInt());
        if(jsonrecv.value("state").toInt()==1){
            //state=1表示成功开始上传
            if(flowCount>0){
                myPtf("max-flowCount:%d\n",flowCount);
                ui->label_tips_process->setText("正在上传中!");
            }else{
                myPtf("min-flowCount:%d\n",flowCount);
            }

        }else{
            //state=0表示当前没有脱机流水无需上传
            if(flowCount==0){
                ui->label_tips_process->setText("用户上传完成!");
            }else{
                ui->label_tips_process->setText("当前没有脱机流水无需上传！");
            }
            uploadFlag=0;
            QTimer::singleShot(2000,this,SLOT(clearUploadPanel()));
        }
    }
}

void MainWindow::on_pushButton_termEnter_clicked()
{
    //脱机模式--进入脱机模式
    ui->groupBox_termInfoBg->hide();
    //ui->pushButton_upload->show();//脱机模式时，“上传”按钮显示
    ui->pushButton_backs->hide();//正常模式时，“合计”按钮(大)隐藏
}



/*void MainWindow::on_pushButton_upload_clicked()
{
    //if(consumeState==1){
        //myPtf("daqdaq");
        //cancelConsume();
    //}
    ui->groupBox_flowBg->show();
    if(rets==10){
        timerFlowNum->start(100);//开启定时器，定时查询尚未上传的脱机流水条数
    }else{
        ui->label_process->setText("当前处于未联网状态，不能进行上传操作！");
        QTimer::singleShot(2000,this,SLOT(clearUploadPanel()));
    }
}*/

void MainWindow::on_pushButton_enter_clicked()
{
    //进入联网模式
    connectFlag=1;
    timerFlowNum->start(100);//开启定时器，定时查询尚未上传的脱机流水条数
    ui->groupBox_normalBg->hide();
}

void MainWindow::on_pushButton_confirm_cancel_clicked()
{
    uploadFlag=0;
    connectFlag=1;
    flowCount=0;
    ui->groupBox_flowBg->hide();
    ui->pushButton_confirm_upload->setEnabled(true);
}

void MainWindow::on_pushButton_confirm_upload_clicked()
{
    ui->pushButton_confirm_upload->setEnabled(false);
    //ui->pushButton_confirm_cancel->setEnabled(false);
    if(flowCount>0){
        uploadFlow(22);
        uploadFlag=1;
    }

}
void MainWindow::clearUploadPanel()
{
    ui->groupBox_flowBg->hide();
    ui->label_process->setText("");
    ui->label_tips_process->setText("");
    ui->pushButton_confirm_cancel->hide();
    ui->pushButton_confirm_upload->hide();
    ui->pushButton_confirm_upload->setEnabled(true);
    //ui->pushButton_confirm_cancel->setEnabled(true);
    uploadFlag=0;
}
int MainWindow::setAttrib1(int attr)
{
    //打开文件
    FILE * pFile;
    pFile = fopen( ATTRIB1_FILE , "wb" );//wb 只写打开或新建一个二进制文件；只允许写数据。
    int buffer[] ={attr};
    if(pFile == NULL){
        myPtf("can not open offline_file\n");
        return -1;//有文件但无法打开，返回-1
    }
    //写入文件
    fwrite(buffer,sizeof(int),1, pFile);
    //关闭文件
    fclose (pFile);
    return 0;
}
int MainWindow::getAttrib1()
{
    //打开文件
    FILE * pFile;
    pFile = fopen( ATTRIB1_FILE , "rb" );//rb+ 读写打开一个二进制文件，只允许读写数据。
    int buffer[1];
    if(pFile == NULL){
        myPtf("can not open offline_file\n");
        return -1;//有文件但无法打开，返回-1
    }
    //读取文件
    fread(buffer,sizeof(int),1, pFile);
    attrib1New=buffer[0];
    myPtf("daqAttr1:%d\n",attrib1New);
    return 0;
}
