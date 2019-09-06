
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "seed_relay.h"
#include <QTextEdit>
#include <QThread>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), relay(new seed_relay)
{
    ui->setupUi(this);

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

    //detect -> (5 sec) -> work_on -> (1 sec) -> third on -> (8 sec) -> detect off -> (20 sec)

    detect_on_timer->setInterval(0);
    work_on_timer->setInterval(5000);
    third_on_timer->setInterval(6000);
    detect_off_timer->setInterval(14000);
    port_reset_timer->setInterval(34000);

    palette.setColor(QPalette::WindowText, Qt::yellow);
    palette.setColor(QPalette::Window, Qt::black);

    ui->label->setAlignment(Qt::AlignCenter);
    ui->label->setAutoFillBackground(true);
    ui->label->setPalette(palette);
    ui->label->setText("Camera is Closed");

    ui->textEdit->setStyleSheet("background-color:black;");
    ui->textEdit->setTextColor("yellow");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_device_check_clicked()
{
    ui->textEdit->clear();
    comm_connect();

    //Test for USB connection
    QTimer::singleShot(2000, this, SLOT(comm_port_reset()));
}

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
    ui->device_check->setEnabled(false);

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

    measure_coount = 0;    

    ui->test_start->setEnabled(true);
    ui->device_check->setEnabled(true);

    measure_port_reset();
    comm_port_reset();

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
        connect(camera_timer, SIGNAL(timeout()), this, SLOT(update_camera()));
        camera_timer->start(20);
    }
}

void MainWindow::on_camera_stop_clicked()
{
    disconnect(camera_timer, SIGNAL(timeout()), this, SLOT(update_camera()));
    ui->camera_start->setEnabled(true);
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

//     measure_coount++;
}

void MainWindow::detect_on()
{
   relay->work(relay->fd_measure, CH1_DETECT, CH_ON);
}

void MainWindow::work_on()
{
    relay->work(relay->fd_measure, CH2_WORK, CH_ON);
}

void MainWindow::third_on()
{
    relay->work(relay->fd_measure, CH3_THIRD, CH_ON);
}

void MainWindow::detect_off()
{
   relay->work(relay->fd_measure, CH1_DETECT, CH_OFF);
}

void MainWindow::measure_port_reset()
{
    relay->port_reset(relay->fd_measure);
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
    for(quint8 channel=0x5; channel>0; channel--)
        relay->work(relay->fd_comm, channel ,CH_ON);
}

void MainWindow::comm_port_reset()
{
   relay->port_reset(relay->fd_comm);
}
