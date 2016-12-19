#include "baiduyunclient.h"
#include <QMessageAuthenticationCode>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QNetworkReply>

BaiduyunClient::BaiduyunClient(QNetworkAccessManager *manger,QString secertID,QString secertKey)
    :CloudClient(manger),secertID(secertID),secertKey(secertKey)
{

}

QNetworkReply* BaiduyunClient::GetObject(QString bucketName,QString objectName)
{
    QString datetime = GetCurrentDateTimeUtc();
    QString host = QString("%1.%2.bcebos.com").arg(bucketName,region);
    QString canonicalRequest = QString("GET\n/%1\n\nhost:%2").arg(objectName,host);
    QString authStringPrefix = QString("bce-auth-v1/%1/%2/1800").arg(secertID,datetime);
    QByteArray signingKey = QMessageAuthenticationCode::hash(authStringPrefix.toLatin1(),secertKey.toLatin1(),QCryptographicHash::Sha256).toHex();
    QByteArray signature =  QMessageAuthenticationCode::hash(canonicalRequest.toLatin1(),signingKey,QCryptographicHash::Sha256).toHex();
    QString authorization = authStringPrefix+"/host/"+signature;
    QString url = QString("https://%1/%2").arg(host,objectName);
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Authorization",authorization.toLatin1());
    QLocale lo = QLocale::English;//设置QLocale为英文
    QString date = lo.toString(QDateTime::currentDateTimeUtc(),"ddd, dd MMM yyyy hh:mm:ss")+" GMT";
    request.setRawHeader("Date",date.toLatin1());
    QNetworkReply *reply = manger->get(request);
    return reply;
}

//百度云的storageclass是对象级别，在上传object时设置
QNetworkReply* BaiduyunClient::PutObject(QString bucketName,QString objectName,QByteArray data)
{
    QString datetime = GetCurrentDateTimeUtc();
    QString storageclass = ParseStorageClass(GetStorageClass(bucketName));
    QString host = QString("%1.%2.bcebos.com").arg(bucketName,region);
    QString canonicalRequest = QString("PUT\n/%1\n\nhost:%2").arg(objectName,host);
    QString authStringPrefix = QString("bce-auth-v1/%1/%2/1800").arg(secertID,datetime);
    QByteArray signingKey = QMessageAuthenticationCode::hash(authStringPrefix.toLatin1(),secertKey.toLatin1(),QCryptographicHash::Sha256).toHex();
    QByteArray signature =  QMessageAuthenticationCode::hash(canonicalRequest.toLatin1(),signingKey,QCryptographicHash::Sha256).toHex();
    QString authorization = authStringPrefix+"/host/"+signature;
    QString url = QString("https://%1/%2").arg(host,objectName);
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Authorization",authorization.toLatin1());
    request.setRawHeader("x-bce-storage-class",storageclass.toLatin1());//STANDARD or STANDARD_IA
    QLocale lo = QLocale::English;//设置QLocale为英文
    QString date = lo.toString(QDateTime::currentDateTimeUtc(),"ddd, dd MMM yyyy hh:mm:ss")+" GMT";
    request.setRawHeader("Date",date.toLatin1());
    QNetworkReply *reply = manger->put(request,data);
    return reply;
}

QNetworkReply* BaiduyunClient::DeleteObject(QString bucketName,QString objectName)
{
    QString datetime = GetCurrentDateTimeUtc();
    QString host = QString("%1.%2.bcebos.com").arg(bucketName,region);
    QString canonicalRequest = QString("DELETE\n/%1\n\nhost:%2").arg(objectName,host);
    QString authStringPrefix = QString("bce-auth-v1/%1/%2/1800").arg(secertID,datetime);
    QByteArray signingKey = QMessageAuthenticationCode::hash(authStringPrefix.toLatin1(),secertKey.toLatin1(),QCryptographicHash::Sha256).toHex();
    QByteArray signature =  QMessageAuthenticationCode::hash(canonicalRequest.toLatin1(),signingKey,QCryptographicHash::Sha256).toHex();
    QString authorization = authStringPrefix+"/host/"+signature;
    QString url = QString("https://%1/%2").arg(host,objectName);
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Authorization",authorization.toLatin1());
    QLocale lo = QLocale::English;//设置QLocale为英文
    QString date = lo.toString(QDateTime::currentDateTimeUtc(),"ddd, dd MMM yyyy hh:mm:ss")+" GMT";
    request.setRawHeader("Date",date.toLatin1());
    QNetworkReply* reply = manger->deleteResource(request);
    return reply;
}

