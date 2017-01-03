#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include<QJsonDocument>
#include<QJsonArray>
#include<QJsonObject>
#include<QFileInfo>
#include <fstream>
#include <QByteArray>
#include <QFileDialog>
#include <QProgressBar>
#include <UI/uploaddialog.h>
#include <UI/createbucketdialog.h>
#include <UI/movefiledialog.h>
#include <UI/cloudswindow.h>
#include <UI/serverdialog.h>
#include <CLASS/progressbardelegation.h>
#include <QFileInfo>
#include <QThread>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Init();
}

void MainWindow::Init()
{
    //初始化云
    ReadCloudFile();
    //此处仍生成传递给DataTransfer的QStringList，待修改……
    /*for(int i=0;i<clouds.size();++i)
        cloud<<clouds[i].cloudName;*/
    dataTransfer = new DataTransfer(clouds);
    //初始化状态栏
    msgLabel=new QLabel();
    msgLabel->setFont(QFont("Microsoft YaHei UI",12));
    this->ui->statusBar->addWidget(msgLabel);
    //均分每一列
    //ui->filesTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //ui->informationTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //ui->taskTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //ui->completeTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->cloudsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //设置列宽
    ui->filesTableWidget->setColumnWidth(0,100);
    ui->filesTableWidget->setColumnWidth(1,80);
    ui->filesTableWidget->setColumnWidth(2,80);
    ui->informationTableWidget->setColumnWidth(1,160);
    ui->taskTableWidget->setColumnWidth(1,150);
    ui->taskTableWidget->setColumnWidth(2,250);
    //表头颜色
    //ui->filesTableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{ background-color: rgb(170, 170, 255)}");
    //初始化按钮隐藏
    ui->informationTableWidget->setVisible(false);
    ui->downloadButton->setVisible(false);
    ui->deleteButton->setVisible(false);
    ui->removeButton->setVisible(false);
    ui->moveButton->setVisible(false);
    //pop menu
    popMenu = new QMenu(ui->filesTableWidget);
    downloadAction = new QAction(tr("download"),this);
    deleteAction = new QAction(tr("delete"),this);
    moveAction = new QAction(tr("move"),this);
    renameAction = new QAction(tr("rename"),this);
    connect(downloadAction,SIGNAL(triggered(bool)),this,SLOT(popMenu_download(bool)));
    connect(deleteAction,SIGNAL(triggered(bool)),this,SLOT(popMenu_delete(bool)));
    connect(moveAction,SIGNAL(triggered(bool)),this,SLOT(popMenu_move(bool)));
    connect(renameAction,SIGNAL(triggered(bool)),this,SLOT(popMenu_rename(bool)));
    popMenu->addAction(downloadAction);
    popMenu->addAction(deleteAction);
    popMenu->addAction(moveAction);
    popMenu->addAction(renameAction);

    cloudPopMenu = new QMenu(ui->cloudsTableWidget);
    removeAction = new QAction(tr("remove"),this);
    connect(removeAction,SIGNAL(triggered(bool)),this,SLOT(popMenu_remove(bool)));
    cloudPopMenu->addAction(removeAction);

    //初始化文件列表、日志和cloud信息
    LoadFileList();
    LoadCloudList();
    //filetable item changed
    connect(ui->filesTableWidget,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(filesTableWidget_itemChanged(QTableWidgetItem*)));
}

