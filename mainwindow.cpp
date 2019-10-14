
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "relay_waveshare.h"
#include "relay_seed_ddl.h"
#include "relay_seed.h"
#include <iostream>
#include <QTextEdit>
#include <QThread>
#include <QDateTime>
#include <QApplication>
#include <QElapsedTimer>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), measure_relay(new relay_waveshare), comm_relay(new relay_seed_ddl)
{
    ui->setupUi(this);

    measure_relay_i2c = new relay_seed;

    mesure_time_check = new QElapsedTimer;

    QPalette palette = ui->label->palette();

    measure_port_reset();
    comm_port_reset();

    camera_timer = new QTimer(this);
    detect_on_timer = new QTimer(this);
    work_on_timer = new QTimer(this);
    third_on_timer = new QTimer(this);
    detect_off_timer = new QTimer(this);
    port_reset_timer = new QTimer(this);

    detect_on_timer->setSingleShot(true);
    work_on_timer->setSingleShot(true);
    third_on_timer->setSingleShot(true);
    detect_off_timer->setSingleShot(true);
    port_reset_timer->setSingleShot(true);

    //detect -> (5 sec) -> work_on -> (1 sec) -> third on -> (8 sec) -> detect off -> (10 sec)

    detect_on_timer->setInterval(0);
    work_on_timer->setInterval(5000);
    third_on_timer->setInterval(6000);
    detect_off_timer->setInterval(14000);
    port_reset_timer->setInterval(34000);

    ui->test_stop->setEnabled(false);
    ui->camera_stop->setEnabled(false);
    ui->device_close->setEnabled(false);
    ui->Capture_on->setEnabled(false);
    ui->Capture_off->setEnabled(false);

    palette.setColor(QPalette::WindowText, Qt::yellow);
    palette.setColor(QPalette::Window, Qt::black);

    ui->label->setAlignment(Qt::AlignCenter);
    ui->label->setAutoFillBackground(true);
    ui->label->setPalette(palette);
    ui->label->setText("Camera is Closed");

    ui->textEdit->setStyleSheet("background-color:black;");
    ui->textEdit->setTextColor("yellow");
    ui->build_date->setText("V0.0.8");
}

MainWindow::~MainWindow()
{
    delete ui;
}

#if 0
void MainWindow::on_device_check_clicked()
{
    ui->textEdit->clear();
    comm_connect();

//    Test for USB connection
//    QTimer::singleShot(2000, this, SLOT(comm_port_reset()));
}
#endif

void MainWindow::on_test_start_clicked()
{    
    connect(this, SIGNAL(measure_start()), this, SLOT(measurement()));
    connect(this, SIGNAL(measure_check()), this, SLOT(measure_count_check()));
    connect(this, SIGNAL(measure_end()), this, SLOT(on_test_stop_clicked()));

    connect(detect_on_timer, SIGNAL(timeout()),SLOT(detect_on()));
    connect(work_on_timer, SIGNAL(timeout()),SLOT(work_on()));
    connect(third_on_timer, SIGNAL(timeout()),SLOT(third_on()));
    connect(detect_off_timer, SIGNAL(timeout()),SLOT(detect_off()));
    connect(port_reset_timer, SIGNAL(timeout()),SLOT(measure_port_reset()));

    ui->textEdit->clear();
    ui->textEdit->append("measure start");
    ui->test_start->setEnabled(false);
    ui->device_close->setEnabled(false);
    ui->test_stop->setEnabled(true);

    emit measure_start();
}

void MainWindow::on_test_stop_clicked()
{   
    disconnect(this, SIGNAL(measure_start()), this, SLOT(measurement()));
    disconnect(this, SIGNAL(measure_check()), this, SLOT(measure_count_check()));
    disconnect(this, SIGNAL(measure_end()), this, SLOT(on_test_stop_clicked()));

    detect_on_timer->stop();
    work_on_timer->stop();
    third_on_timer->stop();
    detect_off_timer->stop();
    port_reset_timer->stop();

    disconnect(detect_on_timer, SIGNAL(timeout()),this,SLOT(detect_on()));
    disconnect(work_on_timer, SIGNAL(timeout()),this,SLOT(work_on()));
    disconnect(third_on_timer, SIGNAL(timeout()),this,SLOT(third_on()));
    disconnect(detect_off_timer, SIGNAL(timeout()),this,SLOT(detect_off()));
    disconnect(port_reset_timer, SIGNAL(timeout()),this,SLOT(measure_port_reset()));

    if(measure_coount ==1000)
        ui->textEdit->append("Measurement count is  " + (QString::number(measure_coount)));

    measure_coount = 0;    

    ui->test_start->setEnabled(true);
    ui->test_stop->setEnabled(false);

    measure_port_reset();
    comm_port_reset();

    if(measure_coount < 1000)
        ui->textEdit->clear();

    ui->textEdit->append("measure stopped");
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
        ui->camera_start->setEnabled(false);
        ui->camera_stop->setEnabled(true);
        ui->Capture_on->setEnabled(true);
        connect(camera_timer, SIGNAL(timeout()), this, SLOT(update_camera()));
//      camera_timer->start(20);
        camera_timer->start(50);

    }
}

