#include "flow.h"
#include "ui_flow.h"
#include "qmessagebox.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QTableView>
flow::flow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::flow)
{
    ui->setupUi(this);
    //定时获取页面信息
    timerFlow = new QTimer(this);

    timerDownloadFlowNum = new QTimer(this);
    init();//整体--初始化函数

    setWindowFlags(Qt::FramelessWindowHint);
    showFullScreen();//全屏
}

flow::~flow()
{
    delete ui;
    //delete myCommon;
    delete flow_model;
    delete timerFlow;
    delete timerDownloadFlowNum;
}
void flow::init()
{
    //界面标签显示状态
    ui->label_unconnect->hide();
    ui->label_connectName->hide();
    ui->label_connect01->hide();
    ui->label_connect02->hide();
    ui->label_connect->hide();
    ui->label_network->hide();
    ui->label_4G->hide();
    ui->label_link->hide();

    ui->label_tips->hide();//提示语
    ui->groupBox_dateTime->hide();//时间控件
    ui->tableView_flowList->setSelectionMode(QAbstractItemView::NoSelection);//设置表格内容不可选
    ui->groupBox_countAndSum->hide();
    ui->groupBox_incountAndOutcount->hide();
    ui->groupBox_mask->hide();
    ui->groupBox_login->hide();
    ui->lineEdit_password->setEchoMode(QLineEdit::Password);//设置输入密码框
    ui->lineEdit_password->installEventFilter(this);//在窗体上为lineEdit安装过滤器

    ui->pushButton_refundMoney->hide();

    //ui->pushButton_refundMoney->hide();
    ui->groupBox_flowBg->hide();
    flowID=0;
    flowFlag=0;

    userPackCount=0;
    downloadFlag=0;
    //flowFlags=0;

    //隐藏脱机和下载名单，modified by yangtao 2019-05-05
    if(AllowedOfflineConsume == 0)
    {
        ui->pushButton_download->hide();
    }


    //信息初始化
    connect(timerFlow,SIGNAL(timeout()),this,SLOT(flowInit()));
    timerFlow->start(1000);

    //获取尚未上传的脱机流水条数
    connect(timerDownloadFlowNum,SIGNAL(timeout()),this,SLOT(getOffLineDownloaFlowNum()));
}
void flow::flowInit()
{
    timerFlow->stop();
    QDateTime time = QDateTime::currentDateTime();
    QString str = time.toString("yyyy-MM-dd hh:mm:ss");
    ui->label_dateTime->setText(str);
    //流水初始化
    if(initFlag==0){
        //流水界面--初始化函数
        getFlowTotalAndInfo(pageNum);
        //终端号
        ui->label_terminalNumeber->setText(QString::number(termNo_flow));
        //getNetworkConnectStatus();//初始化网络状态
        //信息栏显示情况
        if(attrib1_flow==1){
            ui->groupBox_countAndSum->hide();
            ui->groupBox_incountAndOutcount->show();
        }else{
            ui->groupBox_countAndSum->show();
            ui->groupBox_incountAndOutcount->hide();
            if(attrib1_flow==2 || attrib1_flow==3){
                ui->label_totalSum_title->setText("消费金额(元):");
            }else if(attrib1_flow==6){
                ui->label_totalSum_title->setText("消费券数(张):");
            }
        }
        //服务器状态
        if(ret_flow==10){
            ui->label_link->show();
            ui->label_unlink->hide();
            ui->label_linkName->setText("服务器已连接");
        }else{
            ui->label_unlink->show();
            ui->label_link->hide();
            ui->label_linkName->setText("服务器未连接");
        }
        //退款按钮
        if(attrib1_flow==2 || attrib1_flow==3 || attrib1_flow==6){
            ui->pushButton_refundMoney->show();
        }else{
            ui->pushButton_refundMoney->hide();
        }
    }


    /*if(time.time().second()==0){
        getNetworkConnectStatus();
    }*/
    if(downloadFlag==1){
        timerDownloadFlowNum->start(500);//继续查询尚未上传的脱机流水条数
    }

    timerFlow->start(500);
}
void flow::setDateTime()
{
    //获取日期
    QDateTime dateTime = QDateTime::currentDateTime();
    ui->year->setText(QString::number(dateTime.date().year()));
    ui->month->setText(QString::number(dateTime.date().month()));
    ui->day->setText(QString::number(dateTime.date().day()));
    ui->hour->setText(QString::number(dateTime.time().hour()));
    ui->minute->setText(QString::number(dateTime.time().minute()));
    ui->second->setText(QString::number(dateTime.time().second()));
}
/*void flow::getNetworkConnectStatus()
{
    //获取网络连接状态
    myCommon=new common();
    myCommon->scmd_network_common=scmd_network_flow;
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
void flow::getFlowTotalAndInfo(int pageNum)
{
    //获取流水总金额和总条数--&--获取流水明细
    QDateTime dateTime = QDateTime::currentDateTime();
    QString time = dateTime.toString("yyyy-MM-dd");//当天
    QString timeNow = dateTime.toString("yyyy-MM-dd hh:mm:ss");
    QString timePreDay = dateTime.addDays(-3).toString("yyyy-MM-dd");//前三天
    QString timePreWeek = dateTime.addDays(-7).toString("yyyy-MM-dd");//前七天
    QString timePreMonth = dateTime.addDays(-30).toString("yyyy-MM-dd");//前30天
    QString startTime ="";
    QString endTime ="";
    if(flag==1){
        startTime = ui->pushButton_startTime->text();
        endTime = ui->pushButton_endTime->text();
    }else if(flag==2){
        startTime = timePreDay + " 00:00:00";
        endTime = time + " 00:00:00";
    }else if(flag==3){
        startTime = timePreWeek + " 00:00:00";
        endTime = time + " 00:00:00";
    }else if(flag==4){
        startTime = timePreMonth + " 00:00:00";
        endTime = time + " 00:00:00";
    }else{
        startTime = time + " 00:00:00";
        endTime = timeNow;
    }
    //将开始时间和结束时间分别放入相应的标签中
    ui->pushButton_startTime->setText(startTime);
    ui->pushButton_endTime->setText(endTime);

    //消费流水总数--初始化
    QJsonObject jsonsend1;
    jsonsend1.insert("startTime",startTime);
    jsonsend1.insert("endTime",endTime);
    jsonsend1.insert("termCode",termCode_flow);
    jsonsend1.insert("termID",termID_flow);
    jsonsend1.insert("pageNum",pageNum);
    jsonsend1.insert("pageSize",pageSize);
    //发送数据
    QJsonObject jsonrecv1;
    int Cmd1 = UI_GET_FLOWTOTAL;
    char retJson1[1024];
    memset(retJson1,0,1024);
    jsonrecv1 = scmd_main_flow->send_Command(&Cmd1,jsonsend1,retJson1);
    if(Cmd1 != 10000+UI_GET_FLOWTOTAL){
       //数据获取失败，可以尝试重连
       /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd1));
       message.exec();*/
       ui->label_tips->show();
       ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd1));
       QTimer::singleShot(5000,ui->label_tips,SLOT(hide()));
    }else{
       /*QString strShow1 = "";
       strShow1.append(retJson1);
       QMessageBox message(QMessageBox::NoIcon,"提示","json1:"+strShow1);
       message.exec();*/
       //解析json字符串
       if(jsonrecv1.value("ret").toInt()==10){
           initFlag=1;

           if(attrib1_flow==1){
               //数据中条数
               page = jsonrecv1.value("inCount").toInt() + jsonrecv1.value("outCount").toInt();

               //充值总金额
               char inSum[20];
               sprintf(inSum,"%.2f",((float)jsonrecv1.value("inSum").toInt())/100);
               ui->label_totalInsum->setText(inSum);
               //充值笔数
               ui->label_totalIncount->setText(QString::number(jsonrecv1.value("inCount").toInt()));
               //取款总金额
               char outSum[20];
               sprintf(outSum,"%.2f",((float)jsonrecv1.value("outSum").toInt())/100);
               ui->label_totalOutsum->setText(outSum);
               //取款笔数
               ui->label_totalOutcount->setText(QString::number(jsonrecv1.value("outCount").toInt()));
           }else{
               //数据中条数：
               page = jsonrecv1.value("inCount").toInt();
               count = jsonrecv1.value("count").toInt();


               if(attrib1_flow==2 || attrib1_flow==3){
                  char sum[20];
                  sprintf(sum,"%.2f",((float)jsonrecv1.value("sum").toInt())/100);
                  ui->label_totalSum->setText(sum);
                  ui->label_totalCount->setText(QString::number(count));
               }else if(attrib1_flow==6){
                  ui->label_totalSum->setText(QString::number(jsonrecv1.value("sum").toInt()));
                  ui->label_totalCount->setText(QString::number(count));
               }
           }
           //页码
           if(page==0){
               pageTotal=1;
           }else{
               pageTotal=page %  pageSize == 0 ? page / pageSize : page / pageSize + 1;
           }
           ui->label_page->setText("页码："+QString::number(pageNum)+"/"+QString::number(pageTotal));
       }
    }

    //消费流水列表--初始化
    //一 添加表头
    //准备数据模型
    flow_model =new QStandardItemModel();
    flow_model->setHorizontalHeaderItem(0,new QStandardItem(QObject::tr("注册号")));
    if(attrib1_flow==1 || attrib1_flow==2 || attrib1_flow==3){
        flow_model->setHorizontalHeaderItem(1,new QStandardItem(QObject::tr("金额")));
    }else if(attrib1_flow==6){
        flow_model->setHorizontalHeaderItem(1,new QStandardItem(QObject::tr("票数")));
    }

    flow_model->setHorizontalHeaderItem(2,new QStandardItem(QObject::tr("个人账户余额")));
    flow_model->setHorizontalHeaderItem(3,new QStandardItem(QObject::tr("补贴账户余额")));
    flow_model->setHorizontalHeaderItem(4,new QStandardItem(QObject::tr("附加费")));
    flow_model->setHorizontalHeaderItem(5,new QStandardItem(QObject::tr("流水类型")));
    flow_model->setHorizontalHeaderItem(6,new QStandardItem(QObject::tr("交易类型")));
    flow_model->setHorizontalHeaderItem(7,new QStandardItem(QObject::tr("时间")));

    ui->tableView_flowList->setModel(flow_model);
    //二 设置表格属性
    //设置表格的各列的宽度值
    ui->tableView_flowList->setColumnWidth(0,100);
    ui->tableView_flowList->setColumnWidth(1,90);
    ui->tableView_flowList->setColumnWidth(2,110);
    ui->tableView_flowList->setColumnWidth(3,110);
    ui->tableView_flowList->setColumnWidth(4,75);
    ui->tableView_flowList->setColumnWidth(5,75);
    ui->tableView_flowList->setColumnWidth(6,75);
    ui->tableView_flowList->setColumnWidth(7,140);
    ui->tableView_flowList->verticalHeader()->hide();
    //设置表格的单元为只读属性，即不能编辑
    ui->tableView_flowList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //三 设置数据
    QJsonObject jsonsend2;
    jsonsend2.insert("startTime",startTime);
    jsonsend2.insert("endTime",endTime);
    jsonsend2.insert("termCode",termCode_flow);
    jsonsend2.insert("termID",termID_flow);
    jsonsend2.insert("pageNum",pageNum);
    jsonsend2.insert("pageSize",pageSize);
    //发送数据
    QJsonObject jsonrecv2;
    int Cmd2 = UI_GET_FLOWINFO;
    char retJson2[1024];
    memset(retJson2,0,1024);
    jsonrecv2 = scmd_main_flow->send_Command(&Cmd2,jsonsend2,retJson2);
    if(Cmd2 != 10000+UI_GET_FLOWINFO){
       //数据获取失败，可以尝试重连
       /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd2));
       message.exec();*/
       ui->label_tips->show();
       ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd2));
       QTimer::singleShot(5000,ui->label_tips,SLOT(hide()));
    }else{
       if(jsonrecv2.value("ret").toInt()==10){
           QString strShow2 = "";
           strShow2.append(retJson2);
           /*QMessageBox message(QMessageBox::NoIcon,"提示","json2:"+strShow2);
           message.exec();*/

           if(jsonrecv2.contains("flows")){
                QJsonValue valueArray =jsonrecv2.value("flows");
                if(valueArray.isArray()){
                    //正式的数据，需要改数据类型
                    QString child_regID;
                    char child_flowMoney[20];
                    QString child_flowMoneys;

                    char child_remainMoney[20];
                    QString child_remainMoneys;

                    char child_secRemainmoney[20];
                    QString child_secRemainmoneys;

                    char child_manageFee[20];
                    QString child_manageFees;

                    QString child_conTypeID;

                    QString child_reType;

                    QString child_flowTime;
                    QJsonArray jsonArray = valueArray.toArray();
                    int sum=jsonArray.count();
                    myPtf("sum:%d\n",sum);
                    for(int i=0;i<sum;i++){
                        myPtf("i:%d\n",i);
                        QJsonValue childValue = jsonArray[i];
                        if(childValue.isObject()){
                            QJsonObject childObject =childValue.toObject();
                            if (childObject.contains("flowID"))
                            {
                               QJsonValue valueJson = childObject.value("flowID");
                               if(i==0 && flowFlag==0){
                                   flowID=valueJson.toInt();
                               }
                            }
                            if (childObject.contains("regID"))
                            {
                               QJsonValue valueJson = childObject.value("regID");
                               child_regID = valueJson.toString();
                            }
                            if (childObject.contains("flowMoney"))
                            {
                                QJsonValue valueJson = childObject.value("flowMoney");
                                if(attrib1_flow==1 || attrib1_flow==2 || attrib1_flow==3){

                                    sprintf(child_flowMoney,"%.2f",((float)valueJson.toInt())/100);
                                }else if(attrib1_flow==6){
                                    child_flowMoneys =QString::number(valueJson.toInt());
                                }
                            }
                            if (childObject.contains("remainMoney"))
                            {
                               QJsonValue valueJson = childObject.value("remainMoney");
                               if(attrib1_flow==1 || attrib1_flow==2 || attrib1_flow==3){
                                   sprintf(child_remainMoney,"%.2f",((float)valueJson.toInt())/100);
                               }else if(attrib1_flow==6){
                                   child_remainMoneys =QString::number(valueJson.toInt());
                               }
                            }
                            if (childObject.contains("secRemainmoney"))
                            {
                               QJsonValue valueJson = childObject.value("secRemainmoney");
                               if(attrib1_flow==1 || attrib1_flow==2 || attrib1_flow==3){
                                   sprintf(child_secRemainmoney,"%.2f",((float)valueJson.toInt())/100);
                               }else if(attrib1_flow==6){
                                   child_secRemainmoneys =QString::number(valueJson.toInt());
                               }
                            }

                            if (childObject.contains("manageFee"))
                            {
                               QJsonValue valueJson = childObject.value("manageFee");
                               if(attrib1_flow==1 || attrib1_flow==2 || attrib1_flow==3){
                                   sprintf(child_manageFee,"%.2f",((float)valueJson.toInt())/100);
                               }else if(attrib1_flow==6){
                                   child_manageFees =QString::number(valueJson.toInt());
                               }
                            }
                            if(childObject.contains("conTypeID")){
                                QJsonValue valueJson = childObject.value("conTypeID");
                                if(valueJson.toInt()==0){
                                    child_conTypeID = "主账户消费";
                                }else if(valueJson.toInt()==1){
                                    child_conTypeID = "存款";
                                }else if(valueJson.toInt()==2){
                                    child_conTypeID = "取款";
                                }else if(valueJson.toInt()==9){
                                    child_conTypeID = "补贴账户消费";
                                }

                            }
                            if(childObject.contains("reType")){
                                QJsonValue valueJson = childObject.value("reType");
                                if(valueJson.toInt()==1){
                                    child_reType = "交易完成";
                                }else if(valueJson.toInt()==2){
                                    child_reType = "已退款";
                                }
                                //控制是否能退款操作
                                if(i==0 && valueJson.toInt()==2){
                                    flowFlag=1;
                                }
                            }
                            if (childObject.contains("flowTime"))
                            {
                               QJsonValue valueJson = childObject.value("flowTime");
                               child_flowTime = valueJson.toString();
                               QDateTime time;
                               time = QDateTime::fromString(child_flowTime, "yyyy-MM-dd hh:mm:ss");
                               int timeGetStamp = time.toTime_t();//获取的时间戳

                               if(i==0 && flowFlag==0){
                                   flowTime=timeGetStamp;
                               }
                            }
                        }

                        //组织流水数据
                        flow_model->setItem(i,0,new QStandardItem(child_regID));
                        char * child_regID_ch;
                        QByteArray child_regID_ba = child_regID.toLatin1();
                        child_regID_ch=child_regID_ba.data();
                        myPtf("child_regID%s\n",child_regID_ch);
                        if(attrib1_flow==1 || attrib1_flow==2 || attrib1_flow==3){
                            flow_model->setItem(i,1,new QStandardItem(child_flowMoney));
                            flow_model->setItem(i,2,new QStandardItem(child_remainMoney));
                            flow_model->setItem(i,3,new QStandardItem(child_secRemainmoney));
                            flow_model->setItem(i,4,new QStandardItem(child_manageFee));
                        }else if(attrib1_flow==6){
                            flow_model->setItem(i,1,new QStandardItem(child_flowMoneys));
                            flow_model->setItem(i,2,new QStandardItem(child_remainMoneys));
                            flow_model->setItem(i,3,new QStandardItem(child_secRemainmoneys));
                            flow_model->setItem(i,4,new QStandardItem(child_manageFees));
                        }
                        flow_model->setItem(i,5,new QStandardItem(child_conTypeID));
                        flow_model->setItem(i,6,new QStandardItem(child_reType));
                        flow_model->setItem(i,7,new QStandardItem(child_flowTime));

                        //设置表格居中显示
                        flow_model->item(i,0)->setTextAlignment(Qt::AlignCenter);
                        flow_model->item(i,1)->setTextAlignment(Qt::AlignCenter);
                        flow_model->item(i,2)->setTextAlignment(Qt::AlignCenter);
                        flow_model->item(i,3)->setTextAlignment(Qt::AlignCenter);
                        flow_model->item(i,4)->setTextAlignment(Qt::AlignCenter);
                        flow_model->item(i,5)->setTextAlignment(Qt::AlignCenter);
                        flow_model->item(i,6)->setTextAlignment(Qt::AlignCenter);
                        flow_model->item(i,7)->setTextAlignment(Qt::AlignCenter);
                    }
                }
           }
       }
    }
}

