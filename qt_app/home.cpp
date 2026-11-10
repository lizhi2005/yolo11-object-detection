#include "home.h"
#include "ui_home.h"
#include "home.h"
#include<QDebug>
#include<QFile>
#include<QTextCursor>
#include<QUdpSocket>
#include<QPainter>
Home::Home(QString ip,QString port,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Home)
    ,udpsocket(new QUdpSocket(this)),
    tcpsocket(new QTcpSocket(this)),
    num(-1),
    flag_stop(0)
{
    ui->setupUi(this);
    this->resize(1000,800);

    ui->pushButtoncapture->setDisabled(true);


    camera=NULL;
    cameraImageCapture=NULL;
    cameraViewfinder=new QCameraViewfinder(ui->widget);


        // 左边抓拍显示框 → 固定大小，永远不变
     ui->label_2->setFixedSize(500, 350);
     ui->label_2->setScaledContents(true); // 图片自动填满

    connect(udpsocket,&QUdpSocket::readyRead,this,&Home::slotUdpReadyread);

    connect(tcpsocket,&QTcpSocket::connected,[&](){
        ui->textBrowser->insertPlainText("connect netgate ok\r\n");
        tcpsocket->write("Qt",sizeof("Qt"));
    });

    connect(tcpsocket,&QTcpSocket::readyRead,this,&Home::slotTcpReadyread);
//    connect(tcpsocket,&QTcpSocket::readyRead,[&](){
//        int recvSize=tcpsocket->bytesAvailable();
//        ui->textBrowser->insertPlainText("recvSize:"+QString::number(recvSize)+"\r\n");
//        char buf[1024]={0};
//        tcpsocket->read(buf,recvSize);
//        ui->textBrowser->insertPlainText("recvData:"+QString(buf)+"\r\n");

//    });

    //搜索当前主机可用摄像头
    searchCamera();
    //connectTcpsocket();
    tcpsocket->connectToHost(ip,port.toInt());

}

void Home::connectTcpsocket()
{

    tcpsocket->connectToHost("192.168.79.217",9999);

}
void Home::slotTcpReadyread()
{

//    判断是否为-1,程序启动时num=-1，如果程序没有对象那么接收到的num为零
    if(num==-1)
    {
       tcpsocket->read((char*)&num,sizeof(num));
       ui->textBrowser->insertPlainText("num:"+QString::number(num)+"\r\n");
//     tcpsocket->read(&num,sizeof(num));
       if(num==0)
       {
           num=-1;
           if(flag_stop==0)
           {
               cameraImageCapture->capture();
           }
//            cameraImageCapture->capture();

       }
        return ;
    }
    num--;

    char buf[2048]={0};
    tcpsocket->read(buf,sizeof(buf));

    if (QString(buf) == "-1")
    {
        ui->textBrowser->insertPlainText("没有识别到任何物体\r\n");
        return ;
    }
    //ui->textBrowser->insertPlainText(QString("recvResult:")+buf+"\r\n");
    qDebug()<<QString("recvResult:")+buf;

    QJsonParseError e;

    QJsonDocument jsonDocu =  QJsonDocument::fromJson(QByteArray(buf),&e);
    if(e.error!=QJsonParseError::NoError)
    {
        ui->textBrowser->insertPlainText("识别图片出错\r\n");
        ui->textBrowser->insertPlainText("json数据出错了\r\n");
        return;
    }
    QJsonObject jsonObj =jsonDocu.object();
    int cls_idx=jsonObj["cls_idx"].toInt();
    QString cls_name=jsonObj["cls_name"].toString();
    double conf=jsonObj["conf"].toDouble();
    double lx=jsonObj["lx"].toDouble();
    double ly=jsonObj["ly"].toDouble();
    double rx=jsonObj["rx"].toDouble();
    double ry=jsonObj["ry"].toDouble();

    qDebug()<<cls_idx<<cls_name<<conf<<lx<<ly<<rx<<ry;
    QPainter painter(&paintimage);
    QPen pen;
    pen.setWidth(5);
    pen.setColor(QColor(125,125,255));
    painter.setPen(pen);
    painter.drawRect(QRectF(QPoint(lx,ly),QPoint(rx,ry)));
    QFont font=painter.font();
    font.setPixelSize(32);
    painter.setFont(font);
    painter.drawText(QRectF(QPoint(lx,ly),QPoint(rx,ry)),cls_name+""+QString::number(conf));
    //painter.drawText(QRectF(QPoint(lx+100,ly),QPoint(rx,ry)),QString::number(conf));

    painter.end();

    ui->label_2->setPixmap(QPixmap::fromImage(paintimage));
//    一张图片所有目标全部接收完毕
    if(num==0)
    {
       num=-1;
       //处理完结果后，进行下次抓拍
       if(flag_stop==0)
       {
           cameraImageCapture->capture();
       }
// cameraImageCapture->capture();
    }
}

