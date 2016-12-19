#ifndef S3CLIENT_H
#define S3CLIENT_H
#include<QNetworkAccessManager>
#include"CloudSDK/cloudclient.h"
#include<CLASS/bucket.h>
#include<vector>

class S3Client : public CloudClient
{
    Q_OBJECT
public:
    S3Client(QNetworkAccessManager *manger,QString secertID,QString secertKey);
    QNetworkReply* GetObject(QString bucketName,QString objectName) override;
    QNetworkReply* PutObject(QString bucketName,QString objectName,QByteArray data) override;
    QNetworkReply* CopyObject(QString bucketName, QString objectName, QString desBucketName) override;
    QNetworkReply* DeleteObject(QString bucketName,QString objectName) override;
    bool CreateBucket(QString bucketName, QString region, QString storageClass) override;
    void SetBuckets(std::vector<Bucket> &buckets);
    bool Login();
private:
    //注意：在生成签名时，无论bucket的存储区域在哪里，都是us-east-1 !!!（好隐蔽的问题。。。）
    QNetworkRequest GetRequestForGetObject(QString bucketName,QString objectName);
    QNetworkRequest GetRequestForPutObject(QString bucketName,QString objectName,QByteArray data);
    QNetworkRequest GetRequestForDeleteObject(QString bucketName,QString objectName);
    QNetworkRequest GetRequestForCreateBucket(QString bucketName,QByteArray data);
    QNetworkRequest GetRequestForCopyObject(QString bucketName,QString objectName,QString desBucketName);
    QString GetDateTime();
    QString GetDate();
    QByteArray GetSignatureKey(QByteArray date,QByteArray region);
    QString GetHost(QString region,QString bucketName);//根据不同region生成不同host
    QString ParseLoaction(QString region);
    QString ParseStorageClass(QString storageClass);
    QString GetRegion(QString bucketName);
    QString GetStorageClass(QString bucketName);
private:
    std::vector<Bucket>buckets;
    QString secertID;
    QString secertKey;
signals:
    //void uploadProgress_signal(qint64, qint64);
private slots:
    //void uploadProgress_slot(qint64,qint64);
};

#endif // S3CLIENT_H
