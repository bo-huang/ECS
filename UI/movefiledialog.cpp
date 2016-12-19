#include "movefiledialog.h"
#include "ui_movefiledialog.h"
#include <QListView>
#include <QStandardItem>
#include <QStandardItemModel>

MoveFileDialog::MoveFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MoveFileDialog)
{
    ui->setupUi(this);
}

MoveFileDialog::~MoveFileDialog()
{
    delete ui;
}

void MoveFileDialog::Set(std::vector<Bucket> buckets, Snapshot &file)
{
    ui->fileLabel->setText(file.fileName);
    ui->bucketLabel->setText(file.bucket);

    this->buckets = buckets;
    QListView *view = new QListView(this);
    QStandardItemModel *model = new QStandardItemModel(this);
    QStandardItem *item;
    for(int i=0;i<buckets.size();++i)
    {
        if(buckets[i].bucketName==file.bucket)
            continue;
        item = new QStandardItem(buckets[i].bucketName);
        item->setToolTip(buckets[i].storageClass);
        model->appendRow(item);
    }
    ui->bucketComboBox->setView(view);
    ui->bucketComboBox->setModel(model);
}

void MoveFileDialog::on_buttonBox_accepted()
{
    this->close();
    emit moveFile(ui->bucketLabel->text(),ui->fileLabel->text(),
                  ui->bucketComboBox->currentText());
}

void MoveFileDialog::on_buttonBox_rejected()
{
    this->close();
}

void MoveFileDialog::on_bucketComboBox_currentTextChanged(const QString &arg1)
{
    for(int i=0;i<buckets.size();++i)
        if(buckets[i].bucketName == arg1)
        {
            ui->storageclassLabel->setText(buckets[i].storageClass);
            break;
        }
}
