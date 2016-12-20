#ifndef AZURECLIENT_H
#define AZURECLIENT_H
#include<CloudSDK/cloudclient.h>
#include<QObject>
#include<QNetworkAccessManager>

class AzureClient : public CloudClient
{
    Q_OBJECT
public:
    AzureClient(QNetworkAccessManager *manger,QString sharedKey,QString account);
    QNetworkReply* GetObject(QString bucketName,QString objectName) override;
    QNetworkReply* PutObject(QString bucketName,QString objectName,QByteArray data) override;
    QNetworkReply* CopyObject(QString bucketName, QString objectName, QString desBucketName) override;
    QNetworkReply* DeleteObject(QString bucketName,QString objectName) override;
    //这里region和storageclass 没用
    bool CreateBucket(QString bucketName, QString region, QString storageClass) override;
    bool Login() override;
    bool DeleteBucket(QString bucketName) override;
private:
    QString CreateHeader(
            QString Http_Verb, QString Content_Encoding, QString Content_Language, QString Content_Length,
            QString Content_MD5, QString Content_Type, QString Date, QString If_Modified_Since,
            QString If_Match, QString If_None_Match, QString If_Unmodified_Since, QString Range,
            QString CanonicalizedHeaders, QString CanonicalizedResource);
    QString GetCurrentTimeUTC();
private:
    QString sharedKey;
    QString account;
};

#endif // AZURECLIENT_H
