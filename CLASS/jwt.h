#ifndef JWT_H
#define JWT_H
#include <CkPrivateKey.h>
#include <CkJwt.h>
#include <CkGlobal.h>
#include <QString>

class JWT
{
public:
    JWT();
    static QByteArray CreateJWT(const char *jsonPath);
    static QString projectID;
private:
    static bool Register();
};

#endif // JWT_H
