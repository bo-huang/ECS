#include "CloudSDK/googleclient.h"
#include<QNetworkRequest>
#include<QNetworkReply>
#include <QEventLoop>
#include<QJsonObject>
#include<QJsonDocument>
#include<QMessageBox>
#include<CLASS/jwt.h>

QString GoogleClient::access_token=NULL;
QString GoogleClient::refresh_token=NULL;
QTime GoogleClient::expires_time;

GoogleClient::GoogleClient(QNetworkAccessManager *manger,QString secertID,QString secertKey)
    :CloudClient(manger)
{
    this->secertID = secertID;
    this->secertKey = secertKey;

}

GoogleClient::GoogleClient(QNetworkAccessManager *manger, const char *jsonPath)
    :CloudClient(manger)
{
    this->jsonPath = jsonPath;
}

void GoogleClient::SetProjectID(QString projectID)
{
    this->projectID = projectID;
}

void GoogleClient::SetRefreshToken(QString refreshToken)
{
    if(refresh_token==NULL)
    {
        refresh_token = refreshToken;
        access_token = GetAccessTokenByRefreshToken();
    }
}

bool GoogleClient::Login()
{
    //const char *path = "C:/Users/bohuang/Desktop/My Project-f47716ca278a.json";
    QByteArray jwt = JWT::CreateJWT(jsonPath);
    QNetworkRequest request(QUrl("https://www.googleapis.com/oauth2/v4/token"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    QByteArray postData = "grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Ajwt-bearer&assertion="
            +jwt;
    QNetworkAccessManager *manger = new QNetworkAccessManager;
    QEventLoop loop;
    QNetworkReply *reply = manger->post(request,postData);
    connect(reply,SIGNAL(finished()),&loop,SLOT(quit()));
    loop.exec();
    reply->deleteLater();
    if(reply->error() == QNetworkReply::NoError)
    {
        QByteArray data = reply->readAll();
        QJsonObject json = QJsonDocument::fromJson(data).object();
        access_token = json["access_token"].toString();
        int expires_in = json["expires_in"].toInt();
        expires_time = QTime::currentTime().addSecs(expires_in-20);
        projectID = JWT::projectID;
        return true;
    }
    return false;
}

bool GoogleClient::CreateBucket(QString bucketName, QString region, QString storageClass)
{
    //检查access_token是否有效
    if(QTime::currentTime()>= expires_time)
        //access_token = GetAccessTokenByRefreshToken();
        Login();
    if(access_token==NULL)
    {
        qDebug() << "access_token is NULL" ;
        return false;
    }
    region = ParseLoaction(region);
    storageClass = ParseStorageClass(storageClass);
    QString url = "https://www.googleapis.com/storage/v1/b?project="+projectID.toLatin1().toPercentEncoding();
    QString requestBody = QString("{\"name\":\"%1\",\"location\":\"%2\",\"storageClass\":\"%3\"}").arg(bucketName).arg(region).arg(storageClass);

    QNetworkRequest request(url);
    request.setRawHeader("Authorization","Bearer "+access_token.toLatin1());
    request.setRawHeader("Host","www.googleapis.com");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QNetworkReply *reply = manger->post(request,requestBody.toLatin1());
    QEventLoop eventloop;
    connect(reply,SIGNAL(finished()),&eventloop,SLOT(quit()));
    eventloop.exec();
    reply->deleteLater();
    reply->close();
    if (reply->error() == QNetworkReply::NoError)
        return true;
    else
        return false;
}


QString GoogleClient::GetRefreshToken()
{
    return refresh_token;
}

QNetworkReply* GoogleClient::GetObject(QString bucketName, QString objectName)
{
    //检查access_token是否有效
    if(QTime::currentTime()>= expires_time)
        //access_token = GetAccessTokenByRefreshToken();
        Login();
    if(access_token==NULL)
    {
        qDebug() << "access_token is NULL" ;
        return NULL;
    }
    QString url = GetmediaLink(bucketName,objectName);
    if(url==NULL)
        return NULL;
    QNetworkRequest request(url);
    request.setRawHeader("Authorization","Bearer "+access_token.toLatin1());
    request.setRawHeader("Host","www.googleapis.com");
    QNetworkReply *reply = manger->get(request);
    return reply;
    /*QEventLoop eventLoop;
    connect(reply,SIGNAL(finished()),&eventLoop,SLOT(quit()));
    eventLoop.exec();
    if (reply->error() == QNetworkReply::NoError)
    {
        return reply->readAll();
    }
    else
    {
         qDebug() << "google getobject error:" << reply->error();
         return NULL;
    }*/
}

QNetworkReply * GoogleClient::CopyObject(QString bucketName, QString objectName, QString desBucketName)
{
    //检查access_token是否有效
    if(QTime::currentTime()>= expires_time)
        //access_token = GetAccessTokenByRefreshToken();
        Login();
    if(access_token==NULL)
    {
        qDebug() << "access_token is NULL" ;
        return NULL;
    }
    QString url = QString("https://www.googleapis.com/storage/v1/b/%1/o/%2/copyTo/b/%3/o/%4")
            .arg(bucketName,objectName,desBucketName,objectName);
    QNetworkRequest request(url);
    request.setRawHeader("Authorization","Bearer "+access_token.toLatin1());
    request.setRawHeader("Host","www.googleapis.com");
    QNetworkReply *reply = manger->post(request,"");
    return reply;
}

QString GoogleClient::GetmediaLink(QString bucketName, QString objectName)
{
    QString url = "https://www.googleapis.com/storage/v1/b/"+bucketName+"/o/"+objectName;
    QNetworkRequest request(url);
    request.setRawHeader("Authorization","Bearer "+access_token.toLatin1());
    request.setRawHeader("Host","www.googleapis.com");
    QNetworkReply *reply = manger->get(request);
    QEventLoop eventLoop;
    connect(reply,SIGNAL(finished()),&eventLoop,SLOT(quit()));
    eventLoop.exec();
    reply->close();
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray jsondata = reply->readAll();
        //解析json
        QJsonDocument json = QJsonDocument::fromJson(jsondata);
        QJsonObject obj = json.object();
        return obj["mediaLink"].toString();
    }
    else
    {
         qDebug() << "get medialink error" << reply->error();
         return NULL;
    }
}
QString GoogleClient::GetAccessTokenByRefreshToken()
{
    QString url = "https://www.googleapis.com/oauth2/v3/token";
    QNetworkRequest request(url);
    request.setRawHeader("Host","www.googleapis.com");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    //QByteArray QUrl toPercentEncoding() urlencode
    QByteArray post_data = "client_secret="+secertKey.toLatin1().toPercentEncoding()
            +"&grant_type=refresh_token&refresh_token="+refresh_token.toLatin1().toPercentEncoding()
            +"&client_id="+secertID.toLatin1().toPercentEncoding();
    QEventLoop eventLoop;
    QNetworkReply *reply = manger->post(request,post_data);

    connect(reply,SIGNAL(finished()),&eventLoop,SLOT(quit()));
    eventLoop.exec();
    if (reply->error() == QNetworkReply::NoError)
    {
         QByteArray jsondata = reply->readAll();
         //解析json
         QJsonDocument json = QJsonDocument::fromJson(jsondata);
         QJsonObject obj = json.object();
         int expires_in = obj["expires_in"].toInt();
         QString token = obj["access_token"].toString();
         //此处减去20s，防止出现误差
         expires_time = QTime::currentTime().addSecs(expires_in-20);
         reply->deleteLater();
         reply->close();
         return token;
    }
    else
    {
         qDebug() << "google get accesstoken error:" << reply->error();
         return NULL;
    }
}

QNetworkReply* GoogleClient::PutObject(QString bucketName, QString objectName, QByteArray data)
{
    //检查access_token是否有效
    if(QTime::currentTime()>= expires_time)
        //access_token = GetAccessTokenByRefreshToken();
        Login();
    if(access_token==NULL)
    {
        qDebug() << "access_token is NULL" ;
        return NULL;
    }
    QString url = "https://www.googleapis.com/upload/storage/v1/b/"+bucketName+"/o?uploadType=media&name="+objectName;
    QNetworkRequest request(url);
    request.setRawHeader("Authorization","Bearer "+access_token.toLatin1());
    request.setRawHeader("Host","www.googleapis.com");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"image/jpeg");
    QNetworkReply *reply = manger->post(request,data);
    return reply;
}

