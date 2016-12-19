#ifndef MOVEFILEDIALOG_H
#define MOVEFILEDIALOG_H

#include <QDialog>
#include <vector>
#include <CLASS/bucket.h>
#include <CLASS/snapshot.h>

namespace Ui {
class MoveFileDialog;
}

class MoveFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MoveFileDialog(QWidget *parent = 0);
    void Set(std::vector<Bucket> buckets,Snapshot &file);
    ~MoveFileDialog();
signals:
    void moveFile(QString bucketName,QString fileName,QString desBucket);
private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_bucketComboBox_currentTextChanged(const QString &arg1);

private:
    Ui::MoveFileDialog *ui;
    std::vector<Bucket>buckets;
};

#endif // MOVEFILEDIALOG_H
