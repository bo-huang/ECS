#include "datatransfer.h"
#include "CORE/coder.h"
#include<fstream>
#include<QMessageBox>
#include<math.h>
#include<QtConcurrent/QtConcurrent>
#include<CORE/uploadcontrol.h>
#include<CORE/downloadcontrol.h>
#include<CORE/movecontrol.h>
#include<CLASS/uploadassist.h>
#include<CORE/deletecontrol.h>
#include<QCoreApplication>
using namespace std;


QJsonDocument DataTransfer::snapshot_metadata;
QJsonDocument DataTransfer::segmentpool_metadata;
bool DataTransfer::is_first_in = true;

DataTransfer::DataTransfer(QList<CloudInfo> &clouds)
{
    cloudNum = clouds.size();
    this->clouds = clouds;
    /*partition = cloudNum;
    int fair_share = partition/(cloudNum-1);//每个云至少存储的bolck块
    if(partition%(cloudNum-1)!=0)
        fair_share++;
    redundancy = fair_share * cloudNum - partition;
    */
    //partition和redundancy这样设置是为了方便添加和移除云时的数据迁移
    partition = 8;
    redundancy = 4;
    manger = new QNetworkAccessManager;
    LoadMetadata();
    //生成buckets
    QJsonArray jarr_buckets = snapshot_metadata.array();
    for(int i=0;i<jarr_buckets.size();i++)
    {
        QJsonObject jo_bucket = jarr_buckets.at(i).toObject();
        Bucket bucket;
        bucket.bucketName =jo_bucket["bucketname"].toString();
        bucket.region =jo_bucket["region"].toString();
        bucket.storageClass = jo_bucket["storageclass"].toString();
        buckets.push_back(bucket);
    }
}

DataTransfer::~DataTransfer()
{

}

QJsonDocument DataTransfer::GetSnapshot()
{
    return snapshot_metadata;
}
/*
void DataTransfer::SpiltToSegment(QString filename)
{
    ifstream is;
    is.open(filename.toStdString(),ios::binary);
    char *buffer = new char[segmentLength];
    Coder *coder = new Coder(partition,redundancy);
    snapshot.fileSize = 0;

    segments.clear();
    while(!is.eof())
    {
        is.read(buffer,segmentLength);
        snapshot.fileSize+=is.gcount();
        Segment segment;
        segment.segmentLength = is.gcount();
        segment.segmentID = QCryptographicHash::hash(QByteArray(buffer,is.gcount()),QCryptographicHash::Md5).toHex();
        //encode
        vector<QByteArray>blocks_data = coder->Encode(QByteArray(buffer,is.gcount()));
        segment.blockNum = blocks_data.size();
        segment.blockLength = coder->GetBlockLength();
        int cnt=0;
        Block *blocks = new Block[segment.blockNum];
        //分配block(目前先按照这种简单的均分)
        for(int i=0;i<segment.blockNum;i++)
        {
            blocks[i].blockID = segment.segmentID + '_' + char('0'+i);
            blocks[i].block_data = blocks_data[i];
            blocks[i].D = cnt;
            if(++cnt==cloudNum)
                cnt=0;
        }
        segment.blocks = blocks;
        segments.push_back(segment);
    }
    is.close();

    snapshot.fileName = filename.split('/').last();
    snapshot.modified = QDateTime::currentDateTime();

}
*/

