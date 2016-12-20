#include "CloudSDK/aliyunclient.h"
#include<QMessageAuthenticationCode>
#include<QNetworkRequest>
#include<QNetworkReply>
#include<QEventLoop>

AliyunClient::AliyunClient(QNetworkAccessManager *manger,QString accessKeyID,QString accessKeySecert)
    :CloudClient(manger)
{
    this->accessKeyID = accessKeyID;
    this->accessKeySecert = accessKeySecert;
}
//VERB + "\n" + CONTENT-MD5 + "\n" + CONTENT-TYPE + "\n" + DATE + "\n" + CanonicalizedOSSHeaders + CanonicalizedResource
bool AliyunClient::CreateBucket(QString bucketName, QString region,QString storageClass)
{
    region = ParseLoaction(region);
    QString date = GetCurrentTimeUTC();
    QString canonicalizedHeaders = "x-oss-acl:private\n";
    QString canonicalizedResource = "/"+bucketName+"/";
    QString signature =
            CreateHeader("PUT","","",date,canonicalizedHeaders,canonicalizedResource);
    QByteArray authorization = QMessageAuthenticationCode::hash(
                signature.toLatin1(),accessKeySecert.toLatin1(),QCryptographicHash::Sha1).toBase64();
    QString url = QString("http://%1.%2.aliyuncs.com").arg(bucketName,region);
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Date",date.toLatin1());
    request.setRawHeader("x-oss-acl","private");
    request.setRawHeader("Authorization","OSS "+accessKeyID.toLatin1()+":"+authorization);
    QByteArray postData = "<CreateBucketConfiguration >"
            "<LocationConstraint >"+region.toLatin1()+"</LocationConstraint >"
            "</CreateBucketConfiguration >";
    QNetworkReply *reply = manger->put(request,postData);
    QEventLoop loop;
    connect(reply,SIGNAL(finished()),&loop,SLOT(quit()));
    loop.exec();
    reply->deleteLater();
    reply->close();
    if(reply->error()==QNetworkReply::NoError)
        return true;
    else
    {
        qDebug()<<"aliyun createbucket error:"+reply->error();
        return false;
    }
}

QNetworkReply* AliyunClient::PutObject(QString bucketName, QString objectName,QByteArray data)
{
    QString date = GetCurrentTimeUTC();
    QString canonicalizedHeaders = "x-oss-object-acl:private\n";
    QString canonicalizedResource = "/"+bucketName+"/"+objectName;
    QString signature =
            CreateHeader("PUT","","",date,canonicalizedHeaders,canonicalizedResource);
    QByteArray authorization = QMessageAuthenticationCode::hash(
                signature.toLatin1(),accessKeySecert.toLatin1(),QCryptographicHash::Sha1).toBase64();
    QString region = "oss-cn-hangzhou";
    QString url = QString("http://%1.%2.aliyuncs.com/%3").arg(bucketName,region,objectName);
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Date",date.toLatin1());
    request.setRawHeader("x-oss-object-acl","private");
    request.setRawHeader("Content-Length",QByteArray::number(data.size()));
    request.setRawHeader("Authorization","OSS "+accessKeyID.toLatin1()+":"+authorization);
    QNetworkReply *reply = manger->put(request,data);
    return reply;
}

QNetworkReply* AliyunClient::GetObject(QString bucketName, QString objectName)
{
    QString date = GetCurrentTimeUTC();
    QString canonicalizedHeaders = "";
    QString canonicalizedResource = "/"+bucketName+"/"+objectName;
    QString signature =
            CreateHeader("GET","","",date,canonicalizedHeaders,canonicalizedResource);
    QByteArray authorization = QMessageAuthenticationCode::hash(
                signature.toLatin1(),accessKeySecert.toLatin1(),QCryptographicHash::Sha1).toBase64();
    QString region = "oss-cn-hangzhou";
    QString url = QString("http://%1.%2.aliyuncs.com/%3").arg(bucketName,region,objectName);
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Date",date.toLatin1());
    request.setRawHeader("Authorization","OSS "+accessKeyID.toLatin1()+":"+authorization);
    QNetworkReply *reply = manger->get(request);
    return reply;
    /*QEventLoop loop;
    connect(reply,SIGNAL(finished()),&loop,SLOT(quit()));
    loop.exec();
    if(reply->error()==QNetworkReply::NoError)
        return reply->readAll();
    else
    {
        qDebug()<<"aliyun getobject error:"+reply->error();
        return NULL;
    }*/
}

