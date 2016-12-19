#include "createbucketdialog.h"
#include "ui_createbucketdialog.h"
#include <QMessageBox>
#include <QPushButton>

CreateBucketDialog::CreateBucketDialog(QWidget *parent) :
    QDialog(parent),manger(manger),
    ui(new Ui::CreateBucketDialog)
{
    ui->setupUi(this);
}

CreateBucketDialog::~CreateBucketDialog()
{
    delete ui;
}

void CreateBucketDialog::on_ButtonBox_accepted()
{
    QString bucketName = ui->bucketLineEdit->text();
    if(bucketName.isNull()||bucketName.isEmpty())
    {
        ui->errorLabel->setText("The bucket name is empty!");
        return;
    }
    ui->ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->errorLabel->setText("Create bucket……");
    emit beginCreateBucket_signal(bucketName,
                           ui->regionComboBox->currentText(),
                           ui->storageClasscomboBox->currentText());
}

void CreateBucketDialog::on_ButtonBox_rejected()
{
    this->close();
}
void CreateBucketDialog::bucketNameIslegal_slot(bool isOK)
{
    if(!isOK)
    {
        ui->errorLabel->setText(tr("The bucket name is illegal or already exist!"));
    }
    else
    {
        ui->errorLabel->setText(tr("Create bucket successful!"));
    }
    ui->ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}
void CreateBucketDialog::SetText(QString msg)
{
    ui->errorLabel->setText(msg);
}