void flow::refundMoney()
{
    //退款
    QJsonObject jsonsend;
    myPtf("qq-flowID:%d\n",flowID);
    jsonsend.insert("flowID",flowID);
    jsonsend.insert("termCode",termCode_flow);
    jsonsend.insert("termID",termID_flow);
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = UI_EXEC_CANCELFLOW;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_main_flow->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+UI_EXEC_CANCELFLOW){
       //数据获取失败，可以尝试重连
       /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
       message.exec();*/
       ui->label_tips->show();
       ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
       QTimer::singleShot(5000,ui->label_tips,SLOT(hide()));
    }else{
        /*QString strShow = "";
        strShow.append(retJson);
        QMessageBox message(QMessageBox::NoIcon,"提示","json:"+strShow);
        message.exec();*/
        myPtf("qqret:%d\n",jsonrecv.value("ret").toInt());
        if(jsonrecv.value("ret").toInt()==10){
            ui->groupBox_mask->show();
            ui->label_info->setText("退款成功!");
            QTimer::singleShot(1000,ui->groupBox_mask,SLOT(hide()));
            getFlowTotalAndInfo(pageNum);
        }else{
            ui->groupBox_mask->show();
            ui->label_info->setText("退款失败!");
            QTimer::singleShot(1000,ui->groupBox_mask,SLOT(hide()));
        }
    }
}

