#ifndef MOVECONTROL_H
#define MOVECONTROL_H

#include <QObject>
#include <QNetworkReply>
#include <QMutex>

class MoveControl : public QObject
{
    Q_OBJECT
public:
    MoveControl(int blockCount);
    void Copy(QNetworkReply *reply);
signals:
    void finished();
    void subFinished_signal(int finishedCount);
private slots:
    void subFinished_slot();
private:
    int blockCount;
    int finishedCount;
    QMutex mutex;
};

#endif // MOVECONTROL_H