MainWindow::~MainWindow()
{
    delete ui;
}
//加载文件列表
void MainWindow::LoadCloudList()
{
    int currentRow = 0;
    for(int i=0;i<clouds.size();++i)
    {
        ui->cloudsTableWidget->insertRow(currentRow);
        QTableWidgetItem *item[3];
        for(int j=0;j<3;++j)
        {
            item[j]=new QTableWidgetItem();
            item[j]->setTextAlignment(Qt::AlignCenter);//设置单元格对齐方式
            item[j]->setFont(QFont("Microsoft YaHei UI",12));//设置字体
            ui->cloudsTableWidget->setItem(currentRow,j,item[j]);
        }
        item[0]->setText(clouds[i].cloudName);
        item[1]->setText(clouds[i].account);
        item[2]->setText(clouds[i].addTime);

        ++currentRow;
    }
}
//加载云列表
void MainWindow::LoadFileList()
{
    int currentRow = 0;
    //下载snapshot
    QJsonArray jarr_buckets = dataTransfer->GetSnapshot().array();
    //加载文件基本信息
    files.clear();
    //ui->filesTableWidget->clearContents();
    //ui->filesTableWidget->setColumnCount(4);
    int rowCount = ui->filesTableWidget->rowCount();

    for(int i=0;i<jarr_buckets.size();i++)
    {
        QJsonObject jo_bucket = jarr_buckets.at(i).toObject();
        QJsonArray jarr_files = jo_bucket["files"].toArray();
        for(int j=0;j<jarr_files.size();j++)
        {
            QJsonObject jo_file = jarr_files.at(j).toObject();
            QTableWidgetItem *item[4];
            if(currentRow>=rowCount)
            {
                ui->filesTableWidget->insertRow(currentRow);
                for(int k=0;k<4;k++)
                {
                    item[k]=new QTableWidgetItem();
                    item[k]->setTextAlignment(Qt::AlignCenter);//设置单元格对齐方式
                    item[k]->setFont(QFont("Microsoft YaHei UI",12));//设置字体

                    ui->filesTableWidget->setItem(currentRow,k,item[k]);
                }
            }
            else
            {
                for(int k=0;k<4;++k)
                    item[k]=ui->filesTableWidget->item(currentRow,k);
            }
            Snapshot file;
            file.bucket = jo_bucket["bucketname"].toString();
            file.fileName = jo_file["filename"].toString();
            file.storageClass = jo_bucket["storageclass"].toString();
            file.modified = jo_file["modified"].toString();
            file.fileSize = jo_file["filesize"].toInt();
            file.frequency = jo_file["frequency"].toInt();
            file.createTime = jo_file["createtime"].toString();
            file.occupiedSpace  = jo_file["occupiedspace"].toInt();
            files.append(file);
            QString name=file.fileName,type="";
            QStringList tmp = file.fileName.split('.');
            if(tmp.size()>1)
            {
                type = "."+tmp[tmp.size()-1];
                name = "";
                for(int i=tmp.size()-3;i>=0;--i)
                    name+=tmp[i]+".";
                if(tmp.size()>=2)
                    name+=tmp[tmp.size()-2];
            }
            item[0]->setText(name);
            item[1]->setText(type);
            item[2]->setText(FileSizeToString(file.fileSize));
            item[3]->setText(file.modified);

            ++currentRow;
        }
    }
    ui->filesTableWidget->setRowCount(currentRow);

}
//文件字节转KB MB GB
QString MainWindow::FileSizeToString(int size)
{
    int KB = 1024;
    int MB = KB*1024;
    int GB = MB*1024;
    if(size<KB)
        return QString("%1B").arg(size);
    else if (size<MB)
        return QString("%1KB").arg(size/KB);
    else if (size<GB)
        return QString("%1MB").arg(QString::number(1.0*size/MB,10,2));
    else
        return QString("%1GB").arg(QString::number(1.0*size/GB,10,2));
}

void MainWindow::on_listWidget_itemSelectionChanged()
{
    QString selText = ui->listWidget->currentItem()->text();
    if(selText=="Files")
    {
        ui->informationTableWidget->setVisible(false);
        ui->downloadButton->setVisible(false);
        ui->deleteButton->setVisible(false);
        ui->moveButton->setVisible(false);

        ui->stackedWidget->setCurrentWidget(ui->filesPage);
    }
    else if(selText=="Tasks")
    {
        ui->stackedWidget->setCurrentWidget(ui->taskPage);
    }
    else if(selText=="Complete")
    {
        ui->stackedWidget->setCurrentWidget(ui->completePage);
    }
    else if(selText=="Clouds")
    {
        ui->removeButton->setVisible(false);
        ui->stackedWidget->setCurrentWidget(ui->cloudsPage);
    }
}
//新建bucket事件
void MainWindow::on_createButton_clicked()
{
    CreateBucketDialog *dlg = new CreateBucketDialog(this);
    connect(dlg,SIGNAL(beginCreateBucket_signal(QString,QString,QString)),this,SLOT(createBucket(QString,QString,QString)));
    connect(this,SIGNAL(bucketNameIslegal(bool)),dlg,SLOT(bucketNameIslegal_slot(bool)));
    dlg->show();
}
void MainWindow::createBucket(QString bucketName, QString region, QString storageClass)
{
    if(dataTransfer->CreateBucket(bucketName,region,storageClass))
        emit bucketNameIslegal(true);
    else
        emit bucketNameIslegal(false);
}

