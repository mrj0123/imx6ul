#-------------------------------------------------
#
# Project created by QtCreator 2018-09-21T09:31:42
#
#-------------------------------------------------

QT       += core gui
QT += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = terminalProject
TEMPLATE = app
# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    flow.cpp \
    network.cpp \
    login.cpp \
    afunix_udp.cpp \
    sendcmd.cpp \
    sockfile.cpp \
    common.cpp \
    secscreen.cpp \
    hardware.cpp \
    exqrscan.cpp \
    netCommon.cpp \
    getnetsetting.cpp \
    consumecallbackthread.cpp \
    getnetsettingthread.cpp \
    cglobal.cpp



HEADERS += \
        mainwindow.h \
    flow.h \
    network.h \
    login.h \
    a.h \
    afunix_udp.h \
    sendcmd.h \
    sockfile.h \
    common.h \
    secscreen.h \
    hardware.h \
    exqrscan.h \
    signalsender.h \
    netCommon.h \
    getnetsetting.h \
    consumecallbackthread.h \
    getnetsettingthread.h \
    b.h \
    cglobal.h



FORMS += \
        mainwindow.ui \
    flow.ui \
    network.ui \
    login.ui \
    dialog.ui

RESOURCES += \
    resouce.qrc


INCLUDEPATH += /opt/fsl-imx-wayland/4.1.15-2.0.1/sysroots/cortexa7hf-neon-poky-linux-gnueabi/usr/include/c++/5.3.0/
INCLUDEPATH += /opt/fsl-imx-wayland/4.1.15-2.0.1/sysroots/cortexa7hf-neon-poky-linux-gnueabi/usr/include/c++/5.3.0/arm-poky-linux-gnueabi/
INCLUDEPATH += /opt/fsl-imx-wayland/4.1.15-2.0.1/sysroots/cortexa7hf-neon-poky-linux-gnueabi/usr/include/
DEFINES += __ARM_PCS_VFP QT_NO_OPENGL