void DataTransfer::SpiltToSegment(QString bucketname,QString filepath)
{
    ifstream is;
    char *buffer = new char[segmentLength];//保存段文件
    Coder *coder = new Coder(partition,redundancy);
    is.open(filepath.toStdString(),ios::binary);
    QFileInfo info(filepath);
    snapshot.fileSize = info.size();
    snapshot.fileName = filepath.split('/').last();
    snapshot.modified = QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate);
    //这一段需要移到mainwindow里面删除！又是不能跨线程访问的问题
    /*bool isFileExist = IsFileExist(bucketname,snapshot.fileName);
    //先删除文件
    if(isFileExist)
        DeleteFile(bucketname,snapshot.fileName);
*/
    segments.clear();
    totalDownloadSize = 0;//记录下载该文件时至少需要下载多少数据
    occupiedspace = 0;//文件实际占用空间
    //***上传另起一个线程（解决消息循环阻塞的问题）
    UploadAssist *uploadassist = new UploadAssist(clouds,bucketname);
    uploadassist->SetBuckets(buckets);
    QThread *thread = new QThread;
    uploadassist->moveToThread(thread);
    thread->start();
    connect(uploadassist,SIGNAL(subFinished(int)),this,SLOT(subUploadFinished_slot(int)));
    //test
    //char *data = new char[snapshot.fileSize];
    //is.read(data,snapshot.fileSize);
    //is.close();
    emit beginUpload();
    //int pos = 0;
    //end
    while(!is.eof())
    //while(pos<snapshot.fileSize)
    {
        is.read(buffer,segmentLength);
        Segment segment;
        segment.segmentLength = is.gcount();
        /*buffer = data+pos;
        if(pos+segmentLength<=snapshot.fileSize)
        {
            pos+=segmentLength;
            segment.segmentLength = segmentLength;
        }
        else
        {
            segment.segmentLength = snapshot.fileSize-pos;
            pos = snapshot.fileSize;
        }*/
        //MD5
        segment.segmentID = QCryptographicHash::hash(QByteArray(buffer,segment.segmentLength),QCryptographicHash::Md5).toHex();
        //检验该段是否存在，存在则不上传
        QJsonArray jarr_segments = segmentpool_metadata.array();
        bool isSegmentExist = false;
        int segmentUploadSize=0;//如果segment存在，计算segment的大小
        segment.refCnt = 1;
        //检查已经上传的文件中有没有相同的segment
        bool first_time = true;//爱
        for(int s = 0;s<jarr_segments.size();++s)
        {
            QJsonObject jo_segment = jarr_segments.at(s).toObject();
            if(jo_segment["segmentid"].toString().toLatin1() ==segment.segmentID
                    &&bucketname==jo_segment["bucketname"].toString())
            {
                first_time=false;
                isSegmentExist = true;
                segment.refCnt += jo_segment["refcnt"].toInt();
                totalDownloadSize+=partition*jo_segment["blocklength"].toInt();
                segmentUploadSize = (partition+redundancy)*jo_segment["blocklength"].toInt();
                break;
            }
        }
        //检查该文件中有没有相同的segment
        for(int i=0;i<segments.size();++i)
        {
            if(segment.segmentID==segments[i].segmentID)
            {
                ++segment.refCnt;
                if(first_time)
                {
                    totalDownloadSize+=coder->GetBlockLength()*partition;
                    segmentUploadSize+=coder->GetBlockLength()*(partition+redundancy);
                    first_time=false;
                }
            }
        }
        //如果都没有相同的，则上传
        if(isSegmentExist==false&&segment.refCnt==1)
        {
            //encode
            vector<QByteArray>blocks_data = coder->Encode(QByteArray(buffer,segment.segmentLength));

            segment.blockNum = blocks_data.size();
            segment.blockLength = coder->GetBlockLength();
            totalDownloadSize+=segment.blockLength*partition;
            occupiedspace+=segment.blockLength*(partition+redundancy);
            Block *blocks = new Block[segment.blockNum];
            //分配block
            int k = 0;
            for(int i=0;i<segment.blockNum;++i)
            {
                blocks[i].blockID=segment.segmentID+'_'+QString::number(i,10).toLatin1();
                blocks[i].cloudID = k;
                k=(k+1)%cloudNum;
            }
            //上传
            for(int i=0;i<segment.blockNum;++i)
            {
                //很烦……卡在这里2016/10/28 17:46……
                //问题描述：不能立刻上传
                //QNetworkReply *reply = client[blocks[i].cloudID]->PutObject(bucketname,blocks[i].blockID,blocks_data[i]);
                //UploadControl *uploadControl = new UploadControl(reply,blocks[i].cloudID,segment.blockLength);
                //connect(uploadControl,SIGNAL(subFinished(int)),this,SLOT(subUploadFinished_slot(int)));
                //原因：不能进入消息循环，导致上传事件不能执行
                //解决方法（2016//11/7 16:34）：另外用一个线程上传
                uploadassist->Upload(blocks[i].cloudID,blocks[i].blockID,blocks_data[i]);
            }
            segment.blocks = blocks;
        }
        else
        {
            uploadassist->Upload(segmentUploadSize);
        }
        segments.push_back(segment);
        //强制进入mainwindow的消息循环（否则不能刷新进度条）
        QCoreApplication::processEvents();
    }
    delete []buffer;
    delete coder;
    is.close();
    //更新metdata
    UpdateMetadataForUpload();
    //此处非UI线程，而manger是在UI线程中初始化的，不允许跨线程访问manger，所以必须重新new一个
    manger = new QNetworkAccessManager;
    UploadMetadata();  
}

CloudClient* DataTransfer::CreatCloudClient(CloudInfo &cloud)
{

    CloudClient *client = NULL;
    QStringList keys = cloud.certificate.split('|');
    if(keys.size()==0)
        return NULL;
    if(cloud.cloudName == "google")
    {
        client = new GoogleClient(manger,keys[0].toStdString().data());
    }
    else
    {
        if(keys.size()<2)
            return NULL;
        if(cloud.cloudName == "s3")
        {
            S3Client *s3client = new S3Client(manger,keys[0],keys[1]);
            s3client->SetBuckets(buckets);
            client = s3client;
        }
        else if(cloud.cloudName == "azure")
        {
            client = new AzureClient(manger,keys[0],keys[1]);
        }
        else if(cloud.cloudName == "aliyun")
        {
            client = new AliyunClient(manger,keys[0],keys[1]);
        }
        else if(cloud.cloudName == "baiduyun")
        {
            BaiduyunClient *baiduyun = new BaiduyunClient(manger,keys[0],keys[1]);
            baiduyun->SetBuckets(buckets);
            client = baiduyun;
        }
    }
    return client;
}

bool DataTransfer::CreateBucket(QString bucketName, QString region, QString storageClass)
{
    if(IsExistBucket(bucketName))
        return false;
    CloudClient *client;
    for(int i=0;i<cloudNum;i++)
    {
        client = CreatCloudClient(clouds[i]);
        if(!client->CreateBucket(bucketName,region,storageClass))
        {
            //TODO：删除在部分云上new的bucket
            return false;
        }
    }
    QJsonArray bucktes = snapshot_metadata.array();
    QJsonObject bucket;
    bucket.insert("bucketname",bucketName);
    bucket.insert("region",region);
    bucket.insert("storageclass",storageClass);
    QJsonArray files;
    bucket.insert("files",files);
    bucktes.append(bucket);
    snapshot_metadata.setArray(bucktes);
    UploadMetadata();
    return true;
}

void DataTransfer::DeleteBucket(QString bucketName)
{
    CloudClient *client;
    for(int i=0;i<cloudNum;i++)
    {
        client = CreatCloudClient(clouds[i]);
        client->DeleteBucket(bucketName);
    }
}

int DataTransfer::GetFileSize(QString bucketName,QString fileName)
{
    QJsonArray jarr_buckets = snapshot_metadata.array();
    for(int i=0;i<jarr_buckets.size();++i)
    {
        QJsonObject jo_bucket = jarr_buckets.at(i).toObject();
        if(jo_bucket["bucketname"].toString()==bucketName)
        {
            QJsonArray jarr_files = jo_bucket["files"].toArray();
            for(int j=0;j<jarr_files.size();++j)
            {
                QJsonObject jo_file = jarr_files.at(j).toObject();
                if(jo_file["filename"].toString()==fileName)
                    return jo_file["filesize"].toInt();
            }
        }
    }
    return 0;
}

