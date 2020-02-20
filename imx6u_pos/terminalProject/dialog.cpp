#include "dialog.h"
#include "ui_dialog.h"
#include "qmessagebox.h"
Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

}

Dialog::~Dialog()
{
    delete ui;
}
void Dialog::on_pushButton_clicked()
{

    test * myTest = new test();
    myTest->scmdTest=scmd_network_login;
    myTest->getTestDate();

    QMessageBox message(QMessageBox::NoIcon,"提示","num:"+num);
    message.exec();

    QMessageBox messages(QMessageBox::NoIcon,"提示","ip:"+net.ip);
    messages.exec();
}
