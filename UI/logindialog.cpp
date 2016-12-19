#include "logindialog.h"
#include "ui_logindialog.h"
#include <CloudSDK/aliyunclient.h>
#include <CloudSDK/azureclient.h>
#include <CloudSDK/baiduyunclient.h>
#include <CloudSDK/googleclient.h>
#include <CloudSDK/s3client.h>
#include <QFileDialog>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
}

LoginDialog::LoginDialog(QWidget *parent,QString cloudName) :
    QDialog(parent),cloudName(cloudName),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    this->setWindowTitle(cloudName);
    Init();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::Init()
{
    if(cloudName=="google")
    {
        ui->stackedWidget->setCurrentWidget(ui->googleLoginPage);
    }
    else if(cloudName=="azure")
    {
        ui->secertIdLabel->setText("SharedKey:");
        ui->secertKeyLabel->setText("StorageAccount:");
    }
    else if(cloudName=="s3")
    {
        ui->secertIdLabel->setText("AccessKeyID:");
        ui->secertKeyLabel->setText("SecretAccessKey:");
    }
    else if(cloudName=="baiduyun")
    {
        ui->secertIdLabel->setText("AccessKeyID:");
        ui->secertKeyLabel->setText("SecretAccessKey:");
    }
    else if(cloudName=="aliyun")
    {
        ui->secertIdLabel->setText("AccessKeyID:");
        ui->secertKeyLabel->setText("AccessKeySecret:");
    }
}

bool LoginDialog::CheckAccount()
{
    QNetworkAccessManager *manger = new QNetworkAccessManager;
    if(cloudName=="google")
    {
        QString jsonPath = ui->jsonLabel->text();
        if(jsonPath.isEmpty())
            return false;
        GoogleClient *google = new GoogleClient(manger,jsonPath.toStdString().data());
        return google->Login();
    }
    else
    {
        QString accessKeyID = ui->secertIdLineEdit->text();
        QString secertaccesskey = ui->secertKeyLineEdit->text();
        if(accessKeyID.isEmpty()||secertaccesskey.isEmpty())
            return false;
        if(cloudName=="aliyun")
        {
            AliyunClient *aliyun = new AliyunClient(manger,accessKeyID,secertaccesskey);
            return aliyun->Login();
        }
        else if(cloudName=="baiduyun")
        {
            BaiduyunClient *baiduyun = new BaiduyunClient(manger,accessKeyID,secertaccesskey);
            return baiduyun->Login();
        }
        else if(cloudName=="s3")
        {
            S3Client *s3 = new S3Client(manger,accessKeyID,secertaccesskey);
            return s3->Login();
        }
        else if(cloudName=="azure")
        {
            AzureClient *azure = new AzureClient(manger,accessKeyID,secertaccesskey);
            return azure->Login();
        }
    }
    return false;
}

void LoginDialog::on_nextButton_1_clicked()
{
    //检查secertid,secertkey是否正确
    ui->errorLabel->setText("");
    if(CheckAccount())
    {
        ui->stackedWidget->setCurrentWidget(ui->bucketPage);
    }
    else
        ui->errorLabel->setText("Configuration error!");
}

void LoginDialog::on_preButton_clicked()
{
    if(cloudName=="google")
        ui->stackedWidget->setCurrentWidget(ui->googleLoginPage);
    else
        ui->stackedWidget->setCurrentWidget(ui->loginPage);
}

void LoginDialog::on_nextButton_2_clicked()
{
    CloudInfo cloud;
    if(cloudName=="google")
        cloud.account = ui->accountLineEdit_1->text();
    else
        cloud.account = ui->accountLineEdit->text();
    if(cloud.account.isEmpty())
        cloud.account = "noname@mail.com";
    cloud.addTime = QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate);
    cloud.cloudName = cloudName;
    if(cloudName=="google")
    {
        cloud.certificate = ui->jsonLabel->text();
    }
    else
    {
        cloud.certificate = ui->secertIdLineEdit->text() + '|' + ui->secertKeyLineEdit->text();
    }
    cloud.defaultBucket = ui->bucketLineEdit->text();
    emit AddCloud(cloud);
}

void LoginDialog::on_importButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
           this,
           "Load json file",
           QDir::currentPath(),"JSON (*.json)");
    if(!fileName.isEmpty())
        ui->jsonLabel->setText(fileName);
}

void LoginDialog::on_nextButton_3_clicked()
{
    //google cloud
    ui->errorLabel_3->setText("");
    if(CheckAccount())
    {
        ui->stackedWidget->setCurrentWidget(ui->bucketPage);
    }
    else
        ui->errorLabel_3->setText("Configuration error!");
}