//上传文件事件
void MainWindow::on_uploadButton_clicked()
{
    UploadDialog *uploadDialog = new UploadDialog(this);
    uploadDialog->SetBucket(dataTransfer->GetBucket());

    connect(uploadDialog,SIGNAL(beginUpload_signal(QString,QString)),this,SLOT(upload_slot(QString,QString)));
    uploadDialog->show();
}
void MainWindow::upload_slot(QString bucketName,QString path)
{
    Operation op;
    op.bucket = bucketName;
    op.path = path;
    op.operation = "upload";
    op.fileSize = dataTransfer->GetUploadSize(path);
    op.fileName = path.split('/').last();
    if(dataTransfer->IsFileExist(op.bucket,op.fileName))
    {
        QMessageBox  messageBox;
        messageBox.setText(op.fileName+" already exists, do you want to override it?");
        messageBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        if(messageBox.exec()==QMessageBox::Yes)
        {
            //先删除文件
            dataTransfer->DeleteFile(op.bucket,op.fileName);
            for(int row=0;row<files.size();++row)
            {
                if(files[row].fileName==op.fileName&&files[row].bucket==op.bucket)
                {
                    files.removeAt(row);
                    ui->filesTableWidget->removeRow(row);
                    break;
                }
            }
            AddTask(op);
        }
    }
    else
        AddTask(op);
}

//下载文件事件
void MainWindow::on_downloadButton_clicked()
{
    int row = ui->filesTableWidget->currentRow();
    if(row>=0&&row<files.size())
    {
        QString bucketName = files[row].bucket;
        QString fileName = files[row].fileName;
        QStringList tmp = fileName.split('.');
        QString suffix,filter="All(*)";
        if(tmp.size()>1)//文件有后缀
        {
            suffix = tmp.last();
            filter = QString("%1 (*.%2)").arg(suffix).arg(suffix);
        }
        QString outputpath = QFileDialog::getSaveFileName(this,"Save",fileName,filter);
        if(!outputpath.isEmpty())
        {
            ++(files[row].frequency);
            Operation op;
            op.fileName = fileName;
            op.bucket = bucketName;
            op.operation = "download";
            op.path = outputpath;
            op.fileSize = dataTransfer->GetDownloadSize(bucketName,fileName);
            AddTask(op);
        }
    }
}

