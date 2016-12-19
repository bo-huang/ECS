#include "CloudSDK/azureclient.h"
#include<QDateTime>
#include<QMessageAuthenticationCode>
#include<QNetworkRequest>
#include<QNetworkReply>
#include<QEventLoop>


AzureClient::AzureClient(QNetworkAccessManager *manger,QString sharedKey,QString account)
    :CloudClient(manger)
{
    this->account = account;
    this->sharedKey = sharedKey;
}
/*
 StringToSign = VERB + "\n" + Content-Encoding + "\n" + Content-Language + "\n" + Content-Length + "\n" + Content-MD5 + "\n" + Content-Type + "\n" + Date + "\n" + If-Modified-Since + "\n" + If-Match + "\n" + If-None-Match + "\n" + If-Unmodified-Since + "\n" + Range + "\n" + CanonicalizedHeaders + CanonicalizedResource;
*/
//多余的region和storageclass参数
bool AzureClient::CreateBucket(QString bucketName,QString region, QString storageClass)
{
    QString X_MS_DATE = GetCurrentTimeUTC();
    QString X_MS_VERSION = "2015-04-05";
    QString container = bucketName;
    QString canonicalizedResource =
            "/"+account+"/"+container+"\nrestype:container";
    QString canonicalizedHeaders =
            "x-ms-date:"+X_MS_DATE+"\n" + "x-ms-version:" + X_MS_VERSION;
    QString signature =
            CreateHeader("PUT","","","",
                         "","","","",
                         "","","","",
                         canonicalizedHeaders,canonicalizedResource);
    QByteArray authorizationHeader = QMessageAuthenticationCode::hash(
                QByteArray(signature.toUtf8()),
                QByteArray(QByteArray::fromBase64(sharedKey.toLatin1())),
                QCryptographicHash::Sha256).toBase64();
    QString url = QString("https://%1.blob.core.windows.net/%2?restype=container").arg(account,container);
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("Authorization","SharedKey "+account.toLatin1()+":" + authorizationHeader);
    request.setRawHeader("x-ms-date", X_MS_DATE.toLatin1());
    request.setRawHeader("x-ms-version", X_MS_VERSION.toLatin1());
    QNetworkReply *reply = manger->put(request,"");
    QEventLoop eventLoop;
    connect(reply,SIGNAL(finished()),&eventLoop,SLOT(quit()));
    eventLoop.exec();
    reply->deleteLater();
    reply->close();
    if(reply->error()==QNetworkReply::NoError)
        return true;
    else
    {
        qDebug()<<"azure create bucket error:"+reply->error();
        return false;
    }
}

QNetworkReply* AzureClient::PutObject(QString bucketName, QString objectName,QByteArray data)
{
    QString X_MS_DATE = GetCurrentTimeUTC();
    QString X_MS_VERSION = "2015-04-05";
    QString container = bucketName;
    QString blobType = "BlockBlob";//BlockBlob | PageBlob | AppendBlob
    QString contentLength = QString::number(data.size());
    QString canonicalizedResource =
            "/"+account+"/"+container+"/"+objectName;
    QString canonicalizedHeaders =
            "x-ms-blob-type:" + blobType + "\n" + "x-ms-date:"+X_MS_DATE+"\n" + "x-ms-version:" + X_MS_VERSION;
    QString signature =
            CreateHeader("PUT","","",contentLength,
                         "","","","",
                         "","","","",
                         canonicalizedHeaders,canonicalizedResource);
    QByteArray authorizationHeader = QMessageAuthenticationCode::hash(
                QByteArray(signature.toUtf8()),
                QByteArray(QByteArray::fromBase64(sharedKey.toLatin1())),
                QCryptographicHash::Sha256).toBase64();
    QString url = QString("https://%1.blob.core.windows.net/%2/%3").arg(account,container,objectName);
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("Authorization","SharedKey "+account.toLatin1()+":" + authorizationHeader);
    request.setRawHeader("x-ms-date", X_MS_DATE.toLatin1());
    request.setRawHeader("x-ms-version", X_MS_VERSION.toLatin1());
    request.setRawHeader("Content-Length",contentLength.toLatin1());
    request.setRawHeader("x-ms-blob-type","BlockBlob");
    QNetworkReply *reply = manger->put(request,data);
    return reply;
}

