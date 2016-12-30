#include "serverdialog.h"
#include "ui_serverdialog.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>

ServerDialog::ServerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerDialog)
{
    ui->setupUi(this);
    isOK = false;
    ui->serverOkButton->setEnabled(false);
}

ServerDialog::~ServerDialog()
{
    delete ui;
}

bool ServerDialog::IsConnection(QString server)
{
    //发送put请求 同时也生成一个空的migration文件
    QString url = server + "/migration";
    QNetworkAccessManager *manger = new QNetworkAccessManager();
    QNetworkRequest request;
    request.setUrl(url);
    QNetworkReply *reply = manger->put(request,QByteArray("[]",2));
    QEventLoop loop;
    connect(reply,SIGNAL(finished()),&loop,SLOT(quit()));
    loop.exec();
    bool isok = false;
    if(reply->error()==QNetworkReply::NoError)
        isok = true;
    reply->deleteLater();
    reply->close();
    return isok;
}

void ServerDialog::SetServer(QString server)
{
    ui->serverLineEdit->setText(server);
}

void ServerDialog::on_serverOkButton_clicked()
{  
    isOK = true;
    this->close();
}

void ServerDialog::on_testConButton_clicked()
{
    serverName = ui->serverLineEdit->text().trimmed();
    ui->testConButton->setEnabled(false);
    if(IsConnection(serverName))
        ui->serverOkButton->setEnabled(true);
    ui->testConButton->setEnabled(true);
}

void ServerDialog::on_serverLineEdit_textChanged(const QString &arg1)
{
    ui->serverOkButton->setEnabled(false);
}