void flow::on_pushButton_search_clicked()
{
    //查询
    flowFlag=1;
    flag=1;
    pageNum=1;
    getFlowTotalAndInfo(pageNum);
}

void flow::on_pushButton_exit_clicked()
{
    this->close();
}

void flow::on_pushButton_first_clicked()
{
    //首页
    pageNum=1;
    getFlowTotalAndInfo(pageNum);
}

void flow::on_pushButton_pre_clicked()
{
    //上一页
    if(pageNum == 1){
        ui->label_tips->show();
        ui->label_tips->setText("已是首页！");
        QTimer::singleShot(2000,ui->label_tips,SLOT(hide()));
    }else{
        pageNum = pageNum -1;
        getFlowTotalAndInfo(pageNum);
    }
    ui->label_page->setText("页码："+QString::number(pageNum)+"/"+QString::number(pageTotal));
}

void flow::on_pushButton_next_clicked()
{
    //下一页
    sum = pageNum * pageSize;
    if(sum >=page){
        ui->label_tips->show();
        ui->label_tips->setText("已到尾页，没有新数据了！");
        QTimer::singleShot(2000,ui->label_tips,SLOT(hide()));
    }else{
        pageNum = pageNum +1;
        getFlowTotalAndInfo(pageNum);
    }
    ui->label_page->setText("页码："+QString::number(pageNum)+"/"+QString::number(pageTotal));
}

