#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QDebug>
#include "home.h"
#include <QGraphicsDropShadowEffect> // 阴影效果头文件
#include <QMouseEvent> // 窗口拖动头文件
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()//自动链接方式
{
//    QString user=ui->lineEdit->text();
//    QString password=ui->lineEdit_2->text();


//    qDebug()<<user<<password;

    QString ip=ui->lineEdit->text();
    QString port=ui->lineEdit_2->text();

    qDebug()<<port<<ip;
//字符串的匹配  判断是否成功  功能自己去完成


//    Home h;
//    h.show();
  //  Home *h=new Home;//注意空间管理  放在堆区
    Home *h=new Home(ip,port);
    h->show();
    this->close();

}
