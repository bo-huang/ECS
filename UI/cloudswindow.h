#ifndef CLOUDSWINDOW_H
#define CLOUDSWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <CLASS/cloudinfo.h>
#include <UI/logindialog.h>
#include <QLabel>
#include <vector>
#include <CLASS/bucket.h>
namespace Ui {
class CloudsWindow;
}

class CloudsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CloudsWindow(QWidget *parent = 0);
    void SetCloud(QStringList clouds);//设置当前已登录的云
    void SetBuckets(std::vector<Bucket> buckets);
    ~CloudsWindow();
private:
    QPushButton *GetButtonByName(QString cloudName);
    QString ChangeCloudName(QString cloudName);//前面中下的果
signals:
    void AddCloud(CloudInfo cloud);
private slots:
    void AddCloud_slot(CloudInfo cloud);
    void on_addButton_clicked();
    void listcloudItemClicked(QListWidgetItem*);
private:
    Ui::CloudsWindow *ui;
    QListWidget *listcloud;
    QStringList clouds;//clouds name
    QLabel *statusLabel;
    LoginDialog *login;
    std::vector<Bucket>buckets;
};

#endif // CLOUDSWINDOW_H