void flow::on_pushButton_final_clicked()
{
    //末页
    pageNum= pageTotal;
    getFlowTotalAndInfo(pageNum);
}

void flow::on_pushButton_startTime_clicked()
{
    dateFlag=1;//开始时间
    setDateTime();
    ui->groupBox_dateTime->show();
}

void flow::on_pushButton_endTime_clicked()
{
    dateFlag=2;//结束时间
    setDateTime();
    ui->groupBox_dateTime->show();
}

void flow::on_pushButton_submit_clicked()
{
    //时间控件--确定
    year=ui->year->text().toInt();
    month=ui->month->text().toInt();
    day=ui->day->text().toInt();
    hour=ui->hour->text().toInt();
    minute=ui->minute->text().toInt();
    second=ui->second->text().toInt();
    QString dateYear;
    QString dateMonth;
    QString dateDay;
    QString dateHour;
    QString dateMinute;
    QString dateSecond;

    dateYear=QString::number(year);

    if(month<10){
        dateMonth="0"+QString::number(month);
    }else{
        dateMonth=QString::number(month);
    }

    if(day<10){
        dateDay="0"+QString::number(day);
    }else{
        dateDay=QString::number(day);
    }

    if(hour<10){
        dateHour="0"+QString::number(hour);
    }else{
        dateHour=QString::number(hour);
    }

    if(minute<10){
        dateMinute="0"+QString::number(minute);
    }else{
        dateMinute=QString::number(minute);
    }

    if(second<10){
        dateSecond="0"+QString::number(second);
    }else{
        dateSecond=QString::number(second);
    }

    dateTimes = dateYear +"-"+ dateMonth +"-"+ dateDay +" "+ dateHour +":"+ dateMinute +":"+ dateSecond;
    if(dateFlag==1){
        ui->pushButton_startTime->setText(dateTimes);
    }else if(dateFlag==2){
        ui->pushButton_endTime->setText(dateTimes);
    }
    ui->groupBox_dateTime->hide();
}

