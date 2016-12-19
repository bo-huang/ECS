#include"CloudSDK/s3client.h"
#include <QEventLoop>
#include<QNetworkReply>
#include<QMessageAuthenticationCode>
#include<iostream>

S3Client::S3Client(QNetworkAccessManager *manger,QString secertID,QString secertKey)
    :CloudClient(manger)
{
    this->manger=manger;
    this->secertID=secertID;
    this->secertKey=secertKey;
}

bool S3Client::CreateBucket(QString bucketName, QString region, QString storageClass)
{
    region = ParseLoaction(region);
    //如果区域选择US STANDARD（即 us-east-1 ），则不要写约束！
    QByteArray  data = "";
    if(region != "us-east-1")
    {
        data = "<CreateBucketConfiguration xmlns=\"http://s3.amazonaws.com/doc/2006-03-01/\"> "
            "<LocationConstraint>"+region.toLatin1()+"</LocationConstraint> "
          "</CreateBucketConfiguration >";
    }
    QNetworkRequest request = GetRequestForCreateBucket(bucketName,data);
    QNetworkReply *reply = manger->put(request,data);
    QEventLoop eventLoop;
    connect(reply,SIGNAL(finished()),&eventLoop,SLOT(quit()));
    eventLoop.exec();
    reply->deleteLater();
    reply->close();
    if (reply->error() == QNetworkReply::NoError)
        return true;
    else
    {
        qDebug()<<"createbucket error: "<<reply->error();
        return false;
    }
}

QNetworkReply* S3Client::GetObject(QString bucketName,QString objectName)
{
   QNetworkRequest request = GetRequestForGetObject(bucketName,objectName);
   QNetworkReply *reply = manger->get(request);
   return reply;
   /*QEventLoop eventLoop;
   connect(reply,SIGNAL(finished()),&eventLoop,SLOT(quit()));
   eventLoop.exec();
   //reply->close();
   //reply->deleteLater();
   if (reply->error() == QNetworkReply::NoError)
   {
       return reply->readAll();
   }
   else
   {
        qDebug() << "s3 getobject error:" << reply->error();
        return NULL;
   }*/
}
QNetworkReply* S3Client::PutObject(QString bucketName,QString objectName,QByteArray data)
{
    QNetworkRequest request = GetRequestForPutObject(bucketName,objectName,data);
    QNetworkReply *reply = manger->put(request,data);
    return reply;
}
QNetworkReply* S3Client::CopyObject(QString bucketName, QString objectName, QString desBucketName)
{
    QNetworkRequest request = GetRequestForCopyObject(bucketName,objectName,desBucketName);
    QNetworkReply *reply = manger->put(request,"");
    return reply;
}
QNetworkReply* S3Client::DeleteObject(QString bucketName, QString objectName)
{
    QNetworkRequest request = GetRequestForDeleteObject(bucketName,objectName);
    QNetworkReply* reply = manger->deleteResource(request);
    return reply;
}

bool S3Client::Login()
{
    QByteArray date = GetDate().toLatin1();
    QByteArray datetime = GetDateTime().toLatin1();
    QByteArray region = "us-east-1";
    QByteArray host = "s3.amazonaws.com";
    QByteArray canonicalRequest = "GET\n/\n\n";
    canonicalRequest+="host:"+host+"\n";
    canonicalRequest+="x-amz-content-sha256:e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\n";
    canonicalRequest+="x-amz-date:"+datetime+"\n\n";
    canonicalRequest+="host;x-amz-content-sha256;x-amz-date\n";
    canonicalRequest+="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

    QByteArray stringToSign = "AWS4-HMAC-SHA256\n"+datetime+"\n"+date+"/"+region+"/s3/aws4_request\n";
    stringToSign+=QCryptographicHash::hash(canonicalRequest,QCryptographicHash::Sha256).toHex();
    QByteArray kSigning = GetSignatureKey(date,region);
    QByteArray signature = QMessageAuthenticationCode::hash(stringToSign,kSigning, QCryptographicHash::Sha256).toHex();
    QByteArray authorization = "AWS4-HMAC-SHA256 Credential="+secertID.toLatin1()+"/"+date+"/"+region+"/s3/aws4_request,SignedHeaders=host;x-amz-content-sha256;x-amz-date,Signature="+signature;

    QNetworkRequest request(QUrl("http://s3.amazonaws.com/"));
    request.setRawHeader("Host",host);
    request.setRawHeader("x-amz-date",datetime);
    request.setRawHeader("Authorization",authorization);
    request.setRawHeader("x-amz-content-sha256","e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");

    QNetworkReply *reply = manger->get(request);
    QEventLoop eventLoop;
    connect(reply,SIGNAL(finished()),&eventLoop,SLOT(quit()));
    eventLoop.exec();
    reply->deleteLater();
    reply->close();
    if (reply->error() == QNetworkReply::NoError)
        return true;
    else
        return false;
}

