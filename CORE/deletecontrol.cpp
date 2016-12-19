#include "deletecontrol.h"


DeleteControl::DeleteControl(int blockCount)
    :blockCount(blockCount),finishedCount(0)
{

}

void DeleteControl::subFinished_slot()
{
    mutex.lock();
    ++finishedCount;
    mutex.unlock();
    emit subFinished_signal(finishedCount);
    if(finishedCount==blockCount)
        emit finished();
}

void DeleteControl::Delete(QNetworkReply *reply)
{
    if(reply==NULL)
        subFinished_slot();
    else
        connect(reply,SIGNAL(finished()),this,SLOT(subFinished_slot()));
}
