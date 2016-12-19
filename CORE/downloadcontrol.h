#ifndef DOWNLOADCONTROL_H
#define DOWNLOADCONTROL_H
#include<QObject>
#include<QNetworkReply>
#include<QByteArray>
#include<QMutex>
#include<vector>

class Download;

class DownloadControl:public QObject
{
    Q_OBJECT
public:
    DownloadControl(int partition,int tatalCount,int blockSize);
    ~DownloadControl();
    void Run(QNetworkReply *reply);
    std::vector<QByteArray>& GetBlocks(){return blocks;}
private:
    void AbortAllTask();
signals:
    downloadFinished();
    subFinished(int size);
private slots:
    void subFinished(QByteArray&); 
private:
    QMutex mutex;
    bool isFinished = false;
    int partition;
    int totalTask;
    int finishedTask;
    int count;
    int blockSize;
    std::vector<QByteArray>blocks;
    Download *tasks;//每个task负责下载一个block
};

class Download:public QObject
{
    Q_OBJECT
public:
    Download()=default;
    void Run(QNetworkReply *reply)
    {
        this->reply = reply;
        connect(reply,SIGNAL(finished()),this,SLOT(finished()));
    }
    void Abort()
    {
        if(!reply&&!reply->isFinished())
            reply->abort();
    }

private:
    QNetworkReply *reply;
    QByteArray block = NULL;
private slots:
    void finished()
    {
        if(reply!=NULL&&reply->error()==QNetworkReply::NoError)
        {
            block = reply->readAll();
            emit subFinished(block);
        }
        else
            qDebug()<<"download error";
        reply->close();
        reply->deleteLater();
    }

signals:
    void subFinished(QByteArray&);
};

#endif // DOWNLOADCONTROL_H
