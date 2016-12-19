#ifndef OPERATION_H
#define OPERATION_H
#include <QString>


class Operation
{
public:
    Operation();
public:
    QString operation;//upload download delete create
    QString path;//下载位置 or 上传文件位置
    QString bucket;
    QString fileName;
    QString desBucket;//目标bucket
    int fileSize;//upload or download 的数据量（不是实际文件大小）
};

#endif // OPERATION_H
