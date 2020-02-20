#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include "test.h"

//传结构
typedef struct
{
    QString ip;
    QString netmask;
}netstruct;
namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
    sendCmd * scmd_network_login;
    QString num;//传参数
    netstruct net;
private slots:
    void on_pushButton_clicked();


private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