void Home::slotUdpReadyread()
{
    char buf[2048]={0};
    udpsocket->readDatagram(buf,sizeof(buf));
    //ui->textBrowser->insertPlainText(QString("recvResult:")+buf+"\r\n");
    qDebug()<<QString("recvResult:")+buf;
    QJsonParseError e;

    QJsonDocument jsonDocu =  QJsonDocument::fromJson(QByteArray(buf),&e);
    if(e.error!=QJsonParseError::NoError)
    {
        ui->textBrowser->insertPlainText("识别图片出错\r\n");
        ui->textBrowser->insertPlainText("json数据出错了\r\n");
    }
    QJsonObject jsonObj =jsonDocu.object();
    int cls_idx=jsonObj["cls_idx"].toInt();
    QString cls_name=jsonObj["cls_name"].toString();
    double conf=jsonObj["conf"].toDouble();
    double lx=jsonObj["lx"].toDouble();
    double ly=jsonObj["ly"].toDouble();
    double rx=jsonObj["rx"].toDouble();
    double ry=jsonObj["ry"].toDouble();

    qDebug()<<cls_idx<<cls_name<<conf<<lx<<ly<<rx<<ry;
    QPainter painter(&paintimage);
    QPen pen;
    pen.setWidth(5);
    pen.setColor(QColor(125,125,255));
    painter.setPen(pen);
    painter.drawRect(QRectF(QPoint(lx,ly),QPoint(rx,ry)));
    QFont font=painter.font();
    font.setPixelSize(32);
    painter.setFont(font);
    painter.drawText(QRectF(QPoint(lx,ly),QPoint(rx,ry)),cls_name+""+QString::number(conf));
    //painter.drawText(QRectF(QPoint(lx+100,ly),QPoint(rx,ry)),QString::number(conf));

    painter.end();

    ui->label_2->setPixmap(QPixmap::fromImage(paintimage));



}


Home::~Home()
{
    delete ui;
}
void Home::searchCamera()
{
    cameraInfoList=QCameraInfo::availableCameras();
    for(int i=0;i<cameraInfoList.size();i++)
    {
        //打印可用摄像头的描述信息
        qDebug()<<cameraInfoList[i].description();
        ui->comboBox->insertItem(i,cameraInfoList[i].description());

    }
}

void Home::on_pushButtoncamera_clicked()
{
    if(camera != NULL)
    {
       camera->stop();
       delete camera;
       delete cameraImageCapture;
    }
    //初始化摄像头对象
    camera=new QCamera(cameraInfoList[ui->comboBox->currentIndex()]);
    //设置取景器
    camera->setViewfinder(cameraViewfinder);
    //设置取景器大小
    cameraViewfinder->resize(ui->widget->size());
    //选择拍摄模式
    camera->setCaptureMode(QCamera::CaptureVideo);
    //开启摄像头
    camera->start();
    //选择从指定摄像头抓拍图片
    cameraImageCapture=new QCameraImageCapture(camera);
    //信号与槽手动连接
    connect(cameraImageCapture,&QCameraImageCapture::imageCaptured,this,&Home::slotImageCaptured);
    ui->pushButtoncapture->setDisabled(false);

}
void Home::slotImageCaptured(int id,const QImage &preview)
{
    Q_UNUSED(id);
    QImage img(preview);

    img=img.scaled(ui->label_2->size(),Qt::IgnoreAspectRatio);
    ui->label_2->setPixmap(QPixmap::fromImage((img)));//绘制到标签上



    paintimage=img;
    img.save("t1.png");
    QFile imgFile("t1.png");
    if(imgFile.open(QFile::ReadOnly)==false)
    {
       ui->textBrowser->insertPlainText("文件打开失败！\r\n");
       return;
    }


    QByteArray imgData=imgFile.readAll();
    ui->textBrowser->insertPlainText("文件大小："+QString::number(imgData.size())+"bytes\r\n");

    //udpsocket->writeDatagram(imgData,QHostAddress("192.168.157.1"),6666);//发送到yolo上
    int fileSize=imgData.size();
    tcpsocket->write((char*)&fileSize,sizeof(fileSize));
    tcpsocket->write(imgData);

    flag_stop=0;

}


void Home::on_pushButtoncapture_clicked()
{
    cameraImageCapture->capture();


}

void Home::on_pushButtonclose_clicked()
{
 //udpsocket->writeDatagram(QByteArray("stop"),QHostAddress("192.168.157.1"),6666);
     flag_stop=1;
}



void Home::on_textBrowser_textChanged()
{
    ui->textBrowser->moveCursor(QTextCursor::End);
}