void flow::on_pushButton_cancel_clicked()
{
    //时间控件--取消
    ui->groupBox_dateTime->hide();
}

void flow::on_year_add_clicked()
{
    year = ui->year->text().toInt();
    year=year+1;
    ui->year->setText(QString::number(year));
}

void flow::on_year_del_clicked()
{
    year = ui->year->text().toInt();
    year=year-1;
    if(year<1){
        year=1;
    }
    ui->year->setText(QString::number(year));
}

void flow::on_month_add_clicked()
{
    month = ui->month->text().toInt();
    month=month+1;
    if(month>12){
        month=12;
    }
    ui->month->setText(QString::number(month));
}

void flow::on_month_del_clicked()
{
    month = ui->month->text().toInt();
    month=month-1;
    if(month<1){
        month=1;
    }
    ui->month->setText(QString::number(month));
}

void flow::on_day_add_clicked()
{
    year = ui->year->text().toInt();
    month = ui->month->text().toInt();
    day = ui->day->text().toInt();
    if(((year % 4 == 0)&&(year % 100 != 0)) || (year % 400==0)){
       if(month ==2){
           if(day>=29){
               day=29;
               ui->day->setText(QString::number(day));
           }else{
               day=day+1;
               ui->day->setText(QString::number(day));
           }
       }else if(month== 4 || month==6 || month==9 || month==11){
           if(day>=30){
               day=30;
               ui->day->setText(QString::number(day));
           }else{
               day=day+1;
               ui->day->setText(QString::number(day));
           }
       }else{
           if(day>=31){
               day=31;
               ui->day->setText(QString::number(day));
           }else{
               day=day+1;
               ui->day->setText(QString::number(day));
           }
       }
    }else{
        if(month ==2){
            if(day>=28){
                day=28;
                ui->day->setText(QString::number(day));
            }else{
                day=day+1;
                ui->day->setText(QString::number(day));
            }
        }else if(month== 4 || month==6 || month==9 || month==11){
            if(day>=30){
                day=30;
                ui->day->setText(QString::number(day));
            }else{
                day=day+1;
                ui->day->setText(QString::number(day));
            }
        }else{
            if(day>=31){
                day=31;
                ui->day->setText(QString::number(day));
            }else{
                day=day+1;
                ui->day->setText(QString::number(day));
            }
        }
    }
}

