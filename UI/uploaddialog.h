#ifndef UPLOADDIALOG_H
#define UPLOADDIALOG_H

#include <QDialog>
#include <CLASS/bucket.h>
#include <vector>

namespace Ui {
class UploadDialog;
}

class UploadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UploadDialog(QWidget *parent = 0);
    void SetBucket(const std::vector<Bucket>&buckets);
    ~UploadDialog();
private:
    QString FileSizeToString(int size);
    //信号函数只声明不实现!!!
signals:
    void beginUpload_signal(QString bucket,QString fileName);
private slots:
    void on_chooseFileButton_clicked();

    void on_uploadButton_clicked();

    void on_cancleButton_clicked();

    void on_bucketComboBox_currentIndexChanged(int index);

private:
    Ui::UploadDialog *ui;
    QString filepath;
    std::vector<Bucket>buckets;
};

#endif // UPLOADDIALOG_H
