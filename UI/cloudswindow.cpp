#include "cloudswindow.h"
#include "ui_cloudswindow.h"
#include <QDateTime>

CloudsWindow::CloudsWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CloudsWindow)
{
    ui->setupUi(this);
    //init listcloud
    listcloud = new QListWidget(this);
    connect(listcloud,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(listcloudItemClicked(QListWidgetItem*)));
    listcloud->hide();
    listcloud->setMinimumSize(145,125);
    listcloud->addItem(new QListWidgetItem(QIcon(":/google.jpg"), tr("Google cloud")));
    listcloud->addItem(new QListWidgetItem(QIcon(":/amazon.jpg"), tr("Amazon s3")));
    listcloud->addItem(new QListWidgetItem(QIcon(":/azure.jpg"), tr("Azure")));
    listcloud->addItem(new QListWidgetItem(QIcon(":/baiduyun.png"), tr("Baiduyun")));
    listcloud->addItem(new QListWidgetItem(QIcon(":/aliyun.jpg"), tr("Aliyun")));
    listcloud->setIconSize(QSize(20,20));
    listcloud->setFont(QFont("Microsoft YaHei UI",12));
    //
    statusLabel = new QLabel();
    statusLabel->setFont(QFont("Microsoft YaHei UI",12));
    ui->statusbar->addWidget(statusLabel);
}

CloudsWindow::~CloudsWindow()
{
    delete ui;
}

void CloudsWindow::SetCloud(QStringList clouds)
{
    //init button visiable
    ui->googleButton->setVisible(false);
    ui->azureButton->setVisible(false);
    ui->s3Button->setVisible(false);
    ui->aliyunButton->setVisible(false);
    ui->baiduyunButton->setVisible(false);

    this->clouds = clouds;
    QGridLayout *layout = (QGridLayout*)(ui->widget->layout());
    for(int i=0;i<clouds.size();++i)
    {
        QPushButton *button = GetButtonByName(clouds[i]);
        button->setVisible(true);
        layout->addWidget(button,i/3,i%3);
    }
    layout->addWidget(ui->addButton,clouds.size()/3,clouds.size()%3);
}

void CloudsWindow::SetBuckets(std::vector<Bucket> buckets)
{
    this->buckets = buckets;
}

QPushButton *CloudsWindow::GetButtonByName(QString cloudName)
{
    if(cloudName=="aliyun")
        return ui->aliyunButton;
    else if(cloudName=="s3")
        return ui->s3Button;
    else if(cloudName=="google")
        return ui->googleButton;
    else if(cloudName=="baiduyun")
        return ui->baiduyunButton;
    else if(cloudName=="azure")
        return ui->azureButton;
    else
        return NULL;
}
//之前云名字命名没统一……
QString CloudsWindow::ChangeCloudName(QString cloudName)
{
    if(cloudName=="Aliyun")
        return "aliyun";
    else if(cloudName=="Amazon s3")
        return "s3";
    else if(cloudName=="Google cloud")
        return "google";
    else if(cloudName=="Baiduyun")
        return "baiduyun";
    else if(cloudName=="Azure")
        return "azure";
    else
        return "";
}

void CloudsWindow::on_addButton_clicked()
{
    QPoint p = ui->addButton->pos();
    listcloud->setGeometry(p.x()+70,p.y()+50,50,100);
    listcloud->show();
}

void CloudsWindow::listcloudItemClicked(QListWidgetItem *item)
{
    statusLabel->setText("");
    QString cloudName = ChangeCloudName(item->text());
    listcloud->close();
    for(int i=0;i<clouds.size();++i)
        if(cloudName==clouds[i])
            return;
    //此处应调用cloud登录界面--待完善
    login = new LoginDialog(this->parentWidget(),cloudName);
    login->SetBuckets(buckets);
    connect(login,SIGNAL(AddCloud(CloudInfo)),this,SLOT(AddCloud_slot(CloudInfo)));
    login->exec();
}

void CloudsWindow::AddCloud_slot(CloudInfo cloud)
{
    login->close();
    //statusLabel->setText("Authorizing……");
    //QCoreApplication::processEvents();//强制进入消息循环，刷新UI
    statusLabel->setText("Completed!");
    clouds<<cloud.cloudName;
    SetCloud(clouds);
    emit AddCloud(cloud);
}

