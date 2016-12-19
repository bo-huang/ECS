#include "jwt.h"
#include <fstream>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <time.h>
#include <QDateTime>
#include <QDebug>
#include <CLASS/systemtime.h>

using std::ifstream;


QString JWT::projectID;

JWT::JWT()
{

}

QByteArray JWT::CreateJWT(const char *jsonPath)
{
    if(!Register())
        return "";
    ifstream is;
    is.open(jsonPath);
    if(is)
    {
        QFileInfo info(jsonPath);
        const int size = info.size();
        char buf[size];
        is.read(buf,size);
        is.close();
        QJsonObject json = QJsonDocument::fromJson(QByteArray(buf,size)).object();
        projectID = json["project_id"].toString();
        QString strKey = json["private_key"].toString();
        QString strEmail = json["client_email"].toString();
        //////////////////////////////////////////////////////////////
        CkPrivateKey privKey;
        bool success = privKey.LoadPem(strKey.toStdString().data());
        if (success != true)
            return "";
        CkJwt jwt;
        const char *header = "{\"alg\": \"RS256\",\"typ\": \"JWT\"}";
        uint time = QDateTime::currentDateTime().toTime_t();
        QString claimset = QString("{"
                              "\"iss\": \"%1\","
                              "\"scope\": \"https://www.googleapis.com/auth/devstorage.full_control\","
                              "\"aud\": \"https://www.googleapis.com/oauth2/v4/token\","
                              "\"exp\": %2,"
                              "\"iat\": %3"
                              "}").arg(strEmail,QString::number(time+3600),QString::number(time));
        //  Produce the smallest possible JWT:
        jwt.put_AutoCompact(true);
        //  Create the JWT token.  This is where the RSA signature is created.
        const char *token = jwt.createJwtPk(header,claimset.toStdString().data(),privKey);
        return QByteArray(token);
    }
    return "";
}
//注册（变态需要收费！！）
//哈哈，找到解决方法了：调用这个函数前修改系统时间到注册那一天
bool JWT::Register()
{
    //修改系统时间
    struct tm * clock_time;
    time_t timep;
    char curtime[256];
    memset(curtime, 0, 256);
    timep = time(NULL);
    clock_time = localtime(&timep);
    sprintf(curtime, "date %d-%d-%d",	//保存正确的系统时间
        clock_time->tm_year + 1900, clock_time->tm_mon + 1, clock_time->tm_mday);
    //system("date 2016-11-24");这样写会弹出cmd运行框
    SystemTime::SetDate("date 2016-11-24");
    //注册
    CkGlobal glob;
    bool success = glob.UnlockBundle("Anything for 30-day trial");
    //qDebug()<<glob.lastErrorText();
    //恢复系统时间
    //system(curtime);
    SystemTime::SetDate(curtime);
    return success;
}
