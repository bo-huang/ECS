#include "logindialog.h"
#include "ui_logindialog.h"
#include <CloudSDK/aliyunclient.h>
#include <CloudSDK/azureclient.h>
#include <CloudSDK/baiduyunclient.h>
#include <CloudSDK/googleclient.h>
#include <CloudSDK/s3client.h>
#include <QFileDialog>
#include <QDateTime>

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

void LoginDialog::SetBuckets(std::vector<Bucket> &buckets)
{
    this->buckets = buckets;
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

CloudClient* LoginDialog::CreateCloudClient()
{
    QNetworkAccessManager *manger = new QNetworkAccessManager;
    if(cloudName == "google")
    {
        QString jsonPath = ui->jsonLabel->text();
        if(jsonPath.isEmpty())
            return NULL;
        return new GoogleClient(manger,jsonPath.toStdString().data());
    }
    else
    {
        QString accessKeyID = ui->secertIdLineEdit->text();
        QString secertaccesskey = ui->secertKeyLineEdit->text();
        if(accessKeyID.isEmpty()||secertaccesskey.isEmpty())
            return NULL;
        if(cloudName == "aliyun")
        {
            return new AliyunClient(manger,accessKeyID,secertaccesskey);
        }
        else if(cloudName=="baiduyun")
        {
            return new BaiduyunClient(manger,accessKeyID,secertaccesskey);
        }
        else if(cloudName=="s3")
        {
            return new S3Client(manger,accessKeyID,secertaccesskey);
        }
        else if(cloudName=="azure")
        {
            return new AzureClient(manger,accessKeyID,secertaccesskey);
        }
        else
            return NULL;
    }
}

bool LoginDialog::CheckAccount()
{
    CloudClient *client = CreateCloudClient();
    if(client==NULL)
        return false;
    return client->Login();
}

bool LoginDialog::CreateDefaultBucket()
{
    defaultBucket = "unicloud"+QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    CloudClient *client = CreateCloudClient();
    if(client==NULL)
        return false;
    return client->CreateBucket(defaultBucket,"Asia","STANDARD");
}

bool LoginDialog::CreateUserBucket()
{
    CloudClient *client = CreateCloudClient();
    if(client==NULL)
        return false;
    for(int i=0;i<buckets.size();++i)
    {
        Bucket bucket = buckets[i];
        client->CreateBucket(bucket.bucketName,bucket.region,bucket.storageClass);
    }
    return true;
}

void LoginDialog::on_nextButton_1_clicked()
{
    //检查secertid,secertkey是否正确
    ui->errorLabel->setText("Authorizing……");
    if(CheckAccount())
    {
        //ui->stackedWidget->setCurrentWidget(ui->bucketPage);
        ui->errorLabel->setText("Initialize cloud……");
        if(CreateDefaultBucket())
        {
            CreateUserBucket();
            ui->errorLabel->setText("Initializion completed");
            //添加云
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
            cloud.defaultBucket = defaultBucket;
            emit AddCloud(cloud);
        }
        else
            ui->errorLabel->setText("Initializion error");
    }
    else
        ui->errorLabel->setText("Authorization error!");
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
    ui->errorLabel_3->setText("Authorizing……");
    if(CheckAccount())
    {
        //ui->stackedWidget->setCurrentWidget(ui->bucketPage);
        ui->errorLabel_3->setText("Initialize cloud……");
        if(CreateDefaultBucket())
        {
            ui->errorLabel_3->setText("Initializion completed");

            //添加云
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
            cloud.defaultBucket = defaultBucket;
            emit AddCloud(cloud);
        }
        else
            ui->errorLabel_3->setText("Initializion error");
    }
    else
        ui->errorLabel_3->setText("Authorizing error!");
}
