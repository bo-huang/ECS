#ifndef CLOUDINFO_H
#define CLOUDINFO_H
#include <QString>


class CloudInfo
{
public:
    CloudInfo();
public:
    QString cloudName;
    QString account;
    QString addTime;
    QString certificate;//包含secertID和Secertkey等信息，不同项中间由‘|’隔开
    QString defaultBucket;//存放metadata的bucket
};

#endif // CLOUDINFO_H