void flow::on_day_del_clicked()
{
    day = ui->day->text().toInt();
    day=day-1;
    if(day<1){
        day=1;
    }
    ui->day->setText(QString::number(day));
}

void flow::on_hour_add_clicked()
{
    hour = ui->hour->text().toInt();
    hour=hour+1;
    if(hour>23){
        hour=23;
    }
    ui->hour->setText(QString::number(hour));
}

void flow::on_hour_del_clicked()
{
    hour = ui->hour->text().toInt();
    hour=hour-1;
    if(hour<0){
        hour=0;
    }
    ui->hour->setText(QString::number(hour));
}

void flow::on_minute_add_clicked()
{
    minute = ui->minute->text().toInt();
    minute=minute+1;
    if(minute>59){
        minute=59;
    }
    ui->minute->setText(QString::number(minute));
}

void flow::on_minite_del_clicked()
{
    minute = ui->minute->text().toInt();
    minute=minute-1;
    if(minute<0){
        minute=0;
    }
    ui->minute->setText(QString::number(minute));
}

void flow::on_second_add_clicked()
{
    second = ui->second->text().toInt();
    second=second+1;
    if(second>59){
        second=59;
    }
    ui->second->setText(QString::number(second));
}

void flow::on_second_del_clicked()
{
    second = ui->second->text().toInt();
    second=second-1;
    if(second<0){
        second=0;
    }
    ui->second->setText(QString::number(second));
}