QNetworkReply* AzureClient::GetObject(QString bucketName, QString objectName)
{
    QString X_MS_DATE = GetCurrentTimeUTC();
    QString X_MS_VERSION = "2015-04-05";
    QString container = bucketName;
    QString canonicalizedResource =
            "/"+account+"/"+container+"/"+objectName;
    QString canonicalizedHeaders =
            "x-ms-date:"+X_MS_DATE+"\n" + "x-ms-version:" + X_MS_VERSION;
    QString signature =
            CreateHeader("GET","","","",
                         "","","","",
                         "","","","",
                         canonicalizedHeaders,canonicalizedResource);
    QByteArray authorizationHeader = QMessageAuthenticationCode::hash(
                QByteArray(signature.toUtf8()),
                QByteArray(QByteArray::fromBase64(sharedKey.toLatin1())),
                QCryptographicHash::Sha256).toBase64();
    QString url = QString("https://%1.blob.core.windows.net/%2/%3").arg(account,container,objectName);
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("Authorization","SharedKey "+account.toLatin1()+":" + authorizationHeader);
    request.setRawHeader("x-ms-date", X_MS_DATE.toLatin1());
    request.setRawHeader("x-ms-version", X_MS_VERSION.toLatin1());

    QNetworkReply *reply =  manger->get(request);
    return reply;
    /*QEventLoop eventloop;
    connect(reply,SIGNAL(finished()),&eventloop,SLOT(quit()));
    eventloop.exec();
    if(reply->error()==QNetworkReply::NoError)
    {
        return reply->readAll();
    }
    else
    {
        qDebug()<<"azure getobject error:"+reply->error();
        return NULL;
    }*/
}

QNetworkReply* AzureClient::CopyObject(QString bucketName, QString objectName, QString desBucketName)
{
    QString X_MS_DATE = GetCurrentTimeUTC();
    QString X_MS_VERSION = "2015-04-05";
    QString X_MS_COPY_SOURCE = QString("https://%1.blob.core.windows.net/%2/%3")
            .arg(account,bucketName,objectName);
    QString blobType = "BlockBlob";//BlockBlob | PageBlob | AppendBlob
    QString canonicalizedResource =
            "/"+account+"/"+desBucketName+"/"+objectName;
    QString canonicalizedHeaders =
            "x-ms-blob-type:" + blobType + "\n" +"x-ms-copy-source:" + X_MS_COPY_SOURCE + "\n" + "x-ms-date:"+X_MS_DATE+"\n" + "x-ms-version:" + X_MS_VERSION;
    QString signature =
            CreateHeader("PUT","","","",
                         "","","","",
                         "","","","",
                         canonicalizedHeaders,canonicalizedResource);
    QByteArray authorizationHeader = QMessageAuthenticationCode::hash(
                QByteArray(signature.toUtf8()),
                QByteArray(QByteArray::fromBase64(sharedKey.toLatin1())),
                QCryptographicHash::Sha256).toBase64();
    QString url = QString("https://%1.blob.core.windows.net/%2/%3").arg(account,desBucketName,objectName);
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("Authorization","SharedKey "+account.toLatin1()+":" + authorizationHeader);
    request.setRawHeader("x-ms-date", X_MS_DATE.toLatin1());
    request.setRawHeader("x-ms-version", X_MS_VERSION.toLatin1());
    request.setRawHeader("x-ms-blob-type","BlockBlob");
    request.setRawHeader("x-ms-copy-source",X_MS_COPY_SOURCE.toLatin1());
    QNetworkReply *reply = manger->put(request,"");
    return reply;
}

