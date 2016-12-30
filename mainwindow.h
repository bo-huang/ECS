#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QStringList>
#include <QLabel>
#include <QList>
#include <QTableWidgetItem>
#include <CORE/datatransfer.h>
#include <CLASS/snapshot.h>
#include <CLASS/cloudinfo.h>
#include <CLASS/operation.h>
#include <QMenu>
#include <QAction>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    void Init();
    void LoadFileList();
    void LoadCloudList();
    QString FileSizeToString(int size);
    void AddTask(Operation &operation);
    void ReadCloudFile();
    void WriteCloudFile();
    bool ReadServerFile(QByteArray &server);
    bool WriteServerFile(const QByteArray &server);
signals:
    void bucketNameIslegal(bool);
private slots:
    //自定义
    void upload_slot(QString,QString);//开始上传文件
    void upload_finished();
    void createBucket(QString bucketName, QString region, QString storageClass);
    void moveFile(QString bucketName, QString fileName, QString desBucketName);
    void addCloud(CloudInfo cloudInfo);
    void popMenu_download(bool);
    void popMenu_delete(bool);
    void popMenu_move(bool);
    void popMenu_rename(bool);
    void popMenu_remove(bool);
    void filesTableWidget_itemChanged(QTableWidgetItem*);
    void delete_finished();
    ///////////////////////////////////////
    void on_listWidget_itemSelectionChanged();
    void on_createButton_clicked();
    void on_filesTableWidget_itemSelectionChanged();
    void on_deleteButton_clicked();
    void on_filesTableWidget_itemClicked(QTableWidgetItem *item);
    void on_cloudsTableWidget_itemClicked(QTableWidgetItem *item);

    void on_removeButton_clicked();

    void on_addButton_clicked();

    void on_uploadButton_clicked();

    void on_downloadButton_clicked();

    void on_moveButton_clicked();

    void on_filesTableWidget_customContextMenuRequested(const QPoint &pos);

    void on_cloudsTableWidget_customContextMenuRequested(const QPoint &pos);

    void on_serverButton_clicked();

private:
    Ui::MainWindow *ui;
    DataTransfer *dataTransfer;
    QLabel *msgLabel;//状态栏消息
    QList<Snapshot> files;//所有文件的基本信息
    QList<CloudInfo> clouds;//clouds的基本信息
    //QStringList cloud;//cloud的名字
    //contextmenu
    QMenu *popMenu;
    QAction *downloadAction;
    QAction *deleteAction;
    QAction *moveAction;
    QAction *renameAction;
    QMenu *cloudPopMenu;
    QAction *removeAction;
    QString namebefore;//修改前的名字

};

#endif // MAINWINDOW_H
