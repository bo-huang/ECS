#ifndef GOOGLECLIENT_H
#define GOOGLECLIENT_H
#include "CloudSDK/cloudclient.h"
#include<QNetworkAccessManager>
#include<QObject>

class GoogleClient : public CloudClient
{
    Q_OBJECT
public:
    GoogleClient(QNetworkAccessManager *manger,QString secertID,QString secertKey);
    GoogleClient(QNetworkAccessManager *manger, const char *jsonPath);
    void SetProjectID(QString projectID);
    void SetRefreshToken(QString refreshToken);
    QNetworkReply* GetObject(QString bucketName,QString objectName) override;
    QNetworkReply* PutObject(QString bucketName,QString objectName,QByteArray data) override;
    QNetworkReply* CopyObject(QString bucketName, QString objectName, QString desBucketName) override;
    QNetworkReply* DeleteObject(QString bucketName,QString objectName) override;
    bool CreateBucket(QString bucketName, QString region, QString storageClass) override;
    bool Login();
    QString GetRefreshToken();
private:
    //获取下载链接
    QString GetmediaLink(QString bucketName,QString objectName);
    QString GetAccessTokenByRefreshToken();
    QString ParseLoaction(QString region);
    QString ParseStorageClass(QString storageClass);
signals:
    //void uploadProgress_signal(qint64,qint64);
private slots:
    //void uploadProgress_slot(qint64,qint64);
private:
    static QString access_token;
    static QString refresh_token;
    static QTime expires_time;//记录access_token过期时间
    QString secertID;
    QString secertKey;
    QString projectID;
    const char *jsonPath;
};

#endif // GOOGLECLIENT_H
