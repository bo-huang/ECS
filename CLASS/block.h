#ifndef BLOCK_H
#define BLOCK_H
#include<QByteArray>
#include<QString>

class Block
{
public:
    Block();
public:
    QByteArray blockID;//segmentID + 序号
    int cloudID;//存储在哪个云上
    //QByteArray block_data;
};

#endif // BLOCK_H
