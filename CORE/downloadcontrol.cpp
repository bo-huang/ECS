#include "downloadcontrol.h"
#include<exception>

DownloadControl::DownloadControl(int partition,int totalTask,int blockSize)
    :partition(partition),totalTask(totalTask),blockSize(blockSize)
{
    count = 0;
    finishedTask = 0;
    tasks = new Download[totalTask];
}

DownloadControl::~DownloadControl()
{
    delete []tasks;
}

void DownloadControl::Run(QNetworkReply *reply)
{
    connect(tasks+count,SIGNAL(subFinished(QByteArray&)),this,SLOT(subFinished(QByteArray&)));
    tasks[count].Run(reply);
    ++count;//下载任务数
}

void DownloadControl::subFinished(QByteArray &block)
{
    //由于是多线程并发下载，所以此处需要上锁
    //qDebug()<<"block";
    mutex.lock();
    ++finishedTask;
    mutex.unlock();
    if(block!=NULL&&blocks.size()<partition)
    {
        blocks.push_back(block);
        emit subFinished(blockSize);
    }
    if(finishedTask==count)//所有任务都完成
        emit downloadFinished();//但不一定下载完所需要的数据块
    //之前的做法:一个段所有blocks同时下载，当达到指定的partition块时结束其他还在下载中的任务
    //但这样其实还是很浪费下载带宽，因为需要下载额外的数据块，即使最后终止他们了
    /*if(!isFinished&&blocks.size()==partition)
    {
        isFinished = true;       
        emit downloadFinished();

        //AbortAllTask();//当下载到足够的数据块，就结束还在下载中的任务，释放网络资源
    }

    if(!isFinished&&finishedTask==totalTask)
        emit downloadFinished();*/
}

int DownloadControl::DownloadBlocksCount() const
{
    return blocks.size();
}

void DownloadControl::AbortAllTask()
{
    for(int i=0;i<totalTask;++i)
        tasks[i].Abort();
}
