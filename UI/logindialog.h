#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <CLASS/cloudinfo.h>
#include <CloudSDK/cloudclient.h>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = 0);
    LoginDialog(QWidget *parent,QString cloudName);
    ~LoginDialog();
private:
    void Init();
    bool CheckAccount();
    bool CreateDefaultBucket();
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
};

#endif // LOGINDIALOG_H
