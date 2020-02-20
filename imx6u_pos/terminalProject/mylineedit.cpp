#include "mylineedit.h"
#include "qmessagebox.h"
MyLineEdit::MyLineEdit()
{

}
void MyLineEdit::focusInEvent(QFocusEvent *e)
{
    QMessageBox message(QMessageBox::NoIcon,"提示","11");
    message.exec();
}

void MyLineEdit::focusOutEvent(QFocusEvent *e)
{
    QMessageBox message(QMessageBox::NoIcon,"提示","22");
    message.exec();
}
