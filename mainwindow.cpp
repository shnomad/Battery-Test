
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "seed_relay.h"
#include <QTextEdit>
#include <QThread>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), relay(new seed_relay)
{
    ui->setupUi(this);
  //QTextEdit *textEdit = new QTextEdit(this);
//  seed_relay *relay = new seed_relay;
    relay->port_reset();

    QPalette palette = ui->label->palette();
    palette.setColor(QPalette::WindowText, Qt::yellow);
    palette.setColor(QPalette::Window, Qt::black);

    camera_timer = new QTimer(this);
 // ui->test_start->setEnabled(false);

    ui->textEdit->setStyleSheet("background-color:black;");
    ui->textEdit->setTextColor("yellow");

//  ui->textEdit->append("First Checking the Device Conntection");

    ui->label->setAlignment(Qt::AlignCenter);
    ui->label->setAutoFillBackground(true);
    ui->label->setPalette(palette);
    ui->label->setText("Camera is Closed");

    connect(this, SIGNAL(measure_start()), this, SLOT(measurement()));
    connect(this, SIGNAL(measure_check()), this, SLOT(measure_count_check()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_device_check_clicked()
{
    ui->textEdit->clear();
}

void MainWindow::on_test_start_clicked()
{
    ui->textEdit->clear();
    ui->textEdit->append("measure start");

    ui->test_start->setEnabled(false);

    emit measure_start();
}

void MainWindow::on_test_stop_clicked()
{   
    disconnect(this, SIGNAL(measure_start()), this, SLOT(measurement()));
    disconnect(this, SIGNAL(measure_check()), this, SLOT(measure_count_check()));

    measure_coount = 0;

    ui->test_start->setEnabled(true);

    port_reset();

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
        ui->camera_stop->setEnabled(true);
        connect(camera_timer, SIGNAL(timeout()), this, SLOT(update_camera()));
        camera_timer->start(20);
    }
}

void MainWindow::on_camera_stop_clicked()
{
    disconnect(camera_timer, SIGNAL(timeout()), this, SLOT(update_camera()));
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
    //relay->work(CH1_DETECT, CH_ON);
    //delay 3 sec
    //QThread::msleep(3000);

    //relay->work(CH2_WORK, CH_ON);
    //delay 1 sec
    //QThread::msleep(1000);

    //relay->work(CH3_THIRD, CH_ON);
    //delay 8 sec
    //QThread::msleep(5000);

     QTimer::singleShot(0, this, SLOT(detect_on()));
       //5 Sec
     QTimer::singleShot(5000, this, SLOT(work_on()));
       //1 Sec
     QTimer::singleShot(6000, this, SLOT(third_on()));
       //9 Sec
     QTimer::singleShot(15000, this, SLOT(detect_off()));
       //14 Sec
     QTimer::singleShot(30000, this, SLOT(port_reset()));

     measure_coount++;
}

void MainWindow::detect_on()
{
   relay->work(CH1_DETECT, CH_ON);
}

void MainWindow::work_on()
{
    relay->work(CH2_WORK, CH_ON);
}

void MainWindow::third_on()
{
    relay->work(CH3_THIRD, CH_ON);
}

void MainWindow::detect_off()
{
   relay->work(CH1_DETECT, CH_OFF);
}

void MainWindow::port_reset()
{
    relay->port_reset();
    emit measure_check();
}

void MainWindow:: measure_count_check()
{
    ui->textEdit->append("Measurement count is  " + (QString::number(measure_coount)));

    if(measure_coount < 1000)
    {
        emit measure_start();
    }
    else if(measure_coount == 1000)
    {
        disconnect(this, SIGNAL(measure_start()), this, SLOT(measurement()));
        disconnect(this, SIGNAL(measure_stop()), this, SLOT(measure_count_check()));
    }
}


