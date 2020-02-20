#ifndef SIGNALSENDER_H
#define SIGNALSENDER_H

#include <QObject>

#define MSGBUFFLEN 1024

class SignalSender:public QObject
{
    Q_OBJECT
private:
    char retstring[MSGBUFFLEN];

public:
    SignalSender(QObject* parent=0):QObject(parent) {};
    void callEmitSignal(char * strvalue)
    {
        memcpy(retstring,strvalue,MSGBUFFLEN);
        emit newMsgSignal(retstring);
    }

signals:
    void newMsgSignal(char * value);

};


#endif // SIGNALSENDER_H