void flow::on_pushButton_preDay_clicked()
{
    //查看前三天流水
    flowFlag=1;
    flag=2;
    pageNum=1;
    getFlowTotalAndInfo(pageNum);
}

void flow::on_pushButton_preWeek_clicked()
{
    //查看前一周流水
    flowFlag=1;
    flag=3;
    pageNum=1;
    getFlowTotalAndInfo(pageNum);
}

void flow::on_pushButton_preMonth_clicked()
{
    //查看前一月流水
    flowFlag=1;
    flag=4;
    pageNum=1;
    getFlowTotalAndInfo(pageNum);
}

void flow::on_pushButton_refundMoney_clicked()
{
    //退款
    QDateTime time = QDateTime::currentDateTime();//获取系统现在的时间
    int timeNowStamp = time.toTime_t();//当前的时间戳
    int timeDiffer = timeNowStamp - flowTime;
    myPtf("timeNowStamp:%d\n",timeNowStamp);
    myPtf("flowTime:%d\n",flowTime);
    myPtf("timeDiffer:%d\n",timeDiffer);
    if(flowFlag==0){
        if(timeDiffer <= 10*60){
            if(flowID==0){
                ui->groupBox_mask->show();
                ui->label_info->setText("脱机模式下的消费流水不能进行退款操作!");
                QTimer::singleShot(2000,ui->groupBox_mask,SLOT(hide()));
            }else{
                ui->groupBox_login->show();
                ui->lineEdit_password->setFocus();
            }

        }else{
            ui->groupBox_mask->show();
            ui->label_info->setText("流水已过10分钟，不可以退款!");
            QTimer::singleShot(2000,ui->groupBox_mask,SLOT(hide()));
        }
    }else{
        ui->groupBox_mask->show();
        ui->label_info->setText("当前界面流水记录没有可以退款的流水!");
        QTimer::singleShot(2000,ui->groupBox_mask,SLOT(hide()));
    }
}

void flow::on_pushButton_login_clicked()
{
    //退款-确定
    QString pwd = ui->lineEdit_password->text();
    if(pwd ==""){
        ui->label_erroMes->show();
        ui->label_erroMes->setText("请输入密码!");
        QTimer::singleShot(2000,ui->label_erroMes,SLOT(hide()));
    }else if(pwd !=termPassWord_flow){
        ui->label_erroMes->show();
        ui->label_erroMes->setText("密码错误!");
        QTimer::singleShot(2000,ui->label_erroMes,SLOT(hide()));
    }else{
        refundMoney();
        ui->groupBox_login->hide();
        ui->lineEdit_password->setText("");
    }
}

void flow::on_pushButton_back_clicked()
{
    //退款-取消
    ui->groupBox_login->hide();
    ui->lineEdit_password->setText("");
}