int DataTransfer::GetDownloadSize(QString bucketName,QString fileName)
{

    QJsonArray jarr_buckets = snapshot_metadata.array();
    for(int i=0;i<jarr_buckets.size();++i)
    {
        QJsonObject jo_bucket = jarr_buckets.at(i).toObject();
        if(jo_bucket["bucketname"].toString()==bucketName)
        {
            QJsonArray jarr_files = jo_bucket["files"].toArray();
            for(int j=0;j<jarr_files.size();++j)
            {
                QJsonObject jo_file = jarr_files.at(j).toObject();
                if(jo_file["filename"].toString()==fileName)
                {
                    return jo_file["filedownloadsize"].toInt();
                }
            }
        }
    }
    return 0;
}

int DataTransfer::GetUploadSize(QString path)
{
    QFileInfo info(path);
    int size = info.size();
    //block的头占4个字节
    int uploadsize = (4+segmentLength/partition+segmentLength%partition)*(partition+redundancy)
            *(size/segmentLength);
    if(size%segmentLength)
    {
        int rlength = size%segmentLength;
        uploadsize += (4+rlength/partition+rlength%partition)*(partition+redundancy);
    }
    return uploadsize;
}

bool DataTransfer::IsFileExist(QString bucketName,QString fileName)
{
    bool exist = false;
    QJsonArray jarr_buckets = snapshot_metadata.array();
    for(int i=0;i<jarr_buckets.size();i++)
    {
        QJsonObject jo_bucket = jarr_buckets.at(i).toObject();
        if(jo_bucket["bucketname"].toString()==bucketName)
        {

            QJsonArray jarr_files = jo_bucket["files"].toArray();

            for(int j=0;j<jarr_files.size();j++)
            {

                QJsonObject jo_file = jarr_files.at(j).toObject();
                if(jo_file["filename"].toString()==fileName.split('/').last())
                {
                    exist = true;
                    break;
                }
            }
        }
        if(exist)
            break;
    }
    return exist;
}

void DataTransfer::subUploadFinished_slot(int size)
{
    emit uploadProgress_signal(size);
}

bool DataTransfer::Upload(QString bucketName, QString fileName)
{
    //检查文件是否存在
    //bool exist = IsFileExist(bucketName,fileName);
    //切分文件
    snapshot.bucket = bucketName;
    emit upload_signalForThread(bucketName,fileName);
    //不能这样写，因为SpiltToSegment中的QNetworkAccessManager属于另外一个线程（具体也解释不清楚）
    //QtConcurrent::run(this,&DataTransfer::SpiltToSegment,fileName);
    //改为信号与槽的方式
    /*connect(this,SIGNAL(spiltToSegment_signal(QString)),this,SLOT(SpiltToSegment(QString)));
    emit spiltToSegment_signal(fileName);
    */
    /*udClient **client = new CloudClient*[cloudNum];
    for(int i=0;i<cloudNum;i++)
    {
        client[i]=CreatCloudClient(cloud[i]);
    }
    */
    /*
    //上传
    CloudClient *client[cloudNum];
    for(int i=0;i<cloudNum;i++)
    {
        client[i]=CreatCloudClient(cloud[i]);
    }
    //计算每一个云上需要上传的blocks数
    int blocksTotal = 0;
    if(segments.size()!=0)
        blocksTotal = segments.size()*segments[0].blockNum/cloudNum;
    for(int i=0;i<segments.size();i++)
    {
        Segment segment = segments[i];
        for(int j=0;j<segment.blockNum;j++)
        {
            Block block = segment.blocks[j];
            QNetworkReply *reply = client[block.cloudID]->PutObject(bucketName,block.blockID,block.block_data);
            // 这样写上传完成后不知道是上传到的哪一个云
            //connect(reply,SIGNAL(finished()),this,SLOT(subUploadFinished_slot()));

            //用一个uploadcontrol类保存额外信息
            UploadControl *uploadControl = new UploadControl(reply,block.cloudID,blocksTotal);
            connect(uploadControl,SIGNAL(subFinished(int,int)),this,SLOT(subUploadFinished_slot(int,int)));
        }
    }
    */
    //更新metdata
    /*
    UpdateMetadataForUpload(exist);
    UploadMetadata();
    */
    //todo:free space
    //cloudclient and segments
    return true;
}

void DataTransfer::subDownloadFinished_slot(int size)
{
    emit downloadProgress_signal(size);
}

