#ifndef SEGMENT_H
#define SEGMENT_H
#include<QDateTime>
#include<QByteArray>
#include<QString>
#include<CLASS/block.h>

class Segment
{
public:
    Segment();
public:
    QByteArray segmentID;//MD5
    int blockNum;
    int segmentLength;
    int blockLength;
    int refCnt;
    Block *blocks;
};

#endif // SEGMENT_H
