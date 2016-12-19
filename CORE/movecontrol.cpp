#include "movecontrol.h"

MoveControl::MoveControl(int blockCount)
    :blockCount(blockCount),finishedCount(0)
{
}

void MoveControl::subFinished_slot()
{
    mutex.lock();
    ++finishedCount;
    mutex.unlock();
    emit subFinished_signal(finishedCount);
    if(finishedCount==blockCount)
        emit finished();
}

void MoveControl::Copy(QNetworkReply *reply)
{
    connect(reply,SIGNAL(finished()),this,SLOT(subFinished_slot()));
}