bool DataTransfer::DownLoad(QString bucketName, QString fileName, QString outputPath)
{
    CloudClient *client[cloudNum];
    for(int i=0;i<cloudNum;++i)
        client[i] = CreatCloudClient(clouds[i]);
    //get partition and segmentsID
    QJsonArray jarr_snapshot = snapshot_metadata.array();
    QJsonArray jarr_segmentsID;
    for(int i=0;i<jarr_snapshot.size();i++)
    {
        QJsonObject jo_bucket = jarr_snapshot.at(i).toObject();
        if(jo_bucket["bucketname"] != bucketName)
            continue;
        QJsonArray jarr_files = jo_bucket["files"].toArray();
        for(int j=0;j<jarr_files.size();++j)
        {
            QJsonObject jo_file = jarr_files.at(j).toObject();
            if(jo_file["filename"]==fileName)
            {
                partition = jo_file["partition"].toInt();
                jarr_segmentsID = jo_file["segments"].toArray();
                //更改frequency
                int frequency = jo_file["frequency"].toInt();
                jo_file.remove("frequency");
                jo_file.insert("frequency",frequency+1);
                jarr_files.replace(j,jo_file);
                jo_bucket.remove("files");
                jo_bucket.insert("files",jarr_files);
                jarr_snapshot.replace(i,jo_bucket);
                snapshot_metadata.setArray(jarr_snapshot);
                break;
            }
        }
        break;
    }

    QJsonArray jarr_segments = segmentpool_metadata.array();
    //writeFile ofstream
    writeFile.open(outputPath.toStdString(),ios::binary);
    //问题：下载的文件partition可能和当前partition不一样，比如上传这个文件之后添加了一个云

    lastIsFinished = false;//最后一个数据段是否已经写入文件
    int segmentCnt = 0;
    Block *blocks = NULL;
    for(int i=0;i<jarr_segmentsID.size();++i)
    {
        QJsonObject segment;
        for(int j=0;j<jarr_segments.size();++j)
        {
            segment = jarr_segments.at(j).toObject();
            if(jarr_segmentsID.at(i).toString()!=segment["segmentid"].toString())
                continue;
            ++segmentCnt;
            break;
        }

        QJsonArray jarr_blocks = segment["blocks"].toArray();
        int blockSize = segment["blocklength"].toInt();
        download_blocks.clear();

        QEventLoop loop;//控制同步
        DownloadControl downloadControl(partition,jarr_blocks.size(),blockSize);//控制下载segment的所有blocks
        connect(&downloadControl,SIGNAL(downloadFinished()),&loop,SLOT(quit()));
        connect(&downloadControl,SIGNAL(subFinished(int)),this,SLOT(subDownloadFinished_slot(int)));
        //对blocks按下载速率排序
        if(blocks==NULL)
            blocks = new Block[jarr_blocks.size()];
        for(int k=0;k<jarr_blocks.size();++k)
        {
            QJsonObject jo_block = jarr_blocks.at(k).toObject();
            blocks[k].blockID = jo_block["blockid"].toString().toLatin1();
            blocks[k].cloudID = GetIdByName(jo_block["cloudname"].toString());
        }
        SortByRate(blocks,jarr_blocks.size());
        int cnt = 0;//已下载的数据块
        for(int k=0;k<jarr_blocks.size();k++)
        {

            //if(block.cloudID==0) test single point of failure
            //    continue;

            //同步下载
            /*QNetworkReply *reply = client[block.cloudID]->GetObject(bucketName,block.blockID);
            QEventLoop loop;
            connect(reply,SIGNAL(finished()),&loop,SLOT(quit()));
            loop.exec();
            if(reply->error()==QNetworkReply::NoError)
                download_blocks.push_back(reply->readAll());
            if(download_blocks.size()==partition)
                break;*/
            if(blocks[k].cloudID!=-1)
            {
                ++cnt;
                QNetworkReply *reply = client[blocks[k].cloudID]->GetObject(bucketName,blocks[k].blockID);
                downloadControl.Run(reply);
            }
            //先这样处理网络资源浪费（但如果有数据块不能访问则有问题……）
            //if(cnt>=partition)
            //    break;

            //下载完partition快后就结束（改进后的,解决上面那个问题）
            if(cnt==partition)
            {
                //等待下载完成
                loop.exec();
                cnt = downloadControl.DownloadBlocksCount();
                if(cnt==partition)//如果所有数据快下载完成
                    break;
            }
        }

        //这里有点阻塞，因为解码比较耗时
        /*
        QByteArray segment_data = coder.Decode(downloadControl.GetBlocks());
        if(segment_data == NULL)
        {
            qDebug()<<"没能获取到足够的块文件！";
            //break;
        }
        os.write(segment_data,segment_data.count());
        */
        //更改后的
        dblocks = downloadControl.GetBlocks();

        //QtConcurrent::run(DecodeAndWrite); 不能这样写
        QtConcurrent::run(this,&DataTransfer::DecodeAndWrite,segmentCnt==jarr_segmentsID.size());
        if(segmentCnt==jarr_segmentsID.size())
            break;
    }
    //不能这样直接close，因为最后一个数据段还在写文件,因此要等最后一个写文件任务结束
    //writeFile.close();
    //更改后的
    /*while(!lastIsFinished)//阻塞线程
        ;
    writeFile.close();

    emit downloadFinished_signal();
    */
    //最后还是把这些操作放到了DecodeAndWrite函数中
    //修改访问次数
    //UpdataMetadataForDownload(bucketName,fileName);
    UploadMetadata();
    delete []blocks;
    return true;
}
//b1>b2 返回true
bool DataTransfer::CmpRate(Block &b1,Block &b2)
{
    //暂时按照google aliyun s3 baiduyun azure的顺序，实际应该根据每个云的upload ratel
    rateList<<"google"<<"aliyun"<<"s3"<<"baiduyun"<<"azure";
    QString cloudName1 = rateList.last();
    QString cloudName2 = rateList.last();
    //要判断某个云不存在的情形（不存在时cloudID = -1）
    if(b1.cloudID!=-1)
        cloudName1 = clouds[b1.cloudID].cloudName;
    if(b2.cloudID!=-1)
        cloudName2 = clouds[b2.cloudID].cloudName;

    int i;
    for(i=0;i<rateList.size();++i)
        if(cloudName1==rateList[i])
            break;
    for(int j=0;j<rateList.size();++j)
        if(cloudName2==rateList[j])
        {
            if(j>=i)
                return true;
            else
                return false;
        }
    return false;
}

void DataTransfer::SortByRate(Block *blocks,int num)
{

    Block tmp;
    for(int i=1;i<num;++i)
        for(int j=0;j<num-i;++j)
            if(!CmpRate(blocks[j],blocks[j+1]))
            {
                tmp=blocks[j];
                blocks[j]=blocks[j+1];
                blocks[j+1]=tmp;
            }
}

int DataTransfer::GetIdByName(QString cloudName)
{
    for(int i=clouds.size()-1;i>=0;--i)
        if(cloudName==clouds[i].cloudName)
            return i;
    //没有找到
    return -1;
}

