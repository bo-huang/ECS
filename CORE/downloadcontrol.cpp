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
    ++count;
}

void DownloadControl::subFinished(QByteArray &block)
{
    //由于是多线程并发下载，所以此处需要上锁
    //qDebug()<<"block";
    mutex.lock();
    ++finishedTask;
    if(block!=NULL&&blocks.size()<partition)
    {
        blocks.push_back(block);
        emit subFinished(blockSize);
    }
    if(!isFinished&&blocks.size()==partition)
    {
        isFinished = true;       
        emit downloadFinished();
        /*
        AbortAllTask();//当下载到足够的数据块，就结束还在下载中的任务，释放网络资源
        */
    }
    mutex.unlock();
    if(!isFinished&&finishedTask==totalTask)
        emit downloadFinished();
}

void DownloadControl::AbortAllTask()
{
    for(int i=0;i<totalTask;++i)
        tasks[i].Abort();
}