void MainWindow::AddTask(Operation &op)
{
    if(op.operation=="download")
    {
        DataTransfer *download = new DataTransfer(clouds);
        int row = ui->taskTableWidget->rowCount();
        ui->taskTableWidget->insertRow(row);
        QTableWidgetItem *item[3];
        QString str[2]={op.operation,op.fileName};

        for(int i=0;i<2;++i)
        {
            item[i] = new QTableWidgetItem(str[i]);
            item[i]->setTextAlignment(Qt::AlignCenter);
            item[i]->setFont(QFont("Microsoft YaHei UI",12));
            ui->taskTableWidget->setItem(row,i,item[i]);
        }

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setValue(0);
        progressBar->setRange(0,op.fileSize);
        ui->taskTableWidget->setCellWidget(row,2,progressBar);

        item[2] = new QTableWidgetItem();
        item[2]->setTextAlignment(Qt::AlignCenter);
        item[2]->setFont(QFont("Microsoft YaHei UI",12));
        ui->taskTableWidget->setItem(row,3,item[2]);

        ProgressBarDelegation *delegation = new ProgressBarDelegation(progressBar,item[2]);
        connect(download,SIGNAL(downloadProgress_signal(int)),delegation,SLOT(AddValue(int)));
        download->DownLoad(op.bucket,op.fileName,op.path);
    }
    else if(op.operation=="upload")
    {
        DataTransfer *upload = new DataTransfer(clouds);
        int row = ui->taskTableWidget->rowCount();
        ui->taskTableWidget->insertRow(row);
        QTableWidgetItem *item[3];
        QString str[2]={op.operation,op.fileName};

        for(int i=0;i<2;++i)
        {
            item[i] = new QTableWidgetItem(str[i]);
            item[i]->setTextAlignment(Qt::AlignCenter);
            item[i]->setFont(QFont("Microsoft YaHei UI",12));
            ui->taskTableWidget->setItem(row,i,item[i]);
        }

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setValue(0);
        progressBar->setRange(0,op.fileSize);
        ui->taskTableWidget->setCellWidget(row,2,progressBar);

        item[2] = new QTableWidgetItem();
        item[2]->setTextAlignment(Qt::AlignCenter);
        item[2]->setFont(QFont("Microsoft YaHei UI",12));
        ui->taskTableWidget->setItem(row,3,item[2]);

        ProgressBarDelegation *delegation = new ProgressBarDelegation(progressBar,item[2]);
        connect(upload,SIGNAL(uploadProgress_signal(int)),delegation,SLOT(AddValue(int)));
        connect(upload,SIGNAL(beginUpload()),delegation,SLOT(beginUpload()));
        //在新的线程中运行
        QThread *thread = new QThread;
        upload->moveToThread(thread);
        thread->start();
        //虽然moveToThread了，但必须通过信号与槽的方式启动多线程……
        connect(upload,SIGNAL(upload_signalForThread(QString,QString)),upload,SLOT(SpiltToSegment(QString,QString)));
        upload->Upload(op.bucket,op.path);
        //END
        connect(delegation,SIGNAL(finished()),this,SLOT(upload_finished()));
    }
    else if(op.operation=="move")
    {
        int row = ui->taskTableWidget->rowCount();
        ui->taskTableWidget->insertRow(row);
        QTableWidgetItem *item[3];
        QString str[2]={op.operation,op.fileName};

        for(int i=0;i<2;++i)
        {
            item[i] = new QTableWidgetItem(str[i]);
            item[i]->setTextAlignment(Qt::AlignCenter);
            item[i]->setFont(QFont("Microsoft YaHei UI",12));
            ui->taskTableWidget->setItem(row,i,item[i]);
        }

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setValue(0);
        ui->taskTableWidget->setCellWidget(row,2,progressBar);
        item[2] = new QTableWidgetItem();
        item[2]->setTextAlignment(Qt::AlignCenter);
        item[2]->setFont(QFont("Microsoft YaHei UI",12));
        ui->taskTableWidget->setItem(row,3,item[2]);

        DataTransfer *moveFile = new DataTransfer(clouds);
        connect(moveFile,SIGNAL(moveProgress_signal(int)),progressBar,SLOT(setValue(int)));
        connect(moveFile,SIGNAL(moveProgressSetRange(int,int)),progressBar,SLOT(setRange(int,int)));
        moveFile->MoveFile(op.bucket,op.fileName,op.desBucket);
    }
    else if(op.operation=="delete")
    {
        int row = ui->taskTableWidget->rowCount();
        ui->taskTableWidget->insertRow(row);
        QTableWidgetItem *item[3];
        QString str[2]={op.operation,op.fileName};

        for(int i=0;i<2;++i)
        {
            item[i] = new QTableWidgetItem(str[i]);
            item[i]->setTextAlignment(Qt::AlignCenter);
            item[i]->setFont(QFont("Microsoft YaHei UI",12));
            ui->taskTableWidget->setItem(row,i,item[i]);
        }

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setValue(0);
        ui->taskTableWidget->setCellWidget(row,2,progressBar);
        item[2] = new QTableWidgetItem();
        item[2]->setTextAlignment(Qt::AlignCenter);
        item[2]->setFont(QFont("Microsoft YaHei UI",12));
        ui->taskTableWidget->setItem(row,3,item[2]);

        DataTransfer *deleteFile = new DataTransfer(clouds);
        connect(deleteFile,SIGNAL(deleteProgress_signal(int)),progressBar,SLOT(setValue(int)));
        connect(deleteFile,SIGNAL(deleteProgressSetRange(int,int)),progressBar,SLOT(setRange(int,int)));
        connect(deleteFile,SIGNAL(deleteFinished_signal()),this,SLOT(delete_finished()));
        deleteFile->DeleteFile(op.bucket,op.fileName);
    }
}

void MainWindow::delete_finished()
{
    QCoreApplication::processEvents();
    LoadFileList();
}

void MainWindow::upload_finished()
{
    LoadFileList();
}