void DataTransfer::DecodeAndWrite(bool isLast)
{
    Coder coder;
    //有一个问题：dblocks是全局的，所以有可能下载速度比解码速度快，即新的下载数据覆盖了dblocks
    //所以Coder::Decoder改为值传递，之前写的引用传递
    QByteArray segment_data = coder.Decode(dblocks);
    if(segment_data == NULL)
    {
        qDebug()<<"没能获取到足够的块文件！";
        return;
    }
    //上互斥锁
    mutex.lock();
    writeFile.write(segment_data,segment_data.count());
    mutex.unlock();
    if(isLast)//如果当前是最后一个数据段，则最后一个数据段已经写入到文件中了
    {
        lastIsFinished = true;
        writeFile.close();
        emit downloadFinished_signal();
    }
}

bool DataTransfer::MoveFile(QString bucketName, QString fileName, QString desBucketName)
{
    CloudClient *client[cloudNum];
    for(int i=0;i<cloudNum;i++)
        client[i] = CreatCloudClient(clouds[i]);
    QJsonArray jarr_segments = segmentpool_metadata.array();
    //找到文件的segments
    QJsonArray jarr_segmentsID;
    QJsonArray jarr_snapshot = snapshot_metadata.array();
    for(int i=0;i<jarr_snapshot.size();i++)
    {
        QJsonObject jo_bucket = jarr_snapshot.at(i).toObject();
        if(jo_bucket["bucketname"] != bucketName)
            continue;
        QJsonArray jarr_files = jo_bucket["files"].toArray();
        for(int j=0;j<jarr_files.size();j++)
        {
            QJsonObject jo_file = jarr_files.at(j).toObject();
            if(jo_file["filename"] != fileName)
                continue;
            jarr_segmentsID = jo_file["segments"].toArray();
            //更改snapshot
            jo_file.remove("modified");
            jo_file.insert("modified",QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate));
            jarr_files.removeAt(j);
            jo_bucket.remove("files");
            jo_bucket.insert("files",jarr_files);
            jarr_snapshot.replace(i,jo_bucket);
            //----添加到desbucket中
            for(int k=0;k<jarr_snapshot.size();++k)
            {
                jo_bucket = jarr_snapshot.at(k).toObject();
                if(jo_bucket["bucketname"] != desBucketName)
                    continue;
                jarr_files = jo_bucket["files"].toArray();
                jarr_files.append(jo_file);
                jo_bucket.remove("files");
                jo_bucket.insert("files",jarr_files);
                jarr_snapshot.replace(k,jo_bucket);
                break;
            }
            snapshot_metadata.setArray(jarr_snapshot);
            break;
        }
        break;
    }
    //移动文件
    MoveControl *control = NULL;
    QEventLoop loop;
    for(int j=0;j<jarr_segments.size();j++)
    {
        QJsonObject segment = jarr_segments.at(j).toObject();
        int i;
        for(i=0;i<jarr_segmentsID.size();++i)
            if(segment["segmentid"].toString()==jarr_segmentsID.at(i).toString())
                break;
        if(i==jarr_segmentsID.size())
            continue;
        //先不判断desbucket中是否存在该段（有就覆盖）
        QJsonArray jarr_blocks = segment["blocks"].toArray();
        for(int k=0;k<jarr_blocks.size();k++)
        {
            if(control==NULL)
            {
                //此处应该是jarr_segmentsID.size()!!!
                //之前版本用的jarr_segments.size(),偷懒没改，找了好久才发现！
                control = new MoveControl(jarr_segmentsID.size()*jarr_blocks.size());
                emit moveProgressSetRange(0,jarr_segmentsID.size()*jarr_blocks.size());
                connect(control,SIGNAL(finished()),&loop,SLOT(quit()));
                connect(control,SIGNAL(subFinished_signal(int)),this,SLOT(subMoveFinished_slot(int)));
            }
            QJsonObject jo_block = jarr_blocks.at(k).toObject();
            QString blockID = jo_block["blockid"].toString();
            int cloudID = GetIdByName(jo_block["cloudname"].toString());
            //copy each object to desbucket
            QNetworkReply *reply = client[cloudID]->CopyObject(bucketName,blockID,desBucketName);
            control->Copy(reply);
        }
    }

    loop.exec();//copy完成才能delete
    //删除原文件和修改metadata
    DeleteForMoveFile(bucketName,fileName,desBucketName);
    return true;
}

void DataTransfer::subMoveFinished_slot(int value)
{
    emit moveProgress_signal(value);
}