bool flow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched==ui->lineEdit_password){//首先判断控件(这里指lineEdit_password)
        if(event->type()==QEvent::KeyPress){//判断控件的具体事件(这里指获得点击事件)
            QKeyEvent *keyevent=static_cast<QKeyEvent *>(event);
            if(keyevent->isAutoRepeat()){
                ui->lineEdit_password->setText(ui->lineEdit_password->text());
                return true;
            }

            int keyValue = keyevent->key();
            myPtf("qqkeyValue:%d\n",keyValue);
            if(keyValue==16777264){//16777264--小数点
                ui->lineEdit_password->setText(ui->lineEdit_password->text().trimmed()+".");
                return true;
            }else if(keyValue==16777216 || keyValue==16777265){//16777216--取消键;16777265--菜单键
                ui->lineEdit_password->setText(ui->lineEdit_password->text());
                return true;
            }else if(keyValue==42){//42--返回键，定义为退格键
                ui->lineEdit_password->backspace();
                return true;
            }else if(keyValue==16777220){//16777220--确定键
                on_pushButton_login_clicked();
                return true;
            }else{
                 return QWidget::eventFilter(watched,event);     // 最后将事件交给上层对话框
            }
        }
    }
    return QWidget::eventFilter(watched,event);     // 最后将事件交给上层对话框
}

void flow::getDownloadNum(){
    //获取下载流水包数
    QJsonObject jsonsend;
    jsonsend.insert("termCode",termCode_flow);
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = UI_GET_USERLISTNUM;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_main_flow->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+UI_GET_USERLISTNUM){
       //数据获取失败，可以尝试重连
       /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
       message.exec();*/
       ui->label_tips->show();
       ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
       QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else{
        myPtf("getDownloadNum-state--:%d\n",jsonrecv.value("state").toInt());
        if(jsonrecv.value("state").toInt()==1){
            userPackCount = jsonrecv.value("UserPackCount").toInt();
            myPtf("userPackCount--:%d\n",userPackCount);
            if(userPackCount >0){

                ui->label_process->setText("正在下载，当前第"+QString::number(userPackCount)+"包！");
                ui->pushButton_confirm_cancel->show();
                ui->pushButton_confirm_upload->show();

                /*if(flowFlags==0){
                    ui->groupBox_flowBg->show();
                    flowFlags=1;
                }*/
            }else if(userPackCount ==0){
                ui->label_process->setText("下载结束！");
                downloadFlag=0;
                QTimer::singleShot(1000,this,SLOT(clearUploadPanel()));
            }else{
                ui->label_process->setText("下载错误！");
                downloadFlag=0;
                QTimer::singleShot(1000,this,SLOT(clearUploadPanel()));
            }

        }else{
            //state=0:当前没有上传
            ui->label_process->setText("当前没有上传");
            QTimer::singleShot(2000,this,SLOT(clearUploadPanel()));
        }
    }
}
void flow::getOffLineDownloaFlowNum()
{
    timerDownloadFlowNum->stop();
    getDownloadNum();
}
int flow::downloadFlow(){
    //下载流水
    QJsonObject jsonsend;
    jsonsend.insert("termCode",termCode_flow);
    //发送数据
    QJsonObject jsonrecv;
    int Cmd = UI_REQ_DOWNLOADUSERLIST;
    char retJson[1024];
    memset(retJson,0,1024);
    jsonrecv = scmd_main_flow->send_Command(&Cmd,jsonsend,retJson);
    if(Cmd != 10000+UI_REQ_DOWNLOADUSERLIST){
       //数据获取失败，可以尝试重连
       /*QMessageBox message(QMessageBox::NoIcon,"提示","发送数据失败!,ret是"+QString::number(Cmd));
       message.exec();*/
       ui->label_tips->show();
       ui->label_tips->setText("发送数据失败!,ret是"+QString::number(Cmd));
       QTimer::singleShot(3000,ui->label_tips,SLOT(hide()));
    }else{

        int state = jsonrecv.value("state").toInt();
        myPtf("downloadFlow-state--:%d\n",state);
        return state;
    }
}
void flow::clearUploadPanel()
{
    ui->groupBox_flowBg->hide();
    ui->label_process->setText("");
    ui->label_tips_process->setText("");
    //ui->pushButton_confirm_cancel->hide();
    //ui->pushButton_confirm_upload->hide();
    ui->pushButton_confirm_upload->setEnabled(true);
    //flowFlags=0;
}

void flow::on_pushButton_download_clicked()
{
    ui->groupBox_flowBg->show();

}

void flow::on_pushButton_confirm_upload_clicked()
{
    if(downloadFlow() == 1){
        timerDownloadFlowNum->start(100);
        downloadFlag=1;
    }downloadFlow();


}

void flow::on_pushButton_confirm_cancel_clicked()
{
    downloadFlag=0;
    //userPackCount=0;
    ui->groupBox_flowBg->hide();
    ui->pushButton_confirm_upload->setEnabled(true);
}
