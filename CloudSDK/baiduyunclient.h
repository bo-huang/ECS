#ifndef BAIDUYUNCLIENT_H
#define BAIDUYUNCLIENT_H
#include <CloudSDK/cloudclient.h>
#include <vector>
#include <CLASS/bucket.h>

class BaiduyunClient : public CloudClient
{
    Q_OBJECT
public:
    BaiduyunClient(QNetworkAccessManager *manger,QString secertID,QString secertKey);
    QNetworkReply* GetObject(QString bucketName,QString objectName) override;
    QNetworkReply* PutObject(QString bucketName,QString objectName,QByteArray data) override;
    QNetworkReply* CopyObject(QString bucketName, QString objectName, QString desBucketName) override;
    QNetworkReply* DeleteObject(QString bucketName,QString objectName) override;
    bool CreateBucket(QString bucketName, QString region, QString storageClass) override;
    void SetBuckets(std::vector<Bucket> &buckets);
    bool Login() override;
    bool DeleteBucket(QString bucketName) override;
private:
    QString GetCurrentDateTimeUtc();
    QString ParseStorageClass(QString storageClass);
    QString GetStorageClass(QString bucketName);
private:
    QString secertID;
    QString secertKey;
    QString region = "bj";//bj、gz
    std::vector<Bucket>buckets;
};

#endif // BAIDUYUNCLIENT_H