/*
QByteArray S3Client::GetObjectAcl(QString bucketName, QString objectName)
{
    QNetworkRequest request = GetRequestForGetObjectAcl(bucketName,objectName);
    QNetworkReply *reply = manger->get(request);
    QEventLoop eventLoop;
    connect(reply,SIGNAL(finished()),&eventLoop,SLOT(quit()));
    eventLoop.exec();
    reply->close();
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError)
    {
        return reply->readAll();
    }
    else
    {
         qDebug() << "s3 getobject error:" << reply->error();
         return NULL;
    }
}
void S3Client::SetObjectAcl(QString bucketName, QString objectName, QByteArray aclStr)
{
    QNetworkRequest request = GetRequestForSetObjectAcl(bucketName,objectName,aclStr);
    manger->post(request,aclStr);
}
*/
QNetworkRequest S3Client::GetRequestForCreateBucket(QString bucketName,QByteArray data)
{
    QByteArray date = GetDate().toLatin1();
    QByteArray datetime = GetDateTime().toLatin1();
    QByteArray region = "us-east-1";//无论bucket建在哪里，都是us-east-1
    QByteArray contentsha256 = QCryptographicHash::hash(data,QCryptographicHash::Sha256).toHex();
    QByteArray canonicalRequest = "PUT\n/\n\n";
    canonicalRequest+="host:"+bucketName+".s3.amazonaws.com\n";
    canonicalRequest+="x-amz-content-sha256:"+contentsha256+"\n";
    canonicalRequest+="x-amz-date:"+datetime+"\n\n";
    canonicalRequest+="host;x-amz-content-sha256;x-amz-date\n";
    canonicalRequest+=contentsha256;

    QByteArray stringToSign = "AWS4-HMAC-SHA256\n"+datetime+"\n"+date+"/"+region+"/s3/aws4_request\n";
    stringToSign+=QCryptographicHash::hash(canonicalRequest,QCryptographicHash::Sha256).toHex();
    QByteArray kSigning = GetSignatureKey(date,region);
    QByteArray signature = QMessageAuthenticationCode::hash(stringToSign,kSigning, QCryptographicHash::Sha256).toHex();
    QByteArray authorization = "AWS4-HMAC-SHA256 Credential="+secertID.toLatin1()+"/"+date+"/"+region+"/s3/aws4_request,SignedHeaders=host;x-amz-content-sha256;x-amz-date,Signature="+signature;

    QNetworkRequest request(QUrl("http://"+bucketName+".s3.amazonaws.com"));
    request.setRawHeader("Host",bucketName.toLatin1()+".s3.amazonaws.com");
    request.setRawHeader("x-amz-date",datetime);
    request.setRawHeader("Authorization",authorization);
    request.setRawHeader("x-amz-content-sha256",contentsha256);
    request.setHeader(QNetworkRequest::ContentLengthHeader,"application/xml");

    return request;
}