QNetworkReply* GoogleClient::DeleteObject(QString bucketName, QString objectName)
{
    //检查access_token是否有效
    if(QTime::currentTime()>= expires_time)
        //access_token = GetAccessTokenByRefreshToken();
        Login();
    if(access_token==NULL)
    {
        qDebug() << "access_token is NULL" ;
        return NULL;
    }
    QString url = "https://www.googleapis.com/storage/v1/b/"+bucketName+"/o/"+objectName;
    QNetworkRequest request(url);
    request.setRawHeader("Authorization","Bearer "+access_token.toLatin1());
    request.setRawHeader("Host","www.googleapis.com");
    QNetworkReply* reply = manger->deleteResource(request);
    return reply;
}
/*
QByteArray GoogleClient::GetObjectAcl(QString bucketName, QString objectName)
{
    //检查access_token是否有效
    if(QTime::currentTime()>= expires_time)
        access_token = GetAccessTokenByRefreshToken();
    if(access_token==NULL)
    {
        qDebug() << "access_token is NULL" ;
        return NULL;
    }
    QString url = "https://www.googleapis.com/storage/v1/b/"+bucketName+"/o/"+objectName+"/acl";
    QNetworkRequest request(url);
    request.setRawHeader("Authorization","Bearer "+access_token.toLatin1());
    request.setRawHeader("Host","www.googleapis.com");
    QNetworkReply *reply = manger->get(request);
    QEventLoop eventloop;
    connect(reply,SIGNAL(finished()),&eventloop,SLOT(quit()));
    eventloop.exec();

    if (reply->error() == QNetworkReply::NoError)
        return reply->readAll();
    else
    {
        qDebug()<<"google getobjectAcl error:"+reply->error();
        return NULL;
    }
}
void GoogleClient::SetObjectAcl(QString bucketName, QString objectName, QByteArray aclStr)
{
    //检查access_token是否有效
    if(QTime::currentTime()>= expires_time)
        access_token = GetAccessTokenByRefreshToken();
    if(access_token==NULL)
    {
        qDebug() << "access_token is NULL" ;
        return;
    }
    QString url = "https://www.googleapis.com/storage/v1/b/"+bucketName+"/o/"+objectName+"/acl";
    QNetworkRequest request(url);
    request.setRawHeader("Authorization","Bearer "+access_token.toLatin1());
    request.setRawHeader("Host","www.googleapis.com");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    manger->post(request,aclStr);
}
*/
QString GoogleClient::ParseLoaction(QString region)
{
    QString region_google;
    if(region == "US")
        region_google = "US";
    else if(region == "EU")
        region_google = "EU";
    else if(region == "Asia")
        region_google = "asia-east1";
    return region_google;
}
QString GoogleClient::ParseStorageClass(QString storageClass)
{
    qDebug()<<storageClass;
    QString storageClass_google;
    if(storageClass == "STANDARD")
        storageClass_google = "STANDARD";
    else if(storageClass == "STANDARD-IA")
        storageClass_google = "DURABLE_REDUCED_AVAILABILITY";
    else if(storageClass == "GRACIER")
        storageClass_google = "NEARLINE";
    return storageClass_google;
}
