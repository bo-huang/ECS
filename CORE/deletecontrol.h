#ifndef DELETECONTROL_H
#define DELETECONTROL_H

#include <QObject>
#include <QNetworkReply>
#include <QMutex>

class DeleteControl : public QObject
{
    Q_OBJECT
public:
    DeleteControl(int blockCount);
    void Delete(QNetworkReply *reply=NULL);
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

#endif // DELETECONTROL_H
