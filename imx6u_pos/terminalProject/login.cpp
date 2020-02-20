#include "login.h"
#include "ui_login.h"
#include "qmessagebox.h"
#include <QTimer>
login::login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    ui->lineEdit_password_login->setEchoMode(QLineEdit::Password);//设置输入密码框
    ui->label_erroNote->hide();//错误提示框

    ui->lineEdit_password_login->installEventFilter(this);//在窗体上为lineEdit安装过滤器
    //全屏
    setWindowFlags(Qt::FramelessWindowHint);
    showFullScreen();
    ui->lineEdit_password_login->setFocus();
}

login::~login()
{
    delete ui;
}


void login::loginReg()
{
    //登录验证
    QString pwd = ui->lineEdit_password_login->text();
    if(pwd ==""){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","请输入密码!");
        message.exec();*/
        ui->label_erroNote->show();
        ui->label_erroNote->setText("请输入密码!");
        QTimer::singleShot(2000,ui->label_erroNote,SLOT(hide()));
    }else if(pwd !="123456"){
        /*QMessageBox message(QMessageBox::NoIcon,"提示","密码错误!");
        message.exec();*/
        ui->label_erroNote->show();
        ui->label_erroNote->setText("密码错误!");
        QTimer::singleShot(2000,ui->label_erroNote,SLOT(hide()));
    }else{
        //设置网络
        network * myNetwork = new network(this);
        myNetwork->scmd_network_network=scmd_network_login;
        myNetwork->scmd_main_network=scmd_main_login;
        myNetwork->scmd_sec_network=scmd_sec_login;

        myNetwork->termCode_network=termCode_login;
        myNetwork->players_network=players_login;
        //版本号
        myNetwork->uiVersion_network=uiVersion_login;        
        myNetwork->consumeserVersion_network=consumeserVersion_login;
        myNetwork->scanqrserVersion_network=scanqrserVersion_login;
        myNetwork->secscreenVersion_network=secscreenVersion_login;
        myNetwork->networkVersion_network=networkVersion_login;

        myNetwork->exec();
        delete myNetwork;
        this->close();
    }
}
void login::on_pushButton_login_clicked()
{
    loginReg();
}

void login::on_pushButton_back_clicked()
{
    this->close();
}
bool login::eventFilter(QObject *watched, QEvent *event)
{
    if(watched==ui->lineEdit_password_login){//首先判断控件(这里指lineEdit_password)
        if(event->type()==QEvent::KeyPress){//判断控件的具体事件(这里指获得点击事件)
            QKeyEvent *keyevent=static_cast<QKeyEvent *>(event);
            if(keyevent->isAutoRepeat()){
                ui->lineEdit_password_login->setText(ui->lineEdit_password_login->text());
                return true;
            }

            int keyValue = keyevent->key();
            printf("qqkeyValue:%d\n",keyValue);
            if(keyValue==16777264){//16777264--小数点
                ui->lineEdit_password_login->setText(ui->lineEdit_password_login->text().trimmed()+".");
                return true;
            }else if(keyValue==16777216 || keyValue==16777265){//16777216--取消键;16777265--菜单键
                ui->lineEdit_password_login->setText(ui->lineEdit_password_login->text());
                return true;
            }else if(keyValue==42){//42--返回键，定义为退格键
                ui->lineEdit_password_login->backspace();
                return true;
            }else if(keyValue==16777220){//16777220--确定键
                loginReg();
                return true;
            }else{
                 return QWidget::eventFilter(watched,event);     // 最后将事件交给上层对话框
            }
        }
    }
    return QWidget::eventFilter(watched,event);     // 最后将事件交给上层对话框
}