QNetworkRequest S3Client::GetRequestForGetObject(QString bucketName,QString objectName)
{
    QByteArray date = GetDate().toLatin1();
    QByteArray datetime = GetDateTime().toLatin1();
    QString region_str = ParseLoaction(GetRegion(bucketName));
    //QString region_str = "ap-southeast-1";
    QByteArray region = region_str.toLatin1();
    QByteArray host = GetHost(region_str,bucketName).toLatin1();
    QByteArray canonicalRequest = "GET\n/";
    canonicalRequest+=objectName+"\n\n";
    canonicalRequest+="host:"+host+"\n";
    canonicalRequest+="x-amz-content-sha256:e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\n";
    canonicalRequest+="x-amz-date:"+datetime+"\n\n";
    canonicalRequest+="host;x-amz-content-sha256;x-amz-date\n";
    canonicalRequest+="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

    QByteArray stringToSign = "AWS4-HMAC-SHA256\n"+datetime+"\n"+date+"/"+region+"/s3/aws4_request\n";
    stringToSign+=QCryptographicHash::hash(canonicalRequest,QCryptographicHash::Sha256).toHex();
    QByteArray kSigning = GetSignatureKey(date,region);
    QByteArray signature = QMessageAuthenticationCode::hash(stringToSign,kSigning, QCryptographicHash::Sha256).toHex();
    QByteArray authorization = "AWS4-HMAC-SHA256 Credential="+secertID.toLatin1()+"/"+date+"/"+region+"/s3/aws4_request,SignedHeaders=host;x-amz-content-sha256;x-amz-date,Signature="+signature;

    QNetworkRequest request(QUrl("http://"+host+"/"+objectName));
    request.setRawHeader("Host",host);
    request.setRawHeader("x-amz-date",datetime);
    request.setRawHeader("Authorization",authorization);
    request.setRawHeader("x-amz-content-sha256","e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    return request;
}
QNetworkRequest S3Client::GetRequestForPutObject(QString bucketName,QString objectName,QByteArray data)
{
    QByteArray date = GetDate().toLatin1();
    QByteArray datetime = GetDateTime().toLatin1();
    QString region_str = ParseLoaction(GetRegion(bucketName));
    //QString region_str = "ap-southeast-1";//暂时设为新家坡
    QByteArray region = region_str.toLatin1();
    QByteArray host = GetHost(region_str,bucketName).toLatin1();
    //S3在object class level设置storageClass
    QByteArray storageClass = ParseStorageClass(GetStorageClass(bucketName)).toLatin1();
    QByteArray canonicalRequest = "PUT\n/";
    QByteArray contentsha256 = QCryptographicHash::hash(data,QCryptographicHash::Sha256).toHex();
    canonicalRequest+=objectName+"\n\n";
    canonicalRequest+="host:"+host+"\n";
    canonicalRequest+="x-amz-content-sha256:"+contentsha256+"\n";
    canonicalRequest+="x-amz-date:"+datetime+"\n";
    canonicalRequest+="x-amz-storage-class:"+storageClass+"\n\n";
    canonicalRequest+="host;x-amz-content-sha256;x-amz-date;x-amz-storage-class\n";
    canonicalRequest+=contentsha256;

    QByteArray stringToSign = "AWS4-HMAC-SHA256\n"+datetime+"\n"+date+"/"+region+"/s3/aws4_request\n";
    stringToSign+=QCryptographicHash::hash(canonicalRequest,QCryptographicHash::Sha256).toHex();
    QByteArray kSigning = GetSignatureKey(date,region);
    QByteArray signature = QMessageAuthenticationCode::hash(stringToSign,kSigning, QCryptographicHash::Sha256).toHex();
    QByteArray authorization = "AWS4-HMAC-SHA256 Credential="+secertID.toLatin1()+"/"+date+"/"+region+"/s3/aws4_request,SignedHeaders=host;x-amz-content-sha256;x-amz-date;x-amz-storage-class,Signature="+signature;

    QNetworkRequest request(QUrl("http://"+host+"/"+objectName));
    request.setRawHeader("Host",host);
    request.setRawHeader("x-amz-date",datetime);
    request.setRawHeader("Authorization",authorization);
    request.setRawHeader("x-amz-content-sha256",contentsha256);
    request.setRawHeader("x-amz-storage-class",storageClass);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"image/jpeg");
    return request;
}
QNetworkRequest S3Client::GetRequestForCopyObject(QString bucketName, QString objectName, QString desBucketName)
{
    QByteArray date = GetDate().toLatin1();
    QByteArray datetime = GetDateTime().toLatin1();
    QString region_str = ParseLoaction(GetRegion(bucketName));
    QByteArray region = region_str.toLatin1();
    QByteArray host = GetHost(region_str,desBucketName).toLatin1();
    QByteArray copy_source = QString("/%1/%2").arg(bucketName,objectName).toLatin1();
    //S3在object class level设置storageClass
    QByteArray storageClass = ParseStorageClass(GetStorageClass(bucketName)).toLatin1();
    QByteArray canonicalRequest = "PUT\n/";
    QByteArray contentsha256 = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    canonicalRequest+=objectName+"\n\n";
    canonicalRequest+="host:"+host+"\n";
    canonicalRequest+="x-amz-content-sha256:"+contentsha256+"\n";
    canonicalRequest+="x-amz-copy-source:"+copy_source+"\n";
    canonicalRequest+="x-amz-date:"+datetime+"\n";
    canonicalRequest+="x-amz-storage-class:"+storageClass+"\n\n";
    canonicalRequest+="host;x-amz-content-sha256;x-amz-copy-source;x-amz-date;x-amz-storage-class\n";
    canonicalRequest+=contentsha256;

    QByteArray stringToSign = "AWS4-HMAC-SHA256\n"+datetime+"\n"+date+"/"+region+"/s3/aws4_request\n";
    stringToSign+=QCryptographicHash::hash(canonicalRequest,QCryptographicHash::Sha256).toHex();
    QByteArray kSigning = GetSignatureKey(date,region);
    QByteArray signature = QMessageAuthenticationCode::hash(stringToSign,kSigning, QCryptographicHash::Sha256).toHex();
    QByteArray authorization = "AWS4-HMAC-SHA256 Credential="+secertID.toLatin1()+"/"+date+"/"+region+"/s3/aws4_request,SignedHeaders=host;x-amz-content-sha256;x-amz-copy-source;x-amz-date;x-amz-storage-class,Signature="+signature;

    QNetworkRequest request(QUrl("http://"+host+"/"+objectName));
    request.setRawHeader("Host",host);
    request.setRawHeader("x-amz-date",datetime);
    request.setRawHeader("Authorization",authorization);
    request.setRawHeader("x-amz-content-sha256",contentsha256);
    request.setRawHeader("x-amz-copy-source",copy_source);
    request.setRawHeader("x-amz-storage-class",storageClass);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"image/jpeg");
    return request;
}
QNetworkRequest S3Client::GetRequestForDeleteObject(QString bucketName, QString objectName)
{
    QByteArray date = GetDate().toLatin1();
    QByteArray datetime = GetDateTime().toLatin1();
    QString region_str = ParseLoaction(GetRegion(bucketName));
    //QString region_str = "ap-southeast-1";
    QByteArray region = region_str.toLatin1();
    QByteArray host = GetHost(region_str,bucketName).toLatin1();
    QByteArray canonicalRequest = "DELETE\n/";
    canonicalRequest+=objectName+"\n\n";
    canonicalRequest+="host:"+host+"\n";
    canonicalRequest+="x-amz-content-sha256:e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\n";
    canonicalRequest+="x-amz-date:"+datetime+"\n\n";
    canonicalRequest+="host;x-amz-content-sha256;x-amz-date\n";
    canonicalRequest+="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

    QByteArray stringToSign = "AWS4-HMAC-SHA256\n"+datetime+"\n"+date+"/"+region+"/s3/aws4_request\n";
    stringToSign+=QCryptographicHash::hash(canonicalRequest,QCryptographicHash::Sha256).toHex();
    QByteArray kSigning = GetSignatureKey(date,region);
    QByteArray signature = QMessageAuthenticationCode::hash(stringToSign,kSigning, QCryptographicHash::Sha256).toHex();
    QByteArray authorization = "AWS4-HMAC-SHA256 Credential="+secertID.toLatin1()+"/"+date+"/"+region+"/s3/aws4_request,SignedHeaders=host;x-amz-content-sha256;x-amz-date,Signature="+signature;

    QNetworkRequest request(QUrl("http://"+host+"/"+objectName));
    request.setRawHeader("Host",host);
    request.setRawHeader("x-amz-date",datetime);
    request.setRawHeader("Authorization",authorization);
    request.setRawHeader("x-amz-content-sha256","e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");

    return request;
}
/*
QNetworkRequest S3Client::GetRequestForGetObjectAcl(QString bucketName, QString objectName)
{
    QByteArray date = GetDate().toLatin1();
    QByteArray datetime = GetDateTime().toLatin1();
    QString region_str = ParseLoaction(GetRegion(bucketName));
    QByteArray region = region_str.toLatin1();
    QByteArray host = GetHost(region_str,bucketName).toLatin1();
    QByteArray canonicalRequest = "GET\n/";
    canonicalRequest+=objectName+"\n";
    canonicalRequest+="acl=\n";//带参数
    canonicalRequest+="host:"+host+"\n";
    canonicalRequest+="x-amz-content-sha256:e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855\n";
    canonicalRequest+="x-amz-date:"+datetime+"\n\n";
    canonicalRequest+="host;x-amz-content-sha256;x-amz-date\n";
    canonicalRequest+="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

    QByteArray stringToSign = "AWS4-HMAC-SHA256\n"+datetime+"\n"+date+"/"+region+"/s3/aws4_request\n";
    stringToSign+=QCryptographicHash::hash(canonicalRequest,QCryptographicHash::Sha256).toHex();
    QByteArray kSigning = GetSignatureKey(date,region);
    QByteArray signature = QMessageAuthenticationCode::hash(stringToSign,kSigning, QCryptographicHash::Sha256).toHex();
    QByteArray authorization = "AWS4-HMAC-SHA256 Credential="+secertID.toLatin1()+"/"+date+"/"+region+"/s3/aws4_request,SignedHeaders=host;x-amz-content-sha256;x-amz-date,Signature="+signature;

    QNetworkRequest request(QUrl("http://"+host+"/"+objectName+"?acl"));
    request.setRawHeader("Host",host);
    request.setRawHeader("x-amz-date",datetime);
    request.setRawHeader("Authorization",authorization);
    request.setRawHeader("x-amz-content-sha256","e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    qDebug()<<request.url();
    qDebug()<<request.rawHeader("Host");
    qDebug()<<request.rawHeader("x-amz-date");
    qDebug()<<request.rawHeader("Authorization");
    return request;
}
QNetworkRequest S3Client::GetRequestForSetObjectAcl(QString bucketName, QString objectName, QByteArray aclStr)
{
    QByteArray date = GetDate().toLatin1();
    QByteArray datetime = GetDateTime().toLatin1();
    QString region_str = ParseLoaction(GetRegion(bucketName));
    QByteArray region = region_str.toLatin1();
    QByteArray host = GetHost(region_str,bucketName).toLatin1();

    QByteArray canonicalRequest = "PUT\n/";
    QByteArray contentsha256 = QCryptographicHash::hash(aclStr,QCryptographicHash::Sha256).toHex();
    canonicalRequest+=objectName+"\n";
    canonicalRequest+="acl=\n";//带参数
    canonicalRequest+="host:"+host+"\n";
    canonicalRequest+="x-amz-content-sha256:"+contentsha256+"\n";
    canonicalRequest+="x-amz-date:"+datetime+"\n\n";
    canonicalRequest+="host;x-amz-content-sha256;x-amz-date\n";
    canonicalRequest+=contentsha256;

    QByteArray stringToSign = "AWS4-HMAC-SHA256\n"+datetime+"\n"+date+"/"+region+"/s3/aws4_request\n";
    stringToSign+=QCryptographicHash::hash(canonicalRequest,QCryptographicHash::Sha256).toHex();
    QByteArray kSigning = GetSignatureKey(date,region);
    QByteArray signature = QMessageAuthenticationCode::hash(stringToSign,kSigning, QCryptographicHash::Sha256).toHex();
    QByteArray authorization = "AWS4-HMAC-SHA256 Credential="+secertID.toLatin1()+"/"+date+"/"+region+"/s3/aws4_request,SignedHeaders=host;x-amz-content-sha256;x-amz-date,Signature="+signature;

    QNetworkRequest request(QUrl("http://"+host+"/"+objectName+"?acl"));
    request.setRawHeader("Host",host);
    request.setRawHeader("x-amz-date",datetime);
    request.setRawHeader("Authorization",authorization);
    request.setRawHeader("x-amz-content-sha256",contentsha256);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/xml");
    return request;
}
*/
QByteArray S3Client::GetSignatureKey(QByteArray date,QByteArray region)
{
    QByteArray kDate = QMessageAuthenticationCode::hash(date,"AWS4" + secertKey.toLatin1(), QCryptographicHash::Sha256);
    QByteArray kRegion = QMessageAuthenticationCode::hash(region,kDate, QCryptographicHash::Sha256);
    QByteArray kService = QMessageAuthenticationCode::hash("s3",kRegion, QCryptographicHash::Sha256);
    QByteArray kSigning = QMessageAuthenticationCode::hash("aws4_request",kService, QCryptographicHash::Sha256);
    return kSigning;
}
QString S3Client::GetDateTime()
{
    QDateTime datetime = QDateTime::currentDateTimeUtc();
    QString sdatetime = GetDate()+"T";
    QString time = datetime.time().toString(Qt::ISODate);
    QStringList tmp = time.split(':');
    return sdatetime+tmp[0]+tmp[1]+tmp[2]+"Z";
}
QString S3Client::GetDate()
{
    QDateTime datetime = QDateTime::currentDateTimeUtc();
    QString date = datetime.date().toString(Qt::ISODate);
    QStringList tmp = date.split('-');
    return tmp[0]+tmp[1]+tmp[2];
}
QString S3Client::GetHost(QString region, QString bucketName)
{
    if(region=="us-east-1")
        return bucketName+".s3.amazonaws.com";
    else
        return bucketName+".s3-"+region+".amazonaws.com";
}
QString S3Client::ParseLoaction(QString region)
{
    QString region_s3;
    if(region == "US")
        region_s3 = "us-east-1";
    else if (region == "EU")
        region_s3 = "eu-central-1";
    else if(region =="Asia")
        region_s3 = "ap-southeast-1";
    return region_s3;
}
QString S3Client::ParseStorageClass(QString storageClass)
{
    QString storageClass_s3;
    if(storageClass == "STANDARD")
        storageClass_s3 = "STANDARD";
    else if (storageClass == "STANDARD-IA")
        storageClass_s3 = "STANDARD_IA";
    else if(storageClass == "GRACIER")
        storageClass_s3 = "REDUCED_REDUNDANCY";
    return storageClass_s3;
}
void S3Client::SetBuckets(std::vector<Bucket> &buckets)
{
    this->buckets = buckets;
}
QString S3Client::GetRegion(QString bucketName)
{
    QString region;
    for(int i=0;i<buckets.size();i++)
        if(buckets[i].bucketName == bucketName)
        {
            region = buckets[i].region;
            break;
        }
    return region;
}
QString S3Client::GetStorageClass(QString bucketName)
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
