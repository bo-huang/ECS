#ifndef DATATRANSFER_H
#define DATATRANSFER_H
#include<QNetworkAccessManager>
#include<QNetworkReply>
#include<QJsonDocument>
#include<QJsonArray>
#include<QJsonObject>
#include<vector>
#include"CLASS/segment.h"
#include"CLASS/snapshot.h"
#include"CLASS/cloudinfo.h"
#include"CloudSDK/cloudclient.h"
#include"CloudSDK/googleclient.h"
#include"CloudSDK/s3client.h"
#include"CloudSDK/azureclient.h"
#include"CloudSDK/aliyunclient.h"
#include"CloudSDK/baiduyunclient.h"
#include"CORE/coder.h"
#include<map>
#include<CLASS/bucket.h>
#include<fstream>
#include<QMutex>
using namespace std;

class DataTransfer : public QObject
{
    Q_OBJECT
public:
    DataTransfer(QList<CloudInfo> &clouds);
    ~DataTransfer();
    bool Upload(QString bucketName,QString fileName);
    bool DownLoad(QString bucketName,QString fileName,QString outputPath);
    bool DeleteFile(QString bucketName,QString fileName);
    bool MoveFile(QString bucketName,QString fileName,QString desBucketName);
    QJsonDocument GetSnapshot();
    bool IsExistBucket(QString bucketName);
    bool CreateBucket(QString bucketName,QString region,QString storageClass);
    vector<Bucket> GetBucket();
    int GetFileSize(QString bucketName,QString fileName);
    int GetDownloadSize(QString bucketName,QString fileName);
    int GetUploadSize(QString path);
    bool IsFileExist(QString bucketName,QString fileName);
    bool Rename(QString bucketName,QString fileName,QString newName);
signals:
    void uploadProgress_signal(int size);
    void downloadProgress_signal(int size);
    void moveProgress_signal(int value);
    void moveProgressSetRange(int minnum,int maxnum);//设置move progressbar的range
    void downloadFinished_signal();
    void beginUpload();
    void deleteProgress_signal(int value);
    void deleteFinished_signal();
    void deleteProgressSetRange(int minnum,int maxnum);
    //用于启动多线程的信号（Qt很变态的方法……）
    void upload_signalForThread(QString bucketName,QString objectName);
    void download_signalForThread(QString bucketName,QString fileName,QString outputPath);
private slots:
    void subUploadFinished_slot(int size);
    void subDownloadFinished_slot(int size);
    void subMoveFinished_slot(int value);
    void deleteFinished_slot();
    void deleteProgress_slot(int value);
public slots:
    void SpiltToSegment(QString bucketname,QString filename);
private:
    //void SpiltToSegment(QString bucketname,QString filename);
    void LoadMetadata();
    CloudClient* CreatCloudClient(CloudInfo &cloud);
    void UpdateMetadataForUpload();
    void UpdataMetadataForDownload(QString bucketName,QString fileName);
    void UploadMetadata();
    void DecodeAndWrite(bool);
    void SortByRate(Block *blocks,int num);
    bool CmpRate(Block &,Block &);
    int GetIdByName(QString cloudName);
    void DeleteForMoveFile(QString bucketName, QString fileName, QString desBucketName);
private:
    //long long _filesize;//after encode
    const int segmentLength = 1024*1024*16;//each block 2MB
    int partition;
    int redundancy;
    Snapshot snapshot;
    vector<Segment>segments;
    vector<Bucket>buckets;
    QNetworkAccessManager *manger;
    QList<CloudInfo> clouds;
    int cloudNum;
    //metadata
    static QJsonDocument snapshot_metadata;
    static QJsonDocument segmentpool_metadata;
    static bool is_first_in;//第一次进入则加载metadata
    //for download
    vector<QByteArray>download_blocks;//保存下载的每一段的blocks
    int totalDownloadBlocks;//保存总共需要下载多少数据块
    int totalDownloadSize;//总共需要下载的字节
    int occupiedspace;//文件实际占用空间
    //for并行下载
    vector<QByteArray>dblocks;//下载下来的bolcks
    ofstream writeFile;
    QMutex mutex;
    bool lastIsFinished;
    //速率排序列表
    QStringList rateList;
};

#endif // DATATRANSFER_H
