#include "uploadassist.h"
#include <CloudSDK/aliyunclient.h>
#include <CloudSDK/azureclient.h>
#include <CloudSDK/googleclient.h>
#include <CloudSDK/s3client.h>
#include <CloudSDK/baiduyunclient.h>
#include <CORE/uploadcontrol.h>

UploadAssist::UploadAssist(QList<CloudInfo> &clouds,QString bucketName)
    :clouds(clouds),bucketName(bucketName)
{
    manger = NULL;
    client = NULL;
    connect(this,SIGNAL(put_signal(int,QByteArray,QByteArray)),this,SLOT(put_slot(int,QByteArray,QByteArray)));
}

void UploadAssist::SetBuckets(const std::vector<Bucket>& buckets)
{
    this->buckets = buckets;
}

void UploadAssist::Upload(int cloudID,QByteArray blockID,QByteArray data)
{
    emit put_signal(cloudID,blockID,data);
}
//不上传，但需要发送信号。进度条需要
void UploadAssist::Upload(int size)
{
    subUploadFinished_slot(size);
}

void UploadAssist::put_slot(int cloudID,QByteArray blockID, QByteArray data)
{
    //init(can not init in UploadAssist)

    if(manger==NULL)
        manger = new QNetworkAccessManager;
    if(client==NULL)
    {
        client = new CloudClient*[clouds.size()];
        for(int i=0;i<clouds.size();++i)
            client[i]=CreatCloudClient(clouds[i]);
    }
    QNetworkReply *reply = client[cloudID]->PutObject(bucketName,blockID,data);
    UploadControl *uploadControl = new UploadControl(reply,cloudID,data.length());
    connect(uploadControl,SIGNAL(subFinished(int)),this,SLOT(subUploadFinished_slot(int)));

}

void UploadAssist::subUploadFinished_slot(int size)
{
    emit subFinished(size);
}

CloudClient* UploadAssist::CreatCloudClient(CloudInfo &cloud)
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
