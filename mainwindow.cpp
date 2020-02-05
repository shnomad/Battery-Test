
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "relay_waveshare.h"
#include "relay_seed_ddl.h"
#include "relay_seed.h"
#include "usb_hid_comm.h"
#include "bgm_comm_protocol.h"
#include <stdlib.h>
#include <iostream>
#include <QTextEdit>
#include <QTextCursor>
#include <QThread>
#include <QDateTime>
#include <QTime>
#include <QApplication>
#include <QElapsedTimer>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), measure_relay(new relay_seed_ddl)
{
    ui->setupUi(this);

    mesure_time_check = new QElapsedTimer;
    meter_comm_usb = new usb_comm;
    meter_cmd = new bgm_comm_protocol;

//    status_cursor = new QTextCursor;

    measure_port_reset();
//  comm_port_reset();

    timer_sec = new QTimer(this);       //display current time
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

    ui->test_stop->setEnabled(false);
    ui->device_open->setEnabled(true);
    ui->device_close->setEnabled(false);

    ui->bluetooth_sel->setEnabled(true);
    ui->times->setEnabled(true);
    ui->sec->setEnabled(true);

    /*Test Interval*/
    ui->sec->setRange(0.0, 180.0);
    ui->sec->setSingleStep(1.0);
    ui->sec->setValue(0.0);

    /*Test capacity*/
    ui->times->setRange(0.0, 5000.0);
    ui->times->setSingleStep(100.0);
    ui->times->setValue(1000.0);

    /*Display Test status*/
//     QTextCursor textCursor = ui->status->textCursor();

//    QChar posi = textCursor.position();

//  textCursor.setPosition(0);
//  textCursor.insertText("version 0.0.8");
//  textCursor.setVerticalMovementX(3);
//    textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 3);
//    textCursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 0);

//      ui->status->setTextCursor(textCursor);
//      ui->status->setStyleSheet("background-color:white;");
//      ui->status->setTextColor("black");
//      ui->status->setFontPointSize(13);

#if 0
    ui->measure_count->setStyleSheet("background-color:black;");
    ui->measure_count->setTextColor("yellow");
    ui->measure_count->setFontPointSize(15);
    ui->measure_count->append((QString::number(0)));

    ui->current_step->setStyleSheet("background-color:black;");
    ui->current_step->setTextColor("yellow");
    ui->current_step->setFontPointSize(15);
    ui->current_step->append("stopped");

    ui->system_time->setStyleSheet("background-color:black;");
    ui->system_time->setTextColor("yellow");
    ui->system_time->setFontPointSize(15);

#endif

    QObject::connect(timer_sec, SIGNAL(timeout()), this, SLOT(UpdateTime()));
    timer_sec->start(1000);

//   ui->status->setText("V0.0.8");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_test_start_clicked()
{    

    if(ui->bluetooth_sel->isChecked())
    {
        bluetooth_time = 25000;         //add
    }

    //detect -> (5 sec) -> work_on -> (1 sec) -> third on -> (8 sec) -> detect off -> (10 sec)
    detect_on_timer->setInterval(detect_on_time);
    work_on_timer->setInterval(work_on_time);
    third_on_timer->setInterval(third_on_time);
    detect_off_timer->setInterval(detect_off_time);
    port_reset_timer->setInterval(port_reset_time + changed_interval + bluetooth_time);

    connect(this, SIGNAL(measure_start()), this, SLOT(measurement()));
    connect(this, SIGNAL(measure_check()), this, SLOT(measure_count_check()));
    connect(this, SIGNAL(measure_end()), this, SLOT(on_test_stop_clicked()));

    connect(detect_on_timer, SIGNAL(timeout()),SLOT(detect_on()));
    connect(work_on_timer, SIGNAL(timeout()),SLOT(work_on()));
    connect(third_on_timer, SIGNAL(timeout()),SLOT(third_on()));
    connect(detect_off_timer, SIGNAL(timeout()),SLOT(detect_off()));
    connect(port_reset_timer, SIGNAL(timeout()),SLOT(measure_port_reset()));

//    ui->status->clear();
//    ui->status->append("start");
    //ui->status->setTextCursor("start");
    ui->test_start->setEnabled(false);
    ui->device_open->setEnabled(false);
    ui->device_close->setEnabled(false);
    ui->test_stop->setEnabled(true);

    ui->bluetooth_sel->setEnabled(false);
    ui->times->setEnabled(false);
    ui->sec->setEnabled(false);

//    ui->status->setText("\n Test start :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));
    ui->test_start_time->setText("Test start time:" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));

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

//    if(measure_coount == 1000)
//         ui->status->setText("/r/r/r" + (QString::number(measure_coount)));

    measure_coount = 0;

    ui->test_start->setEnabled(true);
    ui->test_stop->setEnabled(false);

    ui->bluetooth_sel->setEnabled(true);
    ui->times->setEnabled(true);
    ui->sec->setEnabled(true);

    ui->device_open->setEnabled(true);

    measure_port_reset();
//  comm_port_reset();

//  ui->status->setText("\n\n action : stopped");
    ui->test_step->setText("Action : stopped");
}