//单击文件列表的某一文件
void MainWindow::on_filesTableWidget_itemClicked(QTableWidgetItem *item)
{
    //解决：当点击的row和上次选中的row一致的时候
    //on_filesTableWidget_itemSelectionChanged()没有调用
    //这个时候informationTableWidget、deleteButton和downloadButton
    //还处于隐藏状态
    //if(item->row()==ui->filesTableWidget->currentRow())
    {
        on_filesTableWidget_itemSelectionChanged();
    }
}
//文件列表中的选中文件改变了（比如按键盘上、下方向键的时候）
void MainWindow::on_filesTableWidget_itemSelectionChanged()
{
    int row = ui->filesTableWidget->currentRow();
    if(row>=0&&row<files.size())
    {
        //设置按钮可见
        ui->informationTableWidget->setVisible(true);
        ui->deleteButton->setVisible(true);
        ui->downloadButton->setVisible(true);
        ui->moveButton->setVisible(true);

        Snapshot file = files[row];
        ui->informationTableWidget->setItem(0,1,new QTableWidgetItem(file.fileName));
        ui->informationTableWidget->setItem(1,1,new QTableWidgetItem(FileSizeToString(file.fileSize)));
        ui->informationTableWidget->setItem(2,1,new QTableWidgetItem(file.modified));
        ui->informationTableWidget->setItem(3,1,new QTableWidgetItem(file.bucket));
        ui->informationTableWidget->setItem(4,1,new QTableWidgetItem(file.storageClass));
        ui->informationTableWidget->setItem(5,1,new QTableWidgetItem(QString::number(file.frequency,10)));
        ui->informationTableWidget->setItem(6,1,new QTableWidgetItem(file.createTime));
        ui->informationTableWidget->setItem(7,1,new QTableWidgetItem(FileSizeToString(file.occupiedSpace)));
        ui->informationTableWidget->setFont(QFont("Microsoft YaHei UI",10));
    }
}
//删除文件
void MainWindow::on_deleteButton_clicked()
{
    int row = ui->filesTableWidget->currentRow();
    if(row>=0&&row<files.size())
    {
        QMessageBox  messageBox;
        messageBox.setText("Confirm to delete "+files[row].fileName+"?");
        messageBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        if(messageBox.exec()==QMessageBox::Yes)
        {
            //if delete success
            /*if(dataTransfer->DeleteFile(files[row].bucket,files[row].fileName))
            {
                files.removeAt(row);
                ui->filesTableWidget->removeRow(row);
            }*/
            Operation operation;
            operation.operation = "delete";
            operation.bucket = files[row].bucket;
            operation.fileName = files[row].fileName;
            AddTask(operation);
        }
    }
    else
        QMessageBox::information(this,"inform","please choose a file!");

}
//移动文件(不同bucket之间)
void MainWindow::on_moveButton_clicked()
{
    int row = ui->filesTableWidget->currentRow();
    if(row>=0&&row<files.size())
    {
        MoveFileDialog *dlg = new MoveFileDialog(this);
        connect(dlg,SIGNAL(moveFile(QString,QString,QString)),this,
                SLOT(moveFile(QString,QString,QString)));
        dlg->Set(dataTransfer->GetBucket(),files[row]);
        dlg->show();
    }
}
void MainWindow::moveFile(QString bucketName, QString fileName, QString desBucketName)
{
    Operation operation;
    operation.operation = "move";
    operation.bucket = bucketName;
    operation.fileName = fileName;
    operation.desBucket = desBucketName;
    if(dataTransfer->IsFileExist(desBucketName,fileName))
    {
        QMessageBox  messageBox;
        messageBox.setText(QString("%1 already exists in %2, do you want to override it?").arg(fileName,desBucketName));
        messageBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        if(messageBox.exec()==QMessageBox::Yes)
        {
            int row = ui->filesTableWidget->currentRow();
            //先删除文件
            dataTransfer->DeleteFile(desBucketName,fileName);
            for(int i=files.size()-1;i>=0;--i)
            {
                if(files[i].bucket==desBucketName&&files[i].fileName==fileName)
                {
                    files[row].bucket = desBucketName;
                    files[row].modified = QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate);
                    ui->filesTableWidget->item(row,3)->setText(files[row].modified);
                    ui->informationTableWidget->item(2,1)->setText(files[row].modified);
                    ui->informationTableWidget->item(3,1)->setText(desBucketName);
                    //删除行
                    files.removeAt(i);
                    ui->filesTableWidget->removeRow(i);
                    break;
                }
            }

            AddTask(operation);
        }
    }
    else
    {
        int row = ui->filesTableWidget->currentRow();
        files[row].bucket = desBucketName;
        files[row].modified = QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate);
        ui->filesTableWidget->item(row,3)->setText(files[row].modified);
        ui->informationTableWidget->item(2,1)->setText(files[row].modified);
        ui->informationTableWidget->item(3,1)->setText(desBucketName);
        AddTask(operation);
    }
}


