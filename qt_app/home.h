#ifndef HOME_H
#define HOME_H

#include <QMainWindow>
#include <QCamera>
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include<QUdpSocket>
#include<QFile>
#include<QTextCursor>
#include<QImage>
#include<QJsonDocument>
#include<QJsonParseError>
#include<QJsonObject>
#include<QTcpSocket>
namespace Ui {
class Home;
}

class Home : public QMainWindow
{
    Q_OBJECT

public:
    explicit Home(QWidget *parent = 0);
    ~Home();
    void searchCamera();//函数声明
    explicit Home(QString ip,QString port,QWidget *parent=0) ;
public slots:
    void slotImageCaptured(int id,const QImage &preview);
    void slotUdpReadyread();
    void connectTcpsocket();
    void slotTcpReadyread();
private slots:
    void on_pushButtoncamera_clicked();

    void on_pushButtoncapture_clicked();

    void on_pushButtonclose_clicked();

    void on_textBrowser_textChanged();

private:
    Ui::Home *ui;
    QCamera *camera;//指向摄像头对象
    QList<QCameraInfo> cameraInfoList;//存放摄像头信息的容器
    QCameraViewfinder *cameraViewfinder;//取景器
    QCameraImageCapture *cameraImageCapture;//指向摄像头抓拍图片的对象
    QUdpSocket *udpsocket;
    QImage paintimage;
    QTcpSocket *tcpsocket;
    int  num;
    int flag_stop;
};

#endif // HOME_H
