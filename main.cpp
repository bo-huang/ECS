#include "mainwindow.h"
#include <QApplication>
#include <QNetworkProxy>

#define MYDEBUG

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //注册代理，方便fiddler调试(必须先打开fiddler！)
    #ifdef MYDEBUG
    QNetworkProxy proxy;
    proxy.setType(QNetworkProxy::HttpProxy);
    proxy.setHostName("127.0.0.1");
    proxy.setPort(8888);
    QNetworkProxy::setApplicationProxy(proxy);
    #endif

    MainWindow w;
    w.show();
    return a.exec();
}