void MainWindow::measurement()
{
     detect_on_timer->start();
     work_on_timer->start();
     third_on_timer->start();
     detect_off_timer->start();
     port_reset_timer->start();
     mesure_time_check->start();
}

void MainWindow::detect_on()
{
    cout<<"detect on : "<< mesure_time_check->elapsed()<<endl;
//    ui->status->setText("\n\n action : detect on");
      ui->test_step->setText("Action : detect on");
    measure_relay->measure_port_control(measure_relay->relay_channel::CH_1, DDL_CH_ON);
}

void MainWindow::work_on()
{
    cout<<"work on : "<< mesure_time_check->elapsed()<<"msec"<<endl;
//    ui->status->setText("\n\n action : work on");
     ui->test_step->setText("Action : work on");
    measure_relay->measure_port_control(measure_relay->relay_channel::CH_2, DDL_CH_ON);
}

void MainWindow::third_on()
{
    cout<<"third on : "<< mesure_time_check->elapsed()<<"msec"<<endl;
//    ui->status->setText("\n\n action : third on");
    ui->test_step->setText("Action : third on");
    measure_relay->measure_port_control(measure_relay->relay_channel::CH_3, DDL_CH_ON);
}

void MainWindow::detect_off()
{
    cout<<"detect off : "<< mesure_time_check->elapsed()<<"msec"<<endl;
//  ui->status->setText("\n\n action : detect off");
   ui->test_step->setText("Action : detect off");
    measure_relay->measure_port_control(measure_relay->relay_channel::CH_1, DDL_CH_OFF);
}

void MainWindow::measure_port_reset()
{
    cout<<"measure port reset : "<< mesure_time_check->elapsed() <<"msec"<<endl;
//  ui->status->setText("\n\n action : port reset");
    ui->test_step->setText("Action : port reset");
    measure_relay->measure_port_reset();

    emit measure_check();
}

void MainWindow:: measure_count_check()
{
      measure_coount++;

//    ui->status->setText("\n\n\n Test Count is " + (QString::number(measure_coount)));
      ui->test_count->setText("Test Count is " + (QString::number(measure_coount)));

    if(measure_coount < measure_capacity)
    {
        emit measure_start();
    }
    else if(measure_coount == measure_capacity)
    {
        emit measure_end();
    }
}

void MainWindow::comm_connect()
{
    comm_port_reset();
}

void MainWindow::comm_close()
{
    system("uhubctl -a off -p 2-5");
}

void MainWindow::comm_port_reset()
{
    system("uhubctl -a off -p 2-5");
    QThread::msleep(300);
    system("uhubctl -a on -p 2-5");
}

void MainWindow::on_quit_clicked()
{
    QProcess process;
    process.startDetached("sudo poweroff");
}

void MainWindow::on_device_open_clicked()
{
    ui->device_open->setEnabled(false);
    ui->device_close->setEnabled(true);

    comm_connect();

    meter_comm_usb->usb_hid_init();
    meter_comm_usb->usb_hid_device_open(meter_comm_usb->COMM_TYPE::STM32);
    meter_comm_usb->usb_hid_data_test();
}

void MainWindow::on_device_close_clicked()
{
    ui->device_open->setEnabled(true);
    ui->device_close->setEnabled(false);
    system("uhubctl -a off -p 2-5");
}

void MainWindow::on_times_valueChanged(const QString &arg1)
{
     measure_capacity = arg1.toInt(0,10);
}

void MainWindow::on_sec_valueChanged(const QString &arg1)
{
    changed_interval = arg1.toInt(0,10)*1000;
}

void MainWindow::UpdateTime()
{
//    ui->status->setText("System Time: " + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));
      ui->system_time->setText("System Time: " + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));
}
