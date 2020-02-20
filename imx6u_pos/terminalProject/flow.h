#ifndef FLOW_H
#define FLOW_H

#include <QDialog>
#include "sendcmd.h"
//#include "common.h"
#include <QTimer>
#include <QDateTime>
#include <QStandardItemModel>
#include <QKeyEvent>
#include <QObject>
#include <QEvent>
namespace Ui {
class flow;
}

static int AllowedOfflineConsume = 1;   //允许脱机交易

class flow : public QDialog
{
    Q_OBJECT

public:
    explicit flow(QWidget *parent = 0);
    ~flow();
    //sendCmd * scmd_network_flow;
    sendCmd * scmd_main_flow;
    QString termCode_flow;//终端唯一识别号
    QString termPassWord_flow;//密码
    int termNo_flow;//终端号
    int termID_flow;
    int attrib1_flow;//终端工作模式
    int ret_flow;//服务连接状态

private slots:
    void flowInit();//获取页面信息

    void setDateTime();

    void on_pushButton_search_clicked();

    void on_pushButton_exit_clicked();

    void on_pushButton_first_clicked();

    void on_pushButton_pre_clicked();

    void on_pushButton_next_clicked();

    void on_pushButton_final_clicked();

    void on_pushButton_startTime_clicked();

    void on_pushButton_endTime_clicked();

    void on_pushButton_submit_clicked();

    void on_pushButton_cancel_clicked();

    void on_year_add_clicked();

    void on_year_del_clicked();

    void on_month_add_clicked();

    void on_month_del_clicked();

    void on_day_add_clicked();

    void on_day_del_clicked();

    void on_hour_add_clicked();

    void on_hour_del_clicked();

    void on_minute_add_clicked();

    void on_minite_del_clicked();

    void on_second_add_clicked();

    void on_second_del_clicked();

    void on_pushButton_preDay_clicked();

    void on_pushButton_preWeek_clicked();

    void on_pushButton_preMonth_clicked();

    void on_pushButton_refundMoney_clicked();

    void on_pushButton_login_clicked();

    void on_pushButton_back_clicked();

    bool eventFilter(QObject *watched, QEvent *event);

    void clearUploadPanel();//清空上传面板

    void on_pushButton_download_clicked();

    void on_pushButton_confirm_upload_clicked();

    void on_pushButton_confirm_cancel_clicked();

    void getOffLineDownloaFlowNum();//获取下载流水包数(实时)

private:
    Ui::flow *ui;
    int initFlag=0;//页面信息初始化标志，0：只调取一次，1：不再执行
    QTimer * timerFlow;//定时获取页面信息
    //common * myCommon;
    void init();//整体--初始化函数
    //void getNetworkConnectStatus();//获取网络连接状态
    void getFlowTotalAndInfo(int pageNum);//获取流水总金额和总条数--获取流水明细
    void refundMoney();//退款

    void getDownloadNum();//获取下载流水包数

    int downloadFlow();//下载流水

    int userPackCount;//尚未上传的脱机流水条数

    QTimer * timerDownloadFlowNum;//获取下载条数定时器
    int downloadFlag;//点击“上传”按钮标志，1:开启
    //int flowFlags;//开机时是否显示上传流水的弹出框，为1时就不显示了

    QStandardItemModel * flow_model;//准备数据模型
    int pageNum=1;//页码
    int pageSize = 5;//每页显示条数
    int count;//流水总数
    int page;
    int flag=0;//0:默认时间,1:时间从控件获取,2:前三天,3:前七天,4:前30天
    int pageTotal;//总页数
    int sum = 0;//总金额或总笔数
    int dateFlag=0;//0:默认,1:开始时间,2:结束时间
    int year;//年
    int month;//月
    int day;//日
    int hour;//时
    int minute;//分
    int second;//秒
    QString dateTimes;//日期时间

    int flowID;//流水ID
    int flowTime;//流水时间
    int flowFlag;//进入到流水查询页面时，获得第一条流水的flowID；选择时间段查询后，将其设置为1，在flowFlag=1的时候不可将新的第一条流水的flowID存入变量flowID中

};

#endif // FLOW_H