void DataTransfer::DeleteForMoveFile(QString bucketName, QString fileName, QString desBucketName)
{
    CloudClient *client[cloudNum];
    for(int i=0;i<cloudNum;i++)
        client[i] = CreatCloudClient(clouds[i]);
    /*
    QJsonArray jarr_segmentpool = segmentpool_metadata.array();

    for(int i=0;i<jarr_segmentpool.size();i++)
    {
        QJsonObject jo_file = jarr_segmentpool.at(i).toObject();
        if(jo_file["bucketname"].toString()!=bucketName||jo_file["filename"].toString()!=fileName)
            continue;
        QJsonArray jarr_segments = jo_file["segments"].toArray();
        for(int j=0;j<jarr_segments.size();j++)
        {
            QJsonObject segment = jarr_segments.at(j).toObject();
            QJsonArray jarr_blocks = segment["blocks"].toArray();
            for(int k=0;k<jarr_blocks.size();k++)
            {

                QJsonObject jo_block = jarr_blocks.at(k).toObject();
                QString blockID = jo_block["blockid"].toString();
                int cloudID = GetIdByName(jo_block["cloudname"].toString());

                client[cloudID]->DeleteObject(bucketName,blockID);
            }
        }
        //更新segmentpool_metadata
        jo_file.remove("bucketname");
        jo_file.insert("bucketname",desBucketName);
        jarr_segmentpool.replace(i,jo_file);
        segmentpool_metadata.setArray(jarr_segmentpool);
        break;
    }
    //更新snapshot_metadata
    QJsonArray jarr_snapshot = snapshot_metadata.array();
    QJsonObject jo_file;
    for(int i=0;i<jarr_snapshot.size();i++)
    {
        QJsonObject jo_bucket = jarr_snapshot.at(i).toObject();
        if(jo_bucket["bucketname"] != bucketName)
            continue;
        QJsonArray jarr_files = jo_bucket["files"].toArray();
        for(int j=0;j<jarr_files.size();j++)
        {
            jo_file = jarr_files.at(j).toObject();
            if(jo_file["filename"]==fileName)
            {
                jarr_files.removeAt(j);
                jo_bucket.remove("files");
                jo_bucket.insert("files",jarr_files);
                jarr_snapshot.replace(i,jo_bucket);
                jo_file.remove("modified");
                //jo_file.remove("frequency");
                jo_file.insert("modified",QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate));
                //jo_file.insert("frequency",0);
                for(int k=0;k<jarr_snapshot.size();++k)
                {
                    jo_bucket = jarr_snapshot.at(k).toObject();
                    if(jo_bucket["bucketname"] != desBucketName)
                        continue;
                    jarr_files = jo_bucket["files"].toArray();
                    jarr_files.append(jo_file);
                    jo_bucket.remove("files");
                    jo_bucket.insert("files",jarr_files);
                    jarr_snapshot.replace(k,jo_bucket);
                    snapshot_metadata.setArray(jarr_snapshot);
                    break;
                }
                break;
            }
        }
        break;
    }*/
    QJsonArray jarr_segments = segmentpool_metadata.array();
    //找到文件的segments
    QJsonArray jarr_segmentsID;
    QJsonArray jarr_snapshot = snapshot_metadata.array();
    for(int i=0;i<jarr_snapshot.size();i++)
    {
        QJsonObject jo_bucket = jarr_snapshot.at(i).toObject();
        //因为在movefile中已经修改了bucketname→desbucketname
        if(jo_bucket["bucketname"] != desBucketName)
            continue;
        QJsonArray jarr_files = jo_bucket["files"].toArray();
        for(int j=0;j<jarr_files.size();j++)
        {
            QJsonObject jo_file = jarr_files.at(j).toObject();
            if(jo_file["filename"] != fileName)
                continue;
            jarr_segmentsID = jo_file["segments"].toArray();
            break;
        }
        break;
    }
    //在desbucket中新增segments（这样写已经处理了该文件中有相同段而在以前的文件中没有的情况）
    for(int i=0;i<jarr_segmentsID.size();++i)
    {
        int j;
        for(j=0;j<jarr_segments.size();++j)
        {
            QJsonObject segment = jarr_segments.at(j).toObject();
            if(jarr_segmentsID.at(i).toString()==segment["segmentid"].toString()
                    &&desBucketName==segment["bucketname"].toString())
            {
                int refCnt = segment["refcnt"].toInt();
                segment.remove("refcnt");
                segment.insert("refcnt",refCnt+1);
                jarr_segments.replace(j,segment);
                break;
            }
        }
        //没有找到，则需要新增segment
        if(j==jarr_segments.size())
        {
            for(int k=0;k<jarr_segments.size();++k)
            {
                QJsonObject segment = jarr_segments.at(k).toObject();
                if(jarr_segmentsID.at(i).toString()==segment["segmentid"].toString())
                {
                    segment.remove("bucketname");
                    segment.insert("bucketname",desBucketName);
                    segment.remove("refcnt");
                    segment.insert("refcnt",1);
                    jarr_segments.append(segment);
                    break;
                }
            }
        }
    }
    //删除原先的blocks(如果refcnt>1，则不能直接删除)
    for(int j=0;j<jarr_segmentsID.size();++j)
    {
       QJsonObject segment;
       int i;
       for(i=jarr_segments.size()-1;i>=0;--i)
       {
           segment = jarr_segments.at(i).toObject();
           if(segment["segmentid"].toString()==jarr_segmentsID.at(j).toString()
                   &&segment["bucketname"].toString()==bucketName)
               break;
       }
       int refCnt = segment["refcnt"].toInt();
       if(refCnt==1)
       {
           QJsonArray jarr_blocks = segment["blocks"].toArray();
           for(int k=0;k<jarr_blocks.size();k++)
           {
               QJsonObject jo_block = jarr_blocks.at(k).toObject();
               QString blockID = jo_block["blockid"].toString();
               int cloudID = GetIdByName(jo_block["cloudname"].toString());
               client[cloudID]->DeleteObject(bucketName,blockID);
           }
           //删除segment_medatata
           jarr_segments.removeAt(i);
       }
       else
       {
           segment.remove("refcnt");
           segment.insert("refcnt",refCnt-1);
           jarr_segments.replace(i,segment);
       }
    }

    segmentpool_metadata.setArray(jarr_segments);
    UploadMetadata();
}