//单击云列表中的某一个云
void MainWindow::on_cloudsTableWidget_itemClicked(QTableWidgetItem *item)
{
    ui->removeButton->setVisible(true);
}
//配置服务器（用于云间数据迁移）
void MainWindow::on_serverButton_clicked()
{
    //读server文本
    QByteArray server;
    ReadServerFile(server);

    ServerDialog *serverDialog = new ServerDialog(this);
    serverDialog->SetServer(QString(server));
    serverDialog->exec();
    if(serverDialog->isOK)
    {
        WriteServerFile(serverDialog->serverName.toLatin1());
    }
}
//移除云
void MainWindow::on_removeButton_clicked()
{
    int row = ui->cloudsTableWidget->currentRow();
    if(row>=0&&row<clouds.size())
    {
        QMessageBox  messageBox;
        messageBox.setText("Confirm to delete "+clouds[row].cloudName+" cloud?");
        messageBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        if(messageBox.exec()==QMessageBox::Yes)
        {
            msgLabel->setText("Prepare for migrating data...");
            if(Migration(clouds[row].cloudName))
            {
                clouds.removeAt(row);
                ui->cloudsTableWidget->removeRow(row);
                dataTransfer = new DataTransfer(clouds);
                WriteCloudFile();
                msgLabel->setText("Completed");
                QMessageBox::information(this,"Warning","Do not delete the cloud before data has been migrated!",QMessageBox::Ok);
            }
            else
            {
                QMessageBox::information(this,"Tip","Please check the server!",QMessageBox::Ok);
            }
            msgLabel->setText("");
        }
    }
    else
        QMessageBox::information(this,"inform","please choose a file!");
}
//向服务器发送待迁移的数据
bool MainWindow::Migration(QString cloudName)
{
    return dataTransfer->RemoveCloud(cloudName);
}

//添加云
void MainWindow::on_addButton_clicked()
{
    CloudsWindow *cloudsWindow = new CloudsWindow(this);
    connect(cloudsWindow,SIGNAL(AddCloud(CloudInfo)),this,SLOT(addCloud(CloudInfo)));
    QStringList cloud;
    for(int i=0;i<clouds.size();++i)
        cloud<<clouds[i].cloudName;
    cloudsWindow->SetCloud(cloud);
    cloudsWindow->SetBuckets(dataTransfer->GetBucket());
    cloudsWindow->show();
}
void MainWindow::addCloud(CloudInfo cloudInfo)
{
     clouds.append(cloudInfo);
     //cloud<<cloudInfo.cloudName;
     int row = ui->cloudsTableWidget->rowCount();
     ui->cloudsTableWidget->insertRow(row);
     QTableWidgetItem *item[3];
     for(int j=0;j<3;++j)
     {
         item[j]=new QTableWidgetItem();
         item[j]->setTextAlignment(Qt::AlignCenter);//设置单元格对齐方式
         item[j]->setFont(QFont("Microsoft YaHei UI",12));//设置字体
         ui->cloudsTableWidget->setItem(row,j,item[j]);
     }
     item[0]->setText(cloudInfo.cloudName);
     item[1]->setText(cloudInfo.account);
     item[2]->setText(cloudInfo.addTime);
     dataTransfer = new DataTransfer(clouds);
     WriteCloudFile();
}

void MainWindow::ReadCloudFile()
{
    QFileInfo info("clouds");
    const int size = info.size();
    char buffer[size];
    ifstream is;
    is.open("clouds",ios::binary);
    is.read(buffer,size);
    is.close();
    QJsonDocument clouds_jd = QJsonDocument::fromJson(QByteArray(buffer,size));
    QJsonArray clouds_ja = clouds_jd.array();
    for(int i=0;i<clouds_ja.size();++i)
    {
        QJsonObject cloud_jo = clouds_ja.at(i).toObject();
        CloudInfo cloudInfo;
        cloudInfo.cloudName = cloud_jo["cloud"].toString();
        cloudInfo.account = cloud_jo["account"].toString();
        cloudInfo.addTime = cloud_jo["addtime"].toString();
        cloudInfo.certificate = cloud_jo["key"].toString();
        cloudInfo.defaultBucket = cloud_jo["defaultbucket"].toString();
        //因为google cloud需要翻墙才能访问，为了便于调试，先去掉
        /*if(cloudInfo.cloudName=="google")
            continue;*/
        clouds.append(cloudInfo);
    }
}

