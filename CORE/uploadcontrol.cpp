 #include "uploadcontrol.h"

UploadControl::UploadControl(QNetworkReply *reply,int cloudID,int blockSize)
    :reply(reply),cloudID(cloudID),blockSize(blockSize)
{
    connect(reply,SIGNAL(finished()),this,SLOT(finished()));
}
void UploadControl::finished()
{
    if(reply->error()==QNetworkReply::NoError)
        //emit subFinished(cloudID,totalBlocks);
        emit subFinished(blockSize);
    else
        qDebug()<<QString("cloud %1 upload error").arg(cloudID);
    reply->deleteLater();
    reply->close();
}
