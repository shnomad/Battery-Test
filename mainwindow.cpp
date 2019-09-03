#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "seed_relay.h"
#include <QTextEdit>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), relay(new seed_relay)
{
    ui->setupUi(this);
    QTextEdit *textEdit = new QTextEdit(this);

    timer = new QTimer(this);

    ui->textEdit->setStyleSheet("background-color:black;");
    ui->textEdit->setTextColor("yellow");
    relay->port_reset();

    ui->textEdit->append("First Checking the Device Conntection");

//    relay->port_reset();

//    relay->work(CH1_DETECT, CH_ON);
//    relay->work(CH1_DETECT, CH_OFF);

//    relay->work(CH2_WORK, CH_ON);
//    relay->work(CH2_WORK, CH_OFF);

//    relay->work(CH3_THIRD, CH_ON);
//    relay->work(CH3_THIRD, CH_OFF);

//    ui->textEdit->append("QT program test1");
//    ui->textEdit->append("QT program test1");
//    ui->textEdit->append("QT program test1");
//    ui->textEdit->append("QT program test1");
//    ui->textEdit->append("QT program test1");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::PrintMessage(quint8 messageType)
{

}

void MainWindow::on_device_check_clicked()
{
    ui->textEdit->clear();
}

void MainWindow::on_test_start_clicked()
{

}

void MainWindow::on_camera_start_clicked()
{
    cap.open(0);

    if(!cap.isOpened())
    {
        ui->label->setText("camera is not open");
        ui->camera_stop->setEnabled(false);
    }
    else
    {
        ui->camera_stop->setEnabled(true);
        connect(timer, SIGNAL(timeout()), this, SLOT(update_camera()));
        timer->start(20);
    }
}

void MainWindow::on_camera_stop_clicked()
{
    disconnect(timer, SIGNAL(timeout()), this, SLOT(update_camera()));
    cap.release();
    Mat image = Mat::zeros(frame.size(), CV_8UC3);
    qt_image = QImage((const unsigned char*)(image.data), image.cols, image.rows, QImage::Format_RGB888);
    ui->label->setPixmap(QPixmap::fromImage(qt_image));
//  ui->label->resize(ui->label->pixmap()->size());       //resize the camera display
//  cout<<"camera is closed"<<endl;
}

void MainWindow::update_camera()
{
    cap >> frame;
    cvtColor(frame, frame, COLOR_BGR2RGB);
    qt_image = QImage((const unsigned char*)(frame.data), frame.cols, frame.rows, QImage::Format_RGB888);
    ui->label->setPixmap(QPixmap::fromImage(qt_image));
//  ui->label->resize(ui->label->pixmap()->size());       //resize the camera display
}
