#ifndef UPLOADASSIST_H
#define UPLOADASSIST_H
#include <QString>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <CloudSDK/cloudclient.h>
#include <vector>
#include <CLASS/bucket.h>
#include <CLASS/cloudinfo.h>
#include <QMutex>

class UploadAssist : public QObject
{
    Q_OBJECT
public:
     UploadAssist(QList<CloudInfo> &clouds,QString bucketName);
     void SetBuckets(const std::vector<Bucket>& buckets);//for s3
     void Upload(int cloudID,QByteArray blockID,QByteArray data);
     void Upload(int size);//已经存在的block不许要上传。但此处还是要发送信号
private:
     CloudClient* CreatCloudClient(CloudInfo &cloud);
signals:
     void put_signal(int cloudID,QByteArray blockID,QByteArray data);
     void subFinished(int size);
private slots:
     void put_slot(int cloudID,QByteArray blockID,QByteArray data);
     void subUploadFinished_slot(int size);
private:
     QNetworkAccessManager *manger;
     CloudClient **client;
     QList<CloudInfo> clouds;
     std::vector<Bucket>buckets;
     QString bucketName;
     QMutex mutex;
};

#endif // UPLOADASSIST_H