QNetworkReply* AzureClient::DeleteObject(QString bucketName, QString objectName)
{
    QString X_MS_DATE = GetCurrentTimeUTC();
    QString X_MS_VERSION = "2015-04-05";
    QString container = bucketName;
    QString canonicalizedResource =
            "/"+account+"/"+container+"/"+objectName;
    QString canonicalizedHeaders =
            "x-ms-date:"+X_MS_DATE+"\n" + "x-ms-version:" + X_MS_VERSION;
    QString signature =
            CreateHeader("DELETE","","","",
                         "","","","",
                         "","","","",
                         canonicalizedHeaders,canonicalizedResource);
    QByteArray authorizationHeader = QMessageAuthenticationCode::hash(
                QByteArray(signature.toUtf8()),
                QByteArray(QByteArray::fromBase64(sharedKey.toLatin1())),
                QCryptographicHash::Sha256).toBase64();
    QString url = QString("https://%1.blob.core.windows.net/%2/%3").arg(account,container,objectName);
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("Authorization","SharedKey "+account.toLatin1()+":" + authorizationHeader);
    request.setRawHeader("x-ms-date", X_MS_DATE.toLatin1());
    request.setRawHeader("x-ms-version", X_MS_VERSION.toLatin1());
    QNetworkReply* reply = manger->deleteResource(request);
    return reply;
}

bool AzureClient::Login()
{
    QString X_MS_DATE = GetCurrentTimeUTC();
    QString X_MS_VERSION = "2015-04-05";
    QString canonicalizedResource ="/"+account+"/\ncomp:list";
    QString canonicalizedHeaders =
            "x-ms-date:"+X_MS_DATE+"\n" + "x-ms-version:" + X_MS_VERSION;
    QString signature =
            CreateHeader("GET","","","",
                         "","","","",
                         "","","","",
                         canonicalizedHeaders,canonicalizedResource);
    QByteArray authorizationHeader = QMessageAuthenticationCode::hash(
                QByteArray(signature.toUtf8()),
                QByteArray(QByteArray::fromBase64(sharedKey.toLatin1())),
                QCryptographicHash::Sha256).toBase64();
    QString url = QString("https://%1.blob.core.windows.net/?comp=list").arg(account);
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("Authorization","SharedKey "+account.toLatin1()+":" + authorizationHeader);
    request.setRawHeader("x-ms-date", X_MS_DATE.toLatin1());
    request.setRawHeader("x-ms-version", X_MS_VERSION.toLatin1());

    QNetworkReply *reply =  manger->get(request);
    QEventLoop eventloop;
    connect(reply,SIGNAL(finished()),&eventloop,SLOT(quit()));
    eventloop.exec();
    if(reply->error()==QNetworkReply::NoError)
        return true;
    else
        return false;
}

QString AzureClient::CreateHeader(
        QString Http_Verb, QString Content_Encoding, QString Content_Language, QString Content_Length,
        QString Content_MD5, QString Content_Type, QString Date, QString If_Modified_Since,
        QString If_Match, QString If_None_Match, QString If_Unmodified_Since, QString Range,
        QString CanonicalizedHeaders, QString CanonicalizedResource){

    QString result = "";

    result = result + Http_Verb + "\n";
    result = result + Content_Encoding + "\n";
    result = result + Content_Language + "\n";
    result = result + Content_Length + "\n";
    result = result + Content_MD5 + "\n";
    result = result + Content_Type + "\n";
    result = result + Date + "\n";
    result = result + If_Modified_Since + "\n";
    result = result + If_Match + "\n";
    result = result + If_None_Match + "\n";
    result = result + If_Unmodified_Since + "\n";
    result = result + Range + "\n";
    result = result + CanonicalizedHeaders + "\n";
    result = result + CanonicalizedResource;

    return result;
}

QString AzureClient::GetCurrentTimeUTC(){
    QLocale lo = QLocale::English;//设置QLocale为英文
    return lo.toString(QDateTime::currentDateTimeUtc(),"ddd, dd MMM yyyy hh:mm:ss")+" GMT";
}