QNetworkReply* AliyunClient::CopyObject(QString bucketName, QString objectName, QString desBucketName)
{
    QString date = GetCurrentTimeUTC();
    QString x_oss_copy_source = QString("/%1/%2").arg(bucketName,objectName);
    QString canonicalizedHeaders = QString("x-oss-copy-source:%1\nx-oss-object-acl:private\n").arg(x_oss_copy_source);
    QString canonicalizedResource = "/"+desBucketName+"/"+objectName;
    QString signature =
            CreateHeader("PUT","","",date,canonicalizedHeaders,canonicalizedResource);
    QByteArray authorization = QMessageAuthenticationCode::hash(
                signature.toLatin1(),accessKeySecert.toLatin1(),QCryptographicHash::Sha1).toBase64();
    QString region = "oss-cn-hangzhou";
    QString url = QString("http://%1.%2.aliyuncs.com/%3").arg(desBucketName,region,objectName);
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Date",date.toLatin1());
    request.setRawHeader("x-oss-object-acl","private");
    request.setRawHeader("x-oss-copy-source",x_oss_copy_source.toLatin1());
    request.setRawHeader("Authorization","OSS "+accessKeyID.toLatin1()+":"+authorization);
    QNetworkReply *reply = manger->put(request,"");
    return reply;
}

QNetworkReply* AliyunClient::DeleteObject(QString bucketName, QString objectName)
{
    QString date = GetCurrentTimeUTC();
    QString canonicalizedHeaders = "";
    QString canonicalizedResource = "/"+bucketName+"/"+objectName;
    QString signature =
            CreateHeader("DELETE","","",date,canonicalizedHeaders,canonicalizedResource);
    QByteArray authorization = QMessageAuthenticationCode::hash(
                signature.toLatin1(),accessKeySecert.toLatin1(),QCryptographicHash::Sha1).toBase64();
    QString region = "oss-cn-hangzhou";
    QString url = QString("http://%1.%2.aliyuncs.com/%3").arg(bucketName,region,objectName);
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Date",date.toLatin1());
    request.setRawHeader("Authorization","OSS "+accessKeyID.toLatin1()+":"+authorization);
    QNetworkReply* reply = manger->deleteResource(request);
    return reply;
}


QString AliyunClient::CreateHeader(
        QString httpVerb,QString contentMD5, QString contentType, QString Date,
        QString canonicalizedHeaders, QString canonicalizedResource)
{
    QString result = "";
    result = result + httpVerb + "\n";
    result = result + contentMD5 + "\n";
    result = result + contentType + "\n";
    result = result + Date + "\n";
    result = result + canonicalizedHeaders;
    result = result + canonicalizedResource;
    return result;
}

QString AliyunClient::GetCurrentTimeUTC(){
    QLocale lo = QLocale::English;//设置QLocale为英文
    return lo.toString(QDateTime::currentDateTimeUtc(),"ddd, dd MMM yyyy hh:mm:ss")+" GMT";
}

QString AliyunClient::ParseLoaction(QString region)
{
    if(region=="Asia")
        return "oss-cn-hangzhou";
}

bool AliyunClient::Login()
{
    QString date = GetCurrentTimeUTC();
    QString canonicalizedHeaders = "";
    QString canonicalizedResource = "/";
    QString signature =
            CreateHeader("GET","","",date,canonicalizedHeaders,canonicalizedResource);
    QByteArray authorization = QMessageAuthenticationCode::hash(
                signature.toLatin1(),accessKeySecert.toLatin1(),QCryptographicHash::Sha1).toBase64();
    QString url = QString("http://oss.aliyuncs.com/");
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Date",date.toLatin1());
    request.setRawHeader("Authorization","OSS "+accessKeyID.toLatin1()+":"+authorization);
    QNetworkReply *reply = manger->get(request);
    QEventLoop loop;
    connect(reply,SIGNAL(finished()),&loop,SLOT(quit()));
    loop.exec();
    if(reply->error()==QNetworkReply::NoError)
        return true;
    else
        return false;
}
//bucket必须为空
bool AliyunClient::DeleteBucket(QString bucketName)
{
    QString date = GetCurrentTimeUTC();
    QString canonicalizedHeaders = "";
    QString canonicalizedResource = "/"+bucketName+"/";
    QString signature =
            CreateHeader("DELETE","","",date,canonicalizedHeaders,canonicalizedResource);
    QByteArray authorization = QMessageAuthenticationCode::hash(
                signature.toLatin1(),accessKeySecert.toLatin1(),QCryptographicHash::Sha1).toBase64();
    QString region = "oss-cn-hangzhou";
    QString url = QString("http://%1.%2.aliyuncs.com").arg(bucketName,region);
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("Date",date.toLatin1());
    request.setRawHeader("Authorization","OSS "+accessKeyID.toLatin1()+":"+authorization);
    QEventLoop loop;
    QNetworkReply* reply = manger->deleteResource(request);
    connect(reply,SIGNAL(finished()),&loop,SLOT(quit()));
    loop.exec();
    reply->deleteLater();
    reply->close();
    if(reply->error()==QNetworkReply::NoError)
        return true;
    return false;
}