bool DataTransfer::DeleteFile(QString bucketName,QString fileName)
{
    CloudClient *client[cloudNum];
    for(int i=0;i<cloudNum;i++)
        client[i] = CreatCloudClient(clouds[i]);
    //得到jarr_segmentsID并修改snapshot
    QJsonArray jarr_segmentsID;
    QJsonArray jarr_snapshot = snapshot_metadata.array();
    for(int i=0;i<jarr_snapshot.size();i++)
    {
        QJsonObject jo_bucket = jarr_snapshot.at(i).toObject();
        if(jo_bucket["bucketname"] != bucketName)
            continue;
        QJsonArray jarr_files = jo_bucket["files"].toArray();
        for(int j=0;j<jarr_files.size();j++)
        {
            QJsonObject jo_file = jarr_files.at(j).toObject();
            if(jo_file["filename"] != fileName)
                continue;
            jarr_segmentsID = jo_file["segments"].toArray();
            jarr_files.removeAt(j);
            jo_bucket.remove("files");
            jo_bucket.insert("files",jarr_files);
            jarr_snapshot.replace(i,jo_bucket);
            snapshot_metadata.setArray(jarr_snapshot);
            break;
        }
        break;
    }
    //删除blocks和修改segments
    QJsonArray jarr_segments = segmentpool_metadata.array();
    DeleteControl *deleteControl = new DeleteControl(jarr_segmentsID.size()*(partition+redundancy));
    connect(deleteControl,SIGNAL(subFinished_signal(int)),this,SLOT(deleteProgress_slot(int)));
    connect(deleteControl,SIGNAL(finished()),this,SLOT(deleteFinished_slot()));
    emit deleteProgressSetRange(0,jarr_segmentsID.size()*(partition+redundancy));
    for(int j=0;j<jarr_segmentsID.size();++j)
    {
        QJsonObject jo_segment;
        int i;
        for(i=jarr_segments.size()-1;i>=0;--i)
        {
            jo_segment = jarr_segments.at(i).toObject();
            if(jarr_segmentsID.at(j).toString()==jo_segment["segmentid"].toString()
                    &&bucketName==jo_segment["bucketname"].toString())
                break;
        }

        int refCnt = jo_segment["refcnt"].toInt();
        if(refCnt==1)
        {
            QJsonArray jarr_blocks = jo_segment["blocks"].toArray();
            for(int k=0;k<jarr_blocks.size();k++)
            {
                QJsonObject jo_block = jarr_blocks.at(k).toObject();
                QString blockID = jo_block["blockid"].toString();
                int cloudID = GetIdByName(jo_block["cloudname"].toString());
                QNetworkReply *reply = client[cloudID]->DeleteObject(bucketName,blockID);
                deleteControl->Delete(reply);
            }
            jarr_segments.removeAt(i);
        }
        else
        {
            jo_segment.remove("refcnt");
            jo_segment.insert("refcnt",refCnt-1);
            jarr_segments.replace(i,jo_segment);
            //此处默认每一个segment的blocks一样多！！
            for(int j=0;j<partition+redundancy;++j)
                deleteControl->Delete();
        }
    }
    segmentpool_metadata.setArray(jarr_segments);
    UploadMetadata();
    return true;
}

void DataTransfer::deleteFinished_slot()
{
    emit deleteFinished_signal();
}

void DataTransfer::deleteProgress_slot(int value)
{
    emit deleteProgress_signal(value);
}

void DataTransfer::LoadMetadata()
{
    if(!is_first_in)
        return;
    is_first_in = false;
    //初始化为空
    QByteArray segmentpool_data = "[]";
    QByteArray snapshot_data = "[]";
    //从一个可用的云上获取metadata
    for(int i=0;i<cloudNum;++i)
    {
        CloudClient *client = CreatCloudClient(clouds[i]);
        QNetworkReply *reply = client->GetObject(clouds[i].defaultBucket,"segmentpool");
        if(reply!=NULL)
        {
            QEventLoop eventLoop;
            connect(reply,SIGNAL(finished()),&eventLoop,SLOT(quit()));
            eventLoop.exec();
            if (reply->error() == QNetworkReply::NoError)
            {
                segmentpool_data = reply->readAll();
                segmentpool_metadata = QJsonDocument::fromJson(segmentpool_data);
                reply->deleteLater();
                reply->close();
                delete client;
                break;
            }
        }
        delete client;
    }
    ////////////////////////////////////////////////////////////////////////
    for(int i=0;i<cloudNum;++i)
    {
        CloudClient *client = CreatCloudClient(clouds[i]);
        QNetworkReply *reply = client->GetObject(clouds[i].defaultBucket,"snapshot");
        if(reply!=NULL)
        {
            QEventLoop eventLoop;
            connect(reply,SIGNAL(finished()),&eventLoop,SLOT(quit()));
            eventLoop.exec();
            if (reply->error() == QNetworkReply::NoError)
            {
                snapshot_data = reply->readAll();
                snapshot_metadata = QJsonDocument::fromJson(snapshot_data);
                reply->deleteLater();
                reply->close();
                delete client;
                break;
            }
        }
        delete client;
    }
}


