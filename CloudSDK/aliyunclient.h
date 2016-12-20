#ifndef ALIYUNCLIENT_H
#define ALIYUNCLIENT_H
#include<QObject>
#include<QNetworkAccessManager>
#include"CloudSDK/cloudclient.h"

class AliyunClient : public CloudClient
{
    Q_OBJECT
public:
    AliyunClient(QNetworkAccessManager *manger,QString accessKeyID,QString accessKeySecert);
    QNetworkReply* GetObject(QString bucketName,QString objectName) override;
    QNetworkReply* PutObject(QString bucketName,QString objectName,QByteArray data) override;
    QNetworkReply* CopyObject(QString bucketName, QString objectName, QString desBucketName) override;
    QNetworkReply* DeleteObject(QString bucketName,QString objectName) override;
    bool CreateBucket(QString bucketName, QString region, QString storageClass) override;
    bool Login() override;
    bool DeleteBucket(QString bucketName) override;
private:
    QString CreateHeader(
            QString httpVerb,QString contentMD5, QString contentType, QString Date,
            QString canonicalizedHeaders, QString canonicalizedResource);
    QString GetCurrentTimeUTC();
    QString ParseLoaction(QString region);
private:
    QString accessKeyID;
    QString accessKeySecert;
};

#endif // ALIYUNCLIENT_H
