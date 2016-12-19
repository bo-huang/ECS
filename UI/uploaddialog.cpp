#include "uploaddialog.h"
#include "ui_uploaddialog.h"
#include <QFileDialog>
#include <QListView>
#include <QStandardItemModel>
#include <QStandardItem>

UploadDialog::UploadDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UploadDialog)
{
    ui->setupUi(this);
}

UploadDialog::~UploadDialog()
{
    delete ui;
}

void UploadDialog::on_chooseFileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
           this,
           "choose file to upload",
           QDir::currentPath());
    if(!fileName.isEmpty())
    {
        filepath = fileName;
        ui->fileNameLabel->setText(fileName.split('/').last());
        QFileInfo info(fileName);
        if (info.exists())
        {
            QString size = FileSizeToString(info.size());
            ui->fileSizeLabel->setText(size);
        }
    }
}

void UploadDialog::on_uploadButton_clicked()
{
    if(!filepath.isEmpty()&&!ui->bucketComboBox->currentText().isEmpty())
    {
        this->close();
        //发射信号函数
        emit beginUpload_signal(ui->bucketComboBox->currentText(),filepath);
    }
    else if(ui->bucketComboBox->currentText().isEmpty())
    {
        ui->errorLabel->setText(tr("Please choose a bucket!"));
    }
    else
    {
        ui->errorLabel->setText(tr("Please choose a file!"));
    }
}

void UploadDialog::on_cancleButton_clicked()
{
    this->close();
}

QString UploadDialog::FileSizeToString(int size)
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

void UploadDialog::SetBucket(const std::vector<Bucket>& buckets)
{
    this->buckets = buckets;
    QListView *view = new QListView(this);
    QStandardItemModel *model = new QStandardItemModel(this);
    QStandardItem *item;
    for(int i=0;i<buckets.size();++i)
    {
        item = new QStandardItem(buckets[i].bucketName);
        item->setToolTip(buckets[i].storageClass);
        model->appendRow(item);
    }
    ui->bucketComboBox->setView(view);
    ui->bucketComboBox->setModel(model);

}

void UploadDialog::on_bucketComboBox_currentIndexChanged(int index)
{
    ui->storagfeclassLabel->setText(buckets[index].storageClass);
}
