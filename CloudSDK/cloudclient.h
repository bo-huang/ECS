#ifndef CLOUDCLIENT_H
#define CLOUDCLIENT_H
#include<QNetworkAccessManager>
#include<QObject>

//Cloud抽象类
class CloudClient : public QObject
{
public:
    CloudClient(QNetworkAccessManager *manger);
    //虚析构函数（当用基类指针释放子类资源时，需将基类的析构函数写成虚析构函数）
    virtual ~CloudClient(){}
    virtual QNetworkReply* GetObject(QString bucketName,QString objectName)=0;
    virtual QNetworkReply* PutObject(QString bucketName,QString objectName,QByteArray data)=0;
    virtual QNetworkReply* DeleteObject(QString bucketName,QString objectName)=0;
    virtual bool CreateBucket(QString bucketName,QString region,QString storageClass)=0;
    virtual QNetworkReply* CopyObject(QString bucketName, QString objectName, QString desBucketName)=0;
    virtual bool Login() = 0;
protected:
    QNetworkAccessManager *manger;
};

#endif // CLOUDCLIENT_H
