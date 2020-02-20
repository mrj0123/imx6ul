#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include "sendcmd.h"
#include "network.h"
#include <QKeyEvent>
#include <QObject>
#include <QEvent>
namespace Ui {
class login;
}

class login : public QDialog
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = 0);
    ~login();
    sendCmd * scmd_network_login;
    sendCmd * scmd_main_login;
    sendCmd * scmd_sec_login;

    QString termCode_login;//终端唯一识别号
    QMediaPlayer * players_login;

    QString uiVersion_login;//UI版本号    
    QString consumeserVersion_login;//获取消费服务版本号，consumeser服务
    QString scanqrserVersion_login;//获取版本信息,scanqrser服务
    QString secscreenVersion_login;//获取版本号,secscreen服务
    QString networkVersion_login;//获取版本号,network服务

private slots:
    void on_pushButton_login_clicked();

    void on_pushButton_back_clicked();

    bool eventFilter(QObject *watched, QEvent *event);
private:
    Ui::login *ui;
    void loginReg();//登录验证
};

#endif // LOGIN_H
