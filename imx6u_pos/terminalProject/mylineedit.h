#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H
#include <QLineEdit>

class MyLineEdit
{
public:
    MyLineEdit();
protected:
       virtual void focusInEvent(QFocusEvent *e);
       virtual void focusOutEvent(QFocusEvent *e);
};

#endif // MYLINEEDIT_H
