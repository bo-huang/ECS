#ifndef PROGRESSBARDELEGATION_H
#define PROGRESSBARDELEGATION_H
#include <QProgressBar>
#include <QTableWidgetItem>
#include <QTime>
//检测传输速率
class ProgressBarDelegation : public QObject
{
    Q_OBJECT
public:
    ProgressBarDelegation(QProgressBar *progressBar,QTableWidgetItem * speed);
private:
    QString ToString(double speed);
signals:
    void finished();
private slots:
    void AddValue(int value);
    void beginUpload();
private:
    QProgressBar *progressBar;
    QTableWidgetItem *speedItem;
    QTime nowTime;
};

#endif // PROGRESSBARDELEGATION_H