QNetworkReply* BaiduyunClient::CopyObject(QString bucketName, QString objectName, QString desBucketName)
{
    QString datetime = GetCurrentDateTimeUtc();
    QString host = QString("%1.%2.bcebos.com").arg(desBucketName,region);
    QString canonicalRequest = QString("PUT\n/%1\n\nhost:%2").arg(objectName,host);
    QString authStringPrefix = QString("bce-auth-v1/%1/%2/1800").arg(secertID,datetime);
    QByteArray signingKey = QMessageAuthenticationCode::hash(authStringPrefix.toLatin1(),secertKey.toLatin1(),QCryptographicHash::Sha256).toHex();
    QByteArray signature =  QMessageAuthenticationCode::hash(canonicalRequest.toLatin1(),signingKey,QCryptographicHash::Sha256).toHex();
    QString authorization = authStringPrefix+"/host/"+signature;
    QString url = QString("https://%1/%2").arg(host,objectName);
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Authorization",authorization.toLatin1());
    request.setRawHeader("x-bce-storage-class","STANDARD");//STANDARD or STANDARD_IA
    request.setRawHeader("x-bce-copy-source",QString("/%1/%2").arg(bucketName,objectName).toLatin1());
    QLocale lo = QLocale::English;//设置QLocale为英文
    QString date = lo.toString(QDateTime::currentDateTimeUtc(),"ddd, dd MMM yyyy hh:mm:ss")+" GMT";
    request.setRawHeader("Date",date.toLatin1());
    QNetworkReply *reply = manger->put(request,"");
    return reply;
}

bool BaiduyunClient::CreateBucket(QString bucketName, QString region, QString storageClass)
{
    QString datetime = GetCurrentDateTimeUtc();
    QString host = QString("%1.%2.bcebos.com").arg(bucketName,this->region);
    QString canonicalRequest = QString("PUT\n/\n\nhost:%1").arg(host);
    QString authStringPrefix = QString("bce-auth-v1/%1/%2/1800").arg(secertID,datetime);
    QByteArray signingKey = QMessageAuthenticationCode::hash(authStringPrefix.toLatin1(),secertKey.toLatin1(),QCryptographicHash::Sha256).toHex();
    QByteArray signature =  QMessageAuthenticationCode::hash(canonicalRequest.toLatin1(),signingKey,QCryptographicHash::Sha256).toHex();
    QString authorization = authStringPrefix+"/host/"+signature;
    QString url = "https://"+host;
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Authorization",authorization.toLatin1());
    QLocale lo = QLocale::English;//设置QLocale为英文
    QString date = lo.toString(QDateTime::currentDateTimeUtc(),"ddd, dd MMM yyyy hh:mm:ss")+" GMT";
    request.setRawHeader("Date",date.toLatin1());
    QNetworkReply *reply = manger->put(request,"");
    QEventLoop loop;
    connect(reply,SIGNAL(finished()),&loop,SLOT(quit()));
    loop.exec();
    reply->deleteLater();
    reply->close();
    if(reply->error()==QNetworkReply::NoError)
        return true;
    else
    {
        qDebug()<<"baiduyun createbucket error:"+reply->error();
        return false;
    }
}

QString BaiduyunClient::GetCurrentDateTimeUtc()
{
    QDateTime datetime = QDateTime::currentDateTimeUtc();
    QString sdatetime = datetime.toString(Qt::ISODate);
    return sdatetime;
}

inline QString BaiduyunClient::ParseStorageClass(QString storageClass)
{
    if(storageClass=="STANDARD")
        return "STANDARD";
    else if(storageClass=="STANDARD-IA")
        return "STANDARD_IA";
    else
        return "";
}

void BaiduyunClient::SetBuckets(std::vector<Bucket> &buckets)
{
    this->buckets = buckets;
}

QString BaiduyunClient::GetStorageClass(QString bucketName)
{
    QString storageClass;
    for(int i=0;i<buckets.size();i++)
        if(buckets[i].bucketName == bucketName)
        {
            storageClass = buckets[i].storageClass;
            break;
        }
    return storageClass;
}

bool BaiduyunClient::Login()
{
    QString datetime = GetCurrentDateTimeUtc();
    QString host = QString("bj.bcebos.com");
    QString canonicalRequest = QString("GET\n/\n\nhost:%1").arg(host);
    QString authStringPrefix = QString("bce-auth-v1/%1/%2/1800").arg(secertID,datetime);
    QByteArray signingKey = QMessageAuthenticationCode::hash(authStringPrefix.toLatin1(),secertKey.toLatin1(),QCryptographicHash::Sha256).toHex();
    QByteArray signature =  QMessageAuthenticationCode::hash(canonicalRequest.toLatin1(),signingKey,QCryptographicHash::Sha256).toHex();
    QString authorization = authStringPrefix+"/host/"+signature;
    QString url = QString("https://%1/").arg(host);
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Authorization",authorization.toLatin1());
    QLocale lo = QLocale::English;//设置QLocale为英文
    QString date = lo.toString(QDateTime::currentDateTimeUtc(),"ddd, dd MMM yyyy hh:mm:ss")+" GMT";
    request.setRawHeader("Date",date.toLatin1());
    QNetworkReply *reply = manger->get(request);
    QEventLoop loop;
    connect(reply,SIGNAL(finished()),&loop,SLOT(quit()));
    loop.exec();
    reply->deleteLater();
    reply->close();
    if(reply->error()==QNetworkReply::NoError)
        return true;
    else
        return false;
}
