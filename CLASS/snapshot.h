#ifndef SNAPSHOT_H
#define SNAPSHOT_H
#include<QByteArray>
#include<QString>

class Snapshot
{
public:
    Snapshot();
public:
    QString fileName;//fileName唯一
    long long fileSize;//文件大小
    long long occupiedSpace;//占用空间
    QString createTime;//创建时间
    QString modified;//修改时间
    QString bucket;
    QString storageClass;
    int frequency;//访问次数
};

#endif // SNAPSHOT_H
