#ifndef UPLOADCONTROL_H
#define UPLOADCONTROL_H
#include<QObject>
#include<QNetworkReply>

class UploadControl:public QObject
{
    Q_OBJECT
public:
    UploadControl(QNetworkReply *reply,int cloudID,int blockSize);
signals:
    void subFinished(int cloudID,int totalBlocks);
    void subFinished(int size);
private slots:
    void finished();
private:
    int cloudID;
    //int totalBlocks;
    int blockSize;
    QNetworkReply *reply;
};

#endif // UPLOADCONTROL_H
