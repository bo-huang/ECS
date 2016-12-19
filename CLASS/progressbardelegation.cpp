#include "progressbardelegation.h"
#include <QTime>

ProgressBarDelegation::ProgressBarDelegation(QProgressBar *progressBar,QTableWidgetItem * speed)
    :progressBar(progressBar),speedItem(speed)
{
    nowTime.start();
}

void ProgressBarDelegation::AddValue(int value)
{
    int time = nowTime.elapsed();
    /*if(progressBar->value()==0)//忽略掉下载前的处理时间（即之后从第二个数据块开始算）
        nowTime.start();
    //不过似乎没什么效果……
    */
    progressBar->setValue(progressBar->value()+value);
    double speed = progressBar->value()*1000.0/time;
    speedItem->setText(ToString(speed));
    if(progressBar->value()==progressBar->maximum())
    {
        speedItem->setText("0B/s");
        emit finished();
    }
}

void ProgressBarDelegation::beginUpload()
{
    nowTime.start();//开始计时
}

QString ProgressBarDelegation::ToString(double speed)
{
    if(speed<1000)
        return QString::number(speed,'d',2)+" B/s";
    else if(speed>=1000&&speed<1000000)
        return QString::number(speed/1000,'d',2)+" KB/s";
    else
        return QString::number(speed/1000000,'d',2)+" MB/s";
}
