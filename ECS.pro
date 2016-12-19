#-------------------------------------------------
#
# Project created by QtCreator 2016-10-10T20:31:18
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ECS
TEMPLATE = app
#设置程序图标
RC_FILE = qrc/icon.rc

SOURCES += main.cpp\
        mainwindow.cpp \
    CLASS/block.cpp \
    CLASS/bucket.cpp \
    CLASS/segment.cpp \
    CLASS/snapshot.cpp \
    CloudSDK/aliyunclient.cpp \
    CloudSDK/azureclient.cpp \
    CloudSDK/cloudclient.cpp \
    CloudSDK/googleclient.cpp \
    CloudSDK/s3client.cpp \
    CORE/coder.cpp \
    CORE/datatransfer.cpp \
    CORE/downloadcontrol.cpp \
    CORE/uploadcontrol.cpp \
    UI/createbucketdialog.cpp \
    UI/uploaddialog.cpp \
    CLASS/cloudinfo.cpp \
    CLASS/operation.cpp \
    CLASS/log.cpp \
    CLASS/progressbardelegation.cpp \
    CLASS/uploadassist.cpp \
    CloudSDK/baiduyunclient.cpp \
    UI/movefiledialog.cpp \
    CORE/movecontrol.cpp \
    UI/cloudswindow.cpp \
    UI/logindialog.cpp \
    CLASS/jwt.cpp \
    CLASS/systemtime.cpp \
    CORE/deletecontrol.cpp

HEADERS  += mainwindow.h \
    CLASS/block.h \
    CLASS/bucket.h \
    CLASS/segment.h \
    CLASS/snapshot.h \
    CloudSDK/aliyunclient.h \
    CloudSDK/azureclient.h \
    CloudSDK/cloudclient.h \
    CloudSDK/googleclient.h \
    CloudSDK/s3client.h \
    CORE/coder.h \
    CORE/datatransfer.h \
    CORE/downloadcontrol.h \
    CORE/uploadcontrol.h \
    UI/createbucketdialog.h \
    UI/uploaddialog.h \
    CLASS/cloudinfo.h \
    CLASS/operation.h \
    CLASS/log.h \
    CLASS/progressbardelegation.h \
    CLASS/uploadassist.h \
    CloudSDK/baiduyunclient.h \
    UI/movefiledialog.h \
    CORE/movecontrol.h \
    UI/cloudswindow.h \
    UI/logindialog.h \
    CLASS/jwt.h \
    CLASS/systemtime.h \
    CORE/deletecontrol.h

FORMS    += mainwindow.ui \
    UI/createbucketdialog.ui \
    UI/uploaddialog.ui \
    UI/movefiledialog.ui \
    UI/cloudswindow.ui \
    UI/logindialog.ui

RESOURCES += \
    qrc/images.qrc
#引入第三方库计算JWT
LIBS += -L"C:\jwt" -llibchilkat-9.5.0
INCLUDEPATH += $$quote(C:\jwt/include)
