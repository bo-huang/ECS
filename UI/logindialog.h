#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <CLASS/cloudinfo.h>
#include <CLASS/bucket.h>
#include <CloudSDK/cloudclient.h>
#include <vector>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = 0);
    LoginDialog(QWidget *parent,QString cloudName);
    void SetBuckets(std::vector<Bucket> &buckets);
    ~LoginDialog();
private:
    void Init();
    bool CheckAccount();
    bool CreateDefaultBucket();
    bool CreateUserBucket();
    CloudClient * CreateCloudClient();
signals:
    void AddCloud(CloudInfo cloud);
private slots:
    void on_nextButton_1_clicked();

    void on_preButton_clicked();

    void on_nextButton_2_clicked();

    void on_importButton_clicked();

    void on_nextButton_3_clicked();

private:
    Ui::LoginDialog *ui;
    QString cloudName;
    QString defaultBucket;
    std::vector<Bucket>buckets;
};

#endif // LOGINDIALOG_H