void MainWindow::on_camera_stop_clicked()
{
    disconnect(camera_timer, SIGNAL(timeout()), this, SLOT(update_camera()));

    ui->camera_start->setEnabled(true);
    ui->camera_stop->setEnabled(false);

    ui->Capture_on->setEnabled(false);
    ui->Capture_off->setEnabled(false);

    cap.release();
    Mat image = Mat::zeros(frame.size(), CV_8UC3);
    qt_image = QImage((const unsigned char*)(image.data), image.cols, image.rows, QImage::Format_RGB888);
    ui->label->setPixmap(QPixmap::fromImage(qt_image));
//  ui->label->resize(ui->label->pixmap()->size());       //resize the camera display
//  ui->label->setStyleSheet("color: yellow");            //font color
    ui->label->setAlignment(Qt::AlignCenter);
    ui->label->setText("Camera is Closed");
}

void MainWindow::update_camera()
{    
    cap >> frame;
    cvtColor(frame, frame, COLOR_BGR2RGB);

    if(capture_flag)
    {
         QDateTime Current_Time = QDateTime::currentDateTime();
         QString filename = "/home/pi/capture_file/" + Current_Time.toString("yyyyMMddhhmmsszzz") + ".jpg";
         imwrite(filename.toStdString(),frame);
    }

    qt_image = QImage((const unsigned char*)(frame.data), frame.cols, frame.rows, QImage::Format_RGB888);
    ui->label->setPixmap(QPixmap::fromImage(qt_image));
//  ui->label->resize(ui->label->pixmap()->size());       //resize the camera display
}

void MainWindow::measurement()
{
     detect_on_timer->start();
     work_on_timer->start();
     third_on_timer->start();
     detect_off_timer->start();
     port_reset_timer->start();

     mesure_time_check->start();

//     measure_coount++;
}

void MainWindow::detect_on()
{
    cout<<"detect on : "<< mesure_time_check->elapsed()<<endl;
    measure_relay->measure_work(measure_relay->relay_channel::DETECT, CH_ON);
    measure_relay_i2c->measure_work(measure_relay_i2c->relay_channel::DETECT, Relay_On);
}

void MainWindow::work_on()
{
     cout<<"work on : "<< mesure_time_check->elapsed() <<endl;
     measure_relay->measure_work(measure_relay->relay_channel::WORK, CH_ON);
     measure_relay_i2c->measure_work(measure_relay_i2c->relay_channel::WORK, Relay_On);
}

void MainWindow::third_on()
{
    cout<<"third on : "<< mesure_time_check->elapsed() <<endl;
    measure_relay->measure_work(measure_relay->relay_channel::THIRD, CH_ON);
    measure_relay_i2c->measure_work(measure_relay_i2c->relay_channel::THIRD, Relay_On);
}

void MainWindow::detect_off()
{
    cout<<"detect off : "<< mesure_time_check->elapsed() <<endl;
    measure_relay->measure_work(measure_relay->relay_channel::DETECT, CH_OFF);
    measure_relay_i2c->measure_work(measure_relay_i2c->relay_channel::DETECT, Relay_Off);
    measure_relay_i2c->reg_data = 0xff;
}

void MainWindow::measure_port_reset()
{
    cout<<"measure port reset : "<< mesure_time_check->elapsed() <<endl;
    measure_relay->measure_port_reset();
    measure_relay_i2c->measure_port_reset();
    emit measure_check();
}

void MainWindow:: measure_count_check()
{
      measure_coount++;

//    ui->textEdit->setTextCursor();
      ui->textEdit->setText("Measurement count is  " + (QString::number(measure_coount)));
//    ui->textEdit->append("Measurement count is  " + (QString::number(measure_coount)));

    if(measure_coount < 1000)
    {
        emit measure_start();
    }
    else if(measure_coount == 1000)
    {
        emit measure_end();
    }
}

void MainWindow::comm_connect()
{
//    for(quint8 channel=0x5; channel>0; channel--)
//        relay->work(relay->fd_comm, channel ,CH_ON);
    comm_relay->comm_port_open();
}

void MainWindow::comm_port_reset()
{
     comm_relay->comm_port_reset();
}

void MainWindow::on_quit_clicked()
{
   QCoreApplication::quit();
}

void MainWindow::on_Capture_on_clicked()
{
    capture_flag = 0x1;
    ui->Capture_on->setEnabled(false);
    ui->Capture_off->setEnabled(true);
}

void MainWindow::on_Capture_off_clicked()
{
    capture_flag = 0x0;
    ui->Capture_on->setEnabled(true);
    ui->Capture_off->setEnabled(false);
}

void MainWindow::on_device_open_clicked()
{
    ui->device_open->setEnabled(false);
    ui->device_close->setEnabled(true);
    comm_connect();
}

void MainWindow::on_device_close_clicked()
{
    ui->device_open->setEnabled(true);
    ui->device_close->setEnabled(false);
    comm_port_reset();
}
