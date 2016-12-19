#ifndef CREATEBUCKETDIALOG_H
#define CREATEBUCKETDIALOG_H

#include <QDialog>
#include <QNetworkAccessManager>

namespace Ui {
class CreateBucketDialog;
}

class CreateBucketDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateBucketDialog(QWidget *parent = 0);
    void SetText(QString);
    ~CreateBucketDialog();
signals:
    void beginCreateBucket_signal(QString bucketName,QString region,QString storageClass);
private slots:
    void on_ButtonBox_accepted();

    void on_ButtonBox_rejected();

    void bucketNameIslegal_slot(bool);
private:
    Ui::CreateBucketDialog *ui;
    QNetworkAccessManager *manger;
};

#endif // CREATEBUCKETDIALOG_H