void DataTransfer::UpdateMetadataForUpload()
{
    QJsonArray jarr_snapshot = snapshot_metadata.array();
    for(int i=0;i<jarr_snapshot.size();i++)
    {
        QJsonObject jo_bucket = jarr_snapshot.at(i).toObject();
        if(jo_bucket["bucketname"] != snapshot.bucket)
            continue;
        QJsonArray jarr_files = jo_bucket["files"].toArray();
        /*if(exist)
        {
            for(int j=0;j<jarr_files.size();j++)
            {
                QJsonObject jo_file = jarr_files.at(j).toObject();
                if(jo_file["filename"]==snapshot.fileName)
                {
                    jarr_files.removeAt(j);
                    break;
                }
            }
        }*/
        QJsonObject jo;
        jo.insert("filename",snapshot.fileName);
        jo.insert("filesize",(int)snapshot.fileSize);
        jo.insert("modified",snapshot.modified);
        jo.insert("filedownloadsize",totalDownloadSize);
        jo.insert("partition",partition);
        jo.insert("redundancy",redundancy);
        jo.insert("frequency",0);//访问次数
        jo.insert("createtime",snapshot.modified);
        jo.insert("occupiedspace",occupiedspace);
        QJsonArray jarr_segmentsID;
        for(int j=0;j<segments.size();++j)
        {
            Segment segment = segments[j];
            jarr_segmentsID.append(QJsonValue(segment.segmentID.data()));
        }
        jo.insert("segments",jarr_segmentsID);
        jarr_files.append(jo);
        jo_bucket.remove("files");
        jo_bucket["files"] = jarr_files;
        jarr_snapshot.replace(i,jo_bucket);
        snapshot_metadata.setArray(jarr_snapshot);
        break;
    }
    ///////////////////////////////////////////////////////////
    /// \brief jarr_files
    //更改segmentpool
    //QJsonArray jarr_files = segmentpool_metadata.array();
    //如果存在则先删除
    /*if(exist)
    {
        for(int i=0;i<jarr_files.size();i++)
        {
            QJsonObject jo_file = jarr_files.at(i).toObject();
            if(jo_file["bucketname"].toString()==snapshot.bucket
                    &&jo_file["filename"].toString()==snapshot.fileName)
            {
                jarr_files.removeAt(i);
                break;
            }
        }
    }*/
    //QJsonObject jo_file;
    //jo_file.insert("filename",snapshot.fileName);
    //jo_file.insert("bucketname",snapshot.bucket);
    //QJsonArray jarr_segments;
    QJsonArray jarr_segments = segmentpool_metadata.array();
    for(int i=0;i<segments.size();i++)
    {       
        Segment segment = segments[i];
        if(segment.refCnt==1)
        {
            QJsonObject jo_segment;
            jo_segment.insert("segmentid",QString(segment.segmentID));
            jo_segment.insert("bucketname",snapshot.bucket);
            //jo_segment.insert("blocknum",segment.blockNum);
            jo_segment.insert("segmentlength",segment.segmentLength);
            jo_segment.insert("blocklength",segment.blockLength);
            jo_segment.insert("refcnt",1);//引用次数
            QJsonArray jarr_blocks;
            for(int j=0;j<segment.blockNum;j++)
            {
                Block block = segment.blocks[j];
                QJsonObject jo_block;
                jo_block.insert("blockid",QString(block.blockID));
                jo_block.insert("cloudname",clouds[block.cloudID].cloudName);
                jarr_blocks.append(jo_block);
            }
            jo_segment.insert("blocks",jarr_blocks);
            jarr_segments.append(jo_segment);
        }
        else
        {
            for(int j=0;j<jarr_segments.size();++j)
            {
                QJsonObject jo_segment = jarr_segments.at(j).toObject();
                if(jo_segment["segmentid"].toString().toLatin1()==segment.segmentID
                        &&snapshot.bucket==jo_segment["bucketname"].toString())
                {
                    jo_segment.remove("refcnt");
                    jo_segment.insert("refcnt",segment.refCnt);
                    jarr_segments.replace(j,jo_segment);
                    break;
                }
            }
        }
    }
    //jo_file.insert("segments",jarr_segments);
    //jarr_files.append(jo_file);
    //segmentpool_metadata.setArray(jarr_files);
    segmentpool_metadata.setArray(jarr_segments);
}

void DataTransfer::UpdataMetadataForDownload(QString bucketName,QString fileName)
{
    //更改snapshot
    QJsonArray jarr_snapshot = snapshot_metadata.array();
    for(int i=0;i<jarr_snapshot.size();i++)
    {
        QJsonObject jo_bucket = jarr_snapshot.at(i).toObject();
        if(jo_bucket["bucketname"] != bucketName)
            continue;
        QJsonArray jarr_files = jo_bucket["files"].toArray();

        for(int j=0;j<jarr_files.size();j++)
        {
            QJsonObject jo_file = jarr_files.at(j).toObject();
            if(jo_file["filename"]!=fileName)
                continue;
            int frequency = jo_file["frequency"].toInt()+1;
            jo_file.remove("frequency");
            jo_file.insert("frequency",frequency);

            jarr_files.replace(j,jo_file);
            jo_bucket.remove("files");
            jo_bucket.insert("files",jarr_files);
            jarr_snapshot.replace(i,jo_bucket);
            snapshot_metadata.setArray(jarr_snapshot);
            break;
        }
        break;
    }
}

void DataTransfer::UploadMetadata()
{
    CloudClient *client;
    for(int i=0;i<cloudNum;i++)
    {
        client=CreatCloudClient(clouds[i]);
        client->PutObject(clouds[i].defaultBucket,"snapshot",snapshot_metadata.toJson(QJsonDocument::Compact));;
        client->PutObject(clouds[i].defaultBucket,"segmentpool",segmentpool_metadata.toJson(QJsonDocument::Compact));
    }
}

bool DataTransfer::IsExistBucket(QString bucketName)
{
    QJsonArray buckets = snapshot_metadata.array();
    for(int i=0;i<buckets.size();i++)
    {
        QJsonObject bucket = buckets.at(i).toObject();
        if(bucket["bucketname"].toString()==bucketName)
            return true;
    }
    return false;
}

bool DataTransfer::Rename(QString bucketName, QString fileName, QString newName)
{
    //更新snapshot_metadata
    QJsonArray jarr_snapshot = snapshot_metadata.array();
    QJsonObject jo_file;
    for(int i=0;i<jarr_snapshot.size();i++)
    {
        QJsonObject jo_bucket = jarr_snapshot.at(i).toObject();
        if(jo_bucket["bucketname"] != bucketName)
            continue;
        QJsonArray jarr_files = jo_bucket["files"].toArray();
        for(int j=0;j<jarr_files.size();j++)
        {
            jo_file = jarr_files.at(j).toObject();
            if(jo_file["filename"]==fileName)
            {
                jo_file.remove("filename");
                jo_file.remove("modified");
                jo_file.insert("filename",newName);
                jo_file.insert("modified",QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate));
                jarr_files.replace(j,jo_file);
                break;
            }
        }
        jo_bucket.remove("files");
        jo_bucket.insert("files",jarr_files);
        jarr_snapshot.replace(i,jo_bucket);
        break;
    }
    snapshot_metadata.setArray(jarr_snapshot);

    UploadMetadata();
    return true;
}

vector<Bucket> DataTransfer::GetBucket()
{
    QJsonArray jarr_buckets = snapshot_metadata.array();
    buckets.clear();
    for(int i=0;i<jarr_buckets.size();i++)
    {
        QJsonObject jo_bucket = jarr_buckets.at(i).toObject();
        Bucket bucket;
        bucket.bucketName =jo_bucket["bucketname"].toString();
        bucket.region =jo_bucket["region"].toString();
        bucket.storageClass = jo_bucket["storageclass"].toString();
        buckets.push_back(bucket);
    }
    return buckets;
}