void MainWindow::WriteCloudFile()
{

    QJsonDocument clouds_jd;
    QJsonArray clouds_ja;
    for(int i=0;i<clouds.size();++i)
    {
        QJsonObject cloud_jo;
        cloud_jo.insert("cloud",clouds[i].cloudName);
        cloud_jo.insert("account",clouds[i].account);
        cloud_jo.insert("addtime",clouds[i].addTime);
        cloud_jo.insert("key",clouds[i].certificate);
        cloud_jo.insert("defaultbucket",clouds[i].defaultBucket);
        clouds_ja.append(cloud_jo);
    }
    clouds_jd.setArray(clouds_ja);
    QByteArray jsondata = clouds_jd.toJson();
    ofstream os;
    os.open("clouds",ios::binary);
    os.write(jsondata.data(),jsondata.size());
    os.close();

}
bool MainWindow::ReadServerFile(QByteArray &server)
{
    QFileInfo info("server");
    const int size = info.size();
    if(size==0)
        return false;
    char buffer[size];
    ifstream is;
    is.open("server",ios::binary);
    is.read(buffer,size);
    is.close();
    server = QByteArray(buffer,size);
    return true;
}
bool MainWindow::WriteServerFile(const QByteArray &server)
{
    ofstream os;
    os.open("server",ios::binary);
    os.write(server.data(),server.size());
    os.close();
    return true;
}

//pop menu
void MainWindow::on_filesTableWidget_customContextMenuRequested(const QPoint &pos)
{
    if(ui->filesTableWidget->itemAt(pos)!=NULL)
        popMenu->exec(QCursor::pos());
}
void MainWindow::on_cloudsTableWidget_customContextMenuRequested(const QPoint &pos)
{
    if(ui->cloudsTableWidget->itemAt(pos)!=NULL)
        cloudPopMenu->exec(QCursor::pos());
}
void MainWindow::popMenu_download(bool)
{
    on_downloadButton_clicked();
}

void MainWindow::popMenu_delete(bool)
{
    on_deleteButton_clicked();
}
void MainWindow::popMenu_move(bool)
{
    on_moveButton_clicked();
}
void MainWindow::popMenu_rename(bool)
{
    int edit_row = ui->filesTableWidget->currentRow();
    if(edit_row>=0&&edit_row<ui->filesTableWidget->rowCount())
    {
        QTableWidgetItem *item = ui->filesTableWidget->item(edit_row, 0);
        ui->filesTableWidget->setCurrentCell(edit_row, 0);
        //ui->filesTableWidget->openPersistentEditor(item); //打开编辑项
        ui->filesTableWidget->editItem(item);
        namebefore = item->text();
        //关闭编辑项
        //ui->filesTableWidget->closePersistentEditor(item);
    }
}
void MainWindow::popMenu_remove(bool)
{
    on_removeButton_clicked();
}

//这个函数很有问题！！！！！多关注一下（找不到更好的监听tablewidget某一项改变的事件了）
void MainWindow::filesTableWidget_itemChanged(QTableWidgetItem *item)
{
    int row = item->row();
    int column = item->column();
    //原来添加行也会触发时间事件，更可怕的是添加行的话，files[row]下标越界了
    //还有一个bug就是rename莫名其妙会调用4次（列数）
    //检查了好久才发现啊……
    if(row>=files.size()||column>0)
        return;
    QStringList tmp = files[row].fileName.split('.');
    QString suffix="";
    if(tmp.size()>1)//文件有后缀
        suffix = "."+tmp.last();
    QString newName = item->text()+suffix;
    if(newName!=files[row].fileName)
    {
        QString bucketName = files[row].bucket;
        if(dataTransfer->IsFileExist(bucketName,newName))
        {

            item->setText(namebefore);
            QMessageBox::information(this,"inform",newName + " alrady exists.");
        }
        else
        {
            dataTransfer->Rename(files[row].bucket,files[row].fileName,newName);
            files[row].fileName = newName;
            files[row].modified = QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate);
            ui->informationTableWidget->item(0,1)->setText(newName);
            ui->informationTableWidget->item(2,1)->setText(files[row].modified);
            ui->filesTableWidget->item(row,3)->setText(files[row].modified);
        }
    }
}

