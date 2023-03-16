#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "relay_seed_ddl.h"
#include "bgm_comm_protocol.h"
#include "settings.h"
#include "setting_flagname_definition.h"
#include "builddatetime.h"
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <QTextEdit>
#include <QTextCursor>
#include <QThread>
#include <QDateTime>
#include <QTime>
#include <QApplication>
#include <QProcess>
#include <QtNetwork/QNetworkInterface>
#include "loggingcategories.h"
#include "tcpsocketrw.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
      ui->setupUi(this);

      qRegisterMetaType<measurement_param>();

      timer_sec = new QTimer(this);                   //display system time

      ui_create_measurement();

      ui_init_measurement();

      ui_system_info_setup();

      comm_cmd = new sys_cmd_resp;

      connect(ui->meter_type_ch1, SIGNAL(currentIndexChanged(int)), this, SLOT(currentMeterIndexChanged(int)));
      connect(ui->meter_type_ch2, SIGNAL(currentIndexChanged(int)), this, SLOT(currentMeterIndexChanged(int)));
      connect(ui->meter_type_ch3, SIGNAL(currentIndexChanged(int)), this, SLOT(currentMeterIndexChanged(int)));
      connect(ui->meter_type_ch4, SIGNAL(currentIndexChanged(int)), this, SLOT(currentMeterIndexChanged(int)));
      connect(ui->meter_type_ch5, SIGNAL(currentIndexChanged(int)), this, SLOT(currentMeterIndexChanged(int)));

      /*BGMS Communication response message*/
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_PORT_OPEN_SUCCESS), "UART/USB Port Open Success");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_PORT_CLOSE_SUCCESS), "UART/USB Port Close Success");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_BGMS_CHECK_SUCCESS), "BGMS Check Success");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_READ_SERIAL_SUCCESS),"BGMS Serial Number : ");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_GET_TIME_SUCCESS), "BGMS Current Time : ");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_SET_TIME_SUCCESS), "BGMS Time synchronized");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_MEM_DELETE_SUCCESS), "BGMS Stored measured results deleted");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_GET_STORED_VALUE_COUNT_SUCCESS), "BGMS Stored measured results count :");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_DOWNLOAD_SUCCESS), "BGMS Stored measured results download complete");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_PORT_OPEN_FAIL), "UART/USB Port Open Fail");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_PORT_CLOSE_FAIL), "UART/USB Port Close Fail");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_BGMS_CHECK_FAIL), "BGMS Check Failed");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_READ_SERIAL_FAIL), "Read BGMS Serial Number Failed");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_SET_TIME_FAIL), "BGMS Time synchronized Failed");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_GET_STORED_VALUE_COUNT_FAIL), "Get BGMS Stored measured value count Failed");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_DOWNLOAD_FAIL), "BGMS Stored measured value download failed");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_MEM_DELETE_FAIL), "BGMS Stored measured value delet failed");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_BGMS_RESP_FAIL), "BGMS Response failed");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::RESP_COMM_UNKNOWN), "BGMS Response unknown error");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::VERSION_01), "protocol version 1 used BGMS connected");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::VERSION_02), "protocol version 2 used BGMS connected");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::VERSION_03), "protocol version 3 used BGMS connected");
      comm_response_msg.insert(static_cast<quint8>(sys_cmd_resp::VERSION_UNKNOWN), "unknown protocol version used BGMS connected");

      /*console command interface */
      m_console_command = new CLI_monitor;

      /*Check USB Mass Stroage plug in*/
      usb_mass_detect = new udev_monitor_usb;

      QObject::connect(usb_mass_detect, &udev_monitor_usb::sig_resp_to_main, [=](sys_cmd_resp *resp)
      {
          switch(resp->m_comm_resp)
          {

              case sys_cmd_resp::RESP_USB_MASS_STORAGE_LIST:

                Log()<<resp->mass_storage_device_info;

                if(resp->mass_storage_device_info.count() == 0)
                {
                        ui->audo_download_ch1->setEnabled(false);
                        ui->audo_download_ch2->setEnabled(false);
                        ui->audo_download_ch3->setEnabled(false);
                        ui->audo_download_ch4->setEnabled(false);
                        ui->audo_download_ch5->setEnabled(false);
                }
                else
                {

                    /*update item list detected USB disk*/
                    QMap<QString, QString>::ConstIterator usb_mass_stroage_connected_list = resp->mass_storage_device_info.constBegin();
                    while (usb_mass_stroage_connected_list!=  resp->mass_storage_device_info.constEnd())
                    {
                        dev_path = usb_mass_stroage_connected_list.key() + "1";
                        usb_mass_stroage_connected_list++;
                    }

                     ui->audo_download_ch1->setEnabled(true);
                }

                break;
          }
      });

      usb_mass_detect->update();
      usb_mass_detect->start();

      system_init_done = true;
}

void MainWindow::ui_init_measurement()
{

    /*
    * CHANNEL_1 UI SETTINGS
    */

    /*Play/Pause/Stop Button*/
/*  ui->test_start_ch1->setIcon(QIcon(":/images/play.png"));
    ui->test_stop_ch1->setIcon(QIcon(":/images/stop.png"));    
    ui->test_pause_ch1->setIcon(QIcon(":/images/pause.png"));
*/

    QPixmap play_icon("/home/pi/Battery_LifeTime_Test/bin/images/play.png"), \
            stop_icon("/home/pi/Battery_LifeTime_Test/bin/images/stop.png"), \
            pause_icon("/home/pi/Battery_LifeTime_Test/bin/images/pause.png");

    ui->test_start_ch1->setIcon(QIcon(play_icon));
    ui->test_stop_ch1->setIcon(QIcon(stop_icon));
    ui->test_pause_ch1->setIcon(QIcon(pause_icon));

    ui->test_start_ch1->setIconSize(QSize(95,95));
    ui->test_stop_ch1->setIconSize(QSize(95,95));
    ui->test_pause_ch1->setIconSize(QSize(95,95));

    ui->test_stop_ch1->setEnabled(false);
    ui->test_pause_ch1->setEnabled(false);
    ui->device_open_ch1->setEnabled(true);
    ui->device_close_ch1->setEnabled(false);

    //Meter type select

    ui->meter_type_ch1->addItems(meter_types);
    ui->meter_type_ch1->setCurrentIndex(0);

    /*Measurement Condition*/

    ui->meter_type_ch1->setEnabled(true);
    ui->measure_count_ch1->setEnabled(true);
    ui->sec_startdelay_ch1->setEnabled(true);
    ui->sec_detoffdelay_ch1->setEnabled(true);
    ui->sec_interval_ch1->setEnabled(true);


    /*Setting the Test Count*/
    ui->measure_count_ch1->setRange(0.0, 5000.0);
    ui->measure_count_ch1->setSingleStep(100.0);

    /*Measuremet Start Delay*/
    ui->sec_startdelay_ch1->setRange(0.0, 180.0);
    ui->sec_startdelay_ch1->setSingleStep(1.0);
//  ui->sec_startdelay_ch1->setValue((int)(m_test_param->work_on_time/1000));

    /*Detect Off Delay*/
    ui->sec_detoffdelay_ch1->setRange(0.0, 180.0);
    ui->sec_detoffdelay_ch1->setSingleStep(1.0);

    /*Test Interval*/
    ui->sec_interval_ch1->setRange(0.0, 300.0);
    ui->sec_interval_ch1->setSingleStep(1.0);

    m_test_param_tmp.channel = measurement_param::CH_1;
    m_test_param_tmp.type = measurement_param::GLUCOSE_BASIC;

    currentMeterIndexChanged(m_test_param_tmp.type);

    m_control->delay_mSec(50);

    /*Init Action Status*/
    ui->test_step_ch1->setText("Current Action :  Stopeed");

    /*Test capacity*/
 // target_measure_count_rest = 1000;

    /*USB Interface select*/
    ui->micro_usb_ch1->setEnabled(true);
    ui->micro_usb_ch1->setChecked(true);

    ui->phone_jack_ch1->setEnabled(true);
    ui->micro_usb_ch1->setChecked(false);

    ui->time_sync_ch1->setEnabled(false);
    ui->mem_delete_ch1->setEnabled(false);
    ui->download_ch1->setEnabled(false);

    /*
    * CHANNEL_2 UI SETTINGS
    */

    /*Play/Pause/Stop Button*/
//    ui->test_start_ch2->setIcon(QIcon(":/images/play.png"));
//    ui->test_stop_ch2->setIcon(QIcon(":/images/stop.png"));
//    ui->test_pause_ch2->setIcon(QIcon(":/images/pause.png"));

    ui->test_start_ch2->setIcon(QIcon(play_icon));
    ui->test_stop_ch2->setIcon(QIcon(stop_icon));
    ui->test_pause_ch2->setIcon(QIcon(pause_icon));

    ui->test_start_ch2->setIconSize(QSize(95,95));
    ui->test_stop_ch2->setIconSize(QSize(95,95));
    ui->test_pause_ch2->setIconSize(QSize(95,95));

    ui->test_stop_ch2->setEnabled(false);
    ui->test_pause_ch2->setEnabled(false);
    ui->device_open_ch2->setEnabled(true);
    ui->device_close_ch2->setEnabled(false);

    //Meter type select

    ui->meter_type_ch2->addItems(meter_types);
    ui->meter_type_ch2->setCurrentIndex(0);

    /*Measurement Condition*/
    ui->meter_type_ch2->setEnabled(true);
    ui->measure_count_ch2->setEnabled(true);
    ui->sec_startdelay_ch2->setEnabled(true);
    ui->sec_detoffdelay_ch2->setEnabled(true);
    ui->sec_interval_ch2->setEnabled(true);

    /*Setting the Test Count*/
    ui->measure_count_ch2->setEnabled(true);
    ui->measure_count_ch2->setRange(0.0, 5000.0);
    ui->measure_count_ch2->setSingleStep(100.0);

    /*Measuremet Start Delay*/
    ui->sec_startdelay_ch2->setRange(0.0, 180.0);
    ui->sec_startdelay_ch2->setSingleStep(1.0);
//  ui->sec_startdelay_ch1->setValue((int)(m_test_param->work_on_time/1000));

    /*Detect Off Delay*/
    ui->sec_detoffdelay_ch2->setRange(0.0, 180.0);
    ui->sec_detoffdelay_ch2->setSingleStep(1.0);

    /*Test Interval*/
    ui->sec_interval_ch2->setRange(0.0, 300.0);
    ui->sec_interval_ch2->setSingleStep(1.0);

    m_test_param_tmp.channel = measurement_param::CH_2;
    m_test_param_tmp.type = measurement_param::GLUCOSE_BASIC;

    currentMeterIndexChanged(m_test_param_tmp.type);

    m_control->delay_mSec(50);

    /*Init Action Status*/
    ui->test_step_ch2->setText("Current Action :  Stopeed");

    /*USB Interface select*/
    ui->micro_usb_ch2->setEnabled(true);
    ui->micro_usb_ch2->setChecked(true);

    ui->phone_jack_ch2->setEnabled(true);
    ui->micro_usb_ch2->setChecked(false);

    ui->time_sync_ch2->setEnabled(false);
    ui->mem_delete_ch2->setEnabled(false);
    ui->download_ch2->setEnabled(false);

    /*
    * CHANNEL_3 UI SETTINGS
    */

    /*Play/Pause/Stop Button*/
/*
 *  ui->test_start_ch3->setIcon(QIcon(":/images/play.png"));
    ui->test_stop_ch3->setIcon(QIcon(":/images/stop.png"));    
    ui->test_pause_ch3->setIcon(QIcon(":/images/pause.png"));
*/
    ui->test_start_ch3->setIcon(QIcon(play_icon));
    ui->test_stop_ch3->setIcon(QIcon(stop_icon));
    ui->test_pause_ch3->setIcon(QIcon(pause_icon));

    ui->test_start_ch3->setIconSize(QSize(95,95));
    ui->test_stop_ch3->setIconSize(QSize(95,95));
    ui->test_pause_ch3->setIconSize(QSize(95,95));

    ui->test_stop_ch3->setEnabled(false);
    ui->test_pause_ch3->setEnabled(false);
    ui->device_open_ch3->setEnabled(true);
    ui->device_close_ch3->setEnabled(false);

    //Meter type select
    ui->meter_type_ch3->addItems(meter_types);
    ui->meter_type_ch3->setCurrentIndex(0);

    /*Measurement Condition*/
    ui->meter_type_ch3->setEnabled(true);
    ui->measure_count_ch3->setEnabled(true);
    ui->sec_startdelay_ch3->setEnabled(true);
    ui->sec_detoffdelay_ch3->setEnabled(true);
    ui->sec_interval_ch3->setEnabled(true);

    /*Setting the Test Count*/
    ui->measure_count_ch3->setEnabled(true);
    ui->measure_count_ch3->setRange(0.0, 5000.0);
    ui->measure_count_ch3->setSingleStep(100.0);

    /*Measuremet Start Delay*/
    ui->sec_startdelay_ch3->setRange(0.0, 180.0);
    ui->sec_startdelay_ch3->setSingleStep(1.0);
//  ui->sec_startdelay_ch1->setValue((int)(m_test_param->work_on_time/1000));

    /*Detect Off Delay*/
    ui->sec_detoffdelay_ch3->setRange(0.0, 180.0);
    ui->sec_detoffdelay_ch3->setSingleStep(1.0);

    /*Test Interval*/
    ui->sec_interval_ch3->setRange(0.0, 300.0);
    ui->sec_interval_ch3->setSingleStep(1.0);

    m_test_param_tmp.channel = measurement_param::CH_3;
    m_test_param_tmp.type = measurement_param::GLUCOSE_BASIC;

    currentMeterIndexChanged(m_test_param_tmp.type);

    m_control->delay_mSec(50);

    /*Init Action Status*/
    ui->test_step_ch3->setText("Current Action :  Stopeed");

    /*USB Interface select*/
    ui->micro_usb_ch3->setEnabled(true);
    ui->micro_usb_ch3->setChecked(true);

    ui->phone_jack_ch3->setEnabled(true);
    ui->micro_usb_ch3->setChecked(false);

    ui->time_sync_ch3->setEnabled(false);
    ui->mem_delete_ch3->setEnabled(false);
    ui->download_ch3->setEnabled(false);

    /*
    * CHANNEL_4 UI SETTINGS
    */

    /*Play/Pause/Stop Button*/
    /*
    ui->test_start_ch4->setIcon(QIcon(":/images/play.png"));
    ui->test_stop_ch4->setIcon(QIcon(":/images/stop.png"));    
    ui->test_pause_ch4->setIcon(QIcon(":/images/pause.png"));
    */

    ui->test_start_ch4->setIcon(QIcon(play_icon));
    ui->test_stop_ch4->setIcon(QIcon(stop_icon));
    ui->test_pause_ch4->setIcon(QIcon(pause_icon));

    ui->test_start_ch4->setIconSize(QSize(95,95));
    ui->test_stop_ch4->setIconSize(QSize(95,95));
    ui->test_pause_ch4->setIconSize(QSize(95,95));

    ui->test_stop_ch4->setEnabled(false);
    ui->test_pause_ch4->setEnabled(false);
    ui->device_open_ch4->setEnabled(true);
    ui->device_close_ch4->setEnabled(false);

    //Meter type select
    ui->meter_type_ch4->addItems(meter_types);
    ui->meter_type_ch4->setCurrentIndex(0);

    /*Measurement Condition*/
    ui->meter_type_ch4->setEnabled(true);
    ui->measure_count_ch4->setEnabled(true);
    ui->sec_startdelay_ch4->setEnabled(true);
    ui->sec_detoffdelay_ch4->setEnabled(true);
    ui->sec_interval_ch4->setEnabled(true);

    /*Setting the Test Count*/
    ui->measure_count_ch4->setEnabled(true);
    ui->measure_count_ch4->setRange(0.0, 5000.0);
    ui->measure_count_ch4->setSingleStep(100.0);

    /*Measuremet Start Delay*/
    ui->sec_startdelay_ch4->setRange(0.0, 180.0);
    ui->sec_startdelay_ch4->setSingleStep(1.0);
//  ui->sec_startdelay_ch1->setValue((int)(m_test_param->work_on_time/1000));

    /*Detect Off Delay*/
    ui->sec_detoffdelay_ch4->setRange(0.0, 180.0);
    ui->sec_detoffdelay_ch4->setSingleStep(1.0);

    /*Test Interval*/
    ui->sec_interval_ch4->setRange(0.0, 300.0);
    ui->sec_interval_ch4->setSingleStep(1.0);

    m_test_param_tmp.channel = measurement_param::CH_4;
    m_test_param_tmp.type = measurement_param::GLUCOSE_BASIC;

    currentMeterIndexChanged(m_test_param_tmp.type);

    m_control->delay_mSec(50);

    /*Init Action Status*/
    ui->test_step_ch4->setText("Current Action :  Stopeed");

    /*USB Interface select*/
    ui->micro_usb_ch4->setEnabled(true);
    ui->micro_usb_ch4->setChecked(true);

    ui->phone_jack_ch4->setEnabled(true);
    ui->micro_usb_ch4->setChecked(false);

    ui->time_sync_ch4->setEnabled(false);
    ui->mem_delete_ch4->setEnabled(false);
    ui->download_ch4->setEnabled(false);

    /*
    * CHANNEL_5 UI SETTINGS
    */

    /*Play/Pause/Stop Button*/
 /*
    ui->test_start_ch5->setIcon(QIcon(":/images/play.png"));    
    ui->test_stop_ch5->setIcon(QIcon(":/images/stop.png"));    
    ui->test_pause_ch5->setIcon(QIcon(":/images/pause.png"));
 */
    ui->test_start_ch5->setIcon(QIcon(play_icon));
    ui->test_stop_ch5->setIcon(QIcon(stop_icon));
    ui->test_pause_ch5->setIcon(QIcon(pause_icon));

    ui->test_start_ch5->setIconSize(QSize(95,95));
    ui->test_stop_ch5->setIconSize(QSize(95,95));
    ui->test_pause_ch5->setIconSize(QSize(95,95));

    ui->test_stop_ch5->setEnabled(false);
    ui->test_pause_ch5->setEnabled(false);
    ui->device_open_ch5->setEnabled(true);
    ui->device_close_ch5->setEnabled(false);

    //Meter type select
    ui->meter_type_ch5->addItems(meter_types);
    ui->meter_type_ch5->setCurrentIndex(0);

    /*Measurement Condition*/
    ui->meter_type_ch5->setEnabled(true);
    ui->measure_count_ch5->setEnabled(true);
    ui->sec_startdelay_ch5->setEnabled(true);
    ui->sec_detoffdelay_ch5->setEnabled(true);
    ui->sec_interval_ch5->setEnabled(true);

    /*Setting the Test Count*/
    ui->measure_count_ch5->setEnabled(true);
    ui->measure_count_ch5->setRange(0.0, 5000.0);
    ui->measure_count_ch5->setSingleStep(100.0);

    /*Measuremet Start Delay*/
    ui->sec_startdelay_ch5->setRange(0.0, 180.0);
    ui->sec_startdelay_ch5->setSingleStep(1.0);
//  ui->sec_startdelay_ch1->setValue((int)(m_test_param->work_on_time/1000));

    /*Detect Off Delay*/
    ui->sec_detoffdelay_ch5->setRange(0.0, 180.0);
    ui->sec_detoffdelay_ch5->setSingleStep(1.0);

    /*Test Interval*/
    ui->sec_interval_ch5->setRange(0.0, 300.0);
    ui->sec_interval_ch5->setSingleStep(1.0);

    m_test_param_tmp.channel = measurement_param::CH_5;
    m_test_param_tmp.type = measurement_param::GLUCOSE_BASIC;

    currentMeterIndexChanged(m_test_param_tmp.type);

    m_control->delay_mSec(50);

    /*Init Action Status*/
    ui->test_step_ch5->setText("Current Action :  Stopeed");

    /*USB Interface select*/
    ui->micro_usb_ch5->setEnabled(true);
    ui->micro_usb_ch5->setChecked(true);

    ui->phone_jack_ch5->setEnabled(true);
    ui->micro_usb_ch5->setChecked(false);

    ui->time_sync_ch5->setEnabled(false);
    ui->mem_delete_ch5->setEnabled(false);
    ui->download_ch5->setEnabled(false);

}

void MainWindow::ui_create_measurement()
{

    m_control = new control;

    /*Channel 1*/
    connect(this, SIGNAL(measure_setup_ch1(measurement_param)), m_control->m_ch[0], SLOT(setup(measurement_param)));
    connect(this, SIGNAL(measure_start_ch1()), m_control->m_ch[0], SLOT(start()));
    connect(this, SIGNAL(measure_stop_ch1()), m_control->m_ch[0], SLOT(stop()));
    connect(this, SIGNAL(measure_pause_ch1()), m_control->m_ch[0], SLOT(pause()));
    connect(m_control->m_ch[0], SIGNAL(update_test_count(int)), this, SLOT(ui_test_count_ch1(int)));
    connect(m_control->m_ch[0], SIGNAL(update_action(QString)), this, SLOT(ui_action_status_ch1(QString)));
    connect(m_control->m_ch[0], SIGNAL(update_interval_time(int)), this, SLOT(ui_interval_time_update_ch1(int)));
    connect(m_control->m_ch[0], SIGNAL(update_dmm_status(QString)), this, SLOT(dmm_working_status(QString)));

    /*Channel 2*/
    connect(this, SIGNAL(measure_setup_ch2(measurement_param)), m_control->m_ch[1], SLOT(setup(measurement_param)));
    connect(this, SIGNAL(measure_start_ch2()), m_control->m_ch[1], SLOT(start()));
    connect(this, SIGNAL(measure_stop_ch2()), m_control->m_ch[1], SLOT(stop()));
    connect(this, SIGNAL(measure_pause_ch2()), m_control->m_ch[1], SLOT(pause()));
    connect(m_control->m_ch[1], SIGNAL(update_test_count(int)), this, SLOT(ui_test_count_ch2(int)));
    connect(m_control->m_ch[1], SIGNAL(update_action(QString)), this, SLOT(ui_action_status_ch2(QString)));
    connect(m_control->m_ch[1], SIGNAL(update_interval_time(int)), this, SLOT(ui_interval_time_update_ch2(int)));

    /*Channel 3*/
    connect(this, SIGNAL(measure_setup_ch3(measurement_param)), m_control->m_ch[2], SLOT(setup(measurement_param)));
    connect(this, SIGNAL(measure_start_ch3()), m_control->m_ch[2], SLOT(start()));
    connect(this, SIGNAL(measure_stop_ch3()), m_control->m_ch[2], SLOT(stop()));
    connect(this, SIGNAL(measure_pause_ch3()), m_control->m_ch[2], SLOT(pause()));
    connect(m_control->m_ch[2], SIGNAL(update_test_count(int)), this, SLOT(ui_test_count_ch3(int)));
    connect(m_control->m_ch[2], SIGNAL(update_action(QString)), this, SLOT(ui_action_status_ch3(QString)));
    connect(m_control->m_ch[2], SIGNAL(update_interval_time(int)), this, SLOT(ui_interval_time_update_ch3(int)));

    /*Channel 4*/
    connect(this, SIGNAL(measure_setup_ch4(measurement_param)), m_control->m_ch[3], SLOT(setup(measurement_param)));
    connect(this, SIGNAL(measure_start_ch4()), m_control->m_ch[3], SLOT(start()));
    connect(this, SIGNAL(measure_stop_ch4()), m_control->m_ch[3], SLOT(stop()));
    connect(this, SIGNAL(measure_pause_ch4()), m_control->m_ch[3], SLOT(pause()));
    connect(m_control->m_ch[3], SIGNAL(update_test_count(int)), this, SLOT(ui_test_count_ch4(int)));
    connect(m_control->m_ch[3], SIGNAL(update_action(QString)), this, SLOT(ui_action_status_ch4(QString)));
    connect(m_control->m_ch[3], SIGNAL(update_interval_time(int)), this, SLOT(ui_interval_time_update_ch4(int)));   

    /*Channel 5*/
    connect(this, SIGNAL(measure_setup_ch5(measurement_param)), m_control->m_ch[4], SLOT(setup(measurement_param)));
    connect(this, SIGNAL(measure_start_ch5()), m_control->m_ch[4], SLOT(start()));
    connect(this, SIGNAL(measure_stop_ch5()), m_control->m_ch[4], SLOT(stop()));
    connect(this, SIGNAL(measure_pause_ch5()), m_control->m_ch[4], SLOT(pause()));
    connect(m_control->m_ch[4], SIGNAL(update_test_count(int)), this, SLOT(ui_test_count_ch5(int)));
    connect(m_control->m_ch[4], SIGNAL(update_action(QString)), this, SLOT(ui_action_status_ch5(QString)));
    connect(m_control->m_ch[4], SIGNAL(update_interval_time(int)), this, SLOT(ui_interval_time_update_ch5(int)));    

}

void MainWindow::ui_set_measurement_start_ch1()
{
    ui->test_start_ch1->setEnabled(false);
    ui->device_open_ch1->setEnabled(false);
    ui->device_close_ch1->setEnabled(false);
    ui->test_stop_ch1->setEnabled(true);
    ui->test_pause_ch1->setEnabled(true);

    //Test option disable
    ui->meter_type_ch1->setEnabled(false);
    ui->measure_count_ch1->setEnabled(false);
    ui->sec_startdelay_ch1->setEnabled(false);
    ui->sec_detoffdelay_ch1->setEnabled(false);
    ui->sec_interval_ch1->setEnabled(false);
    ui->audo_download_ch1->setEnabled(false);
    ui->all_channel_select->setEnabled(false);    

    //Comm Interface option
    ui->micro_usb_ch1->setEnabled(false);
    ui->phone_jack_ch1->setEnabled(false);

    ui_test_count_ch1(0);

    //ui->test_start_time_ch1->setText("Test start :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));
    ui->meter_info_ch1->setText("Test Start :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));

    /*button color change*/
    ui->test_pause_ch1->setStyleSheet("default");
    ui->test_start_ch1->setStyleSheet("background-color:rgb(244,0,0);border-style:insert");

}

void MainWindow::ui_set_measurement_stop_ch1()
{
    ui->test_start_ch1->setEnabled(true);
    ui->test_stop_ch1->setEnabled(false);
    ui->test_pause_ch1->setEnabled(false);

    ui->meter_type_ch1->setEnabled(true);
    ui->measure_count_ch1->setEnabled(true);
    ui->sec_startdelay_ch1->setEnabled(true);
    ui->sec_detoffdelay_ch1->setEnabled(true);
    ui->sec_interval_ch1->setEnabled(true);
    ui->audo_download_ch1->setEnabled(true);
    ui->all_channel_select->setEnabled(true);
//   ui->dmm_capture->setEnabled(true);

    ui->device_open_ch1->setEnabled(true);
    ui->micro_usb_ch1->setEnabled(true);
    ui->phone_jack_ch1->setEnabled(true);

//  ui->test_step_ch1->setText("Action : stopped");
    ui->meter_info_ch1->append("Test stopped :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));

    ui->test_start_ch1->setStyleSheet("default");
    ui->test_pause_ch1->setStyleSheet("default");
}

void MainWindow::ui_set_measurement_pause_ch1()
{
    ui->test_start_ch1->setEnabled(true);
    ui->test_stop_ch1->setEnabled(true);
    ui->test_pause_ch1->setEnabled(false);

    ui->device_open_ch1->setEnabled(true);
    ui->micro_usb_ch1->setEnabled(true);
    ui->phone_jack_ch1->setEnabled(true);

    ui->test_start_ch1->setStyleSheet("default");
    ui->test_pause_ch1->setStyleSheet("background-color:rgb(244,0,0);border-style:insert");
}


void MainWindow::ui_set_measurement_start_ch2()
{
    ui->test_start_ch2->setEnabled(false);
    ui->device_open_ch2->setEnabled(false);
    ui->device_close_ch2->setEnabled(false);
    ui->test_stop_ch2->setEnabled(true);
    ui->test_pause_ch2->setEnabled(true);

    //Test option disable
    ui->meter_type_ch2->setEnabled(false);
    ui->measure_count_ch2->setEnabled(false);
    ui->sec_startdelay_ch2->setEnabled(false);
    ui->sec_detoffdelay_ch2->setEnabled(false);
    ui->sec_interval_ch2->setEnabled(false);
    ui->audo_download_ch2->setEnabled(false);

    //Comm Interface option
    ui->micro_usb_ch2->setEnabled(false);
    ui->phone_jack_ch2->setEnabled(false);

    ui_test_count_ch2(0);

    ui->meter_info_ch2->setText("Test Start :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));

    ui->test_pause_ch2->setStyleSheet("default");
    ui->test_start_ch2->setStyleSheet("background-color:rgb(244,0,0);border-style:insert");
}

void MainWindow::ui_set_measurement_stop_ch2()
{
    ui->test_start_ch2->setEnabled(true);
    ui->test_stop_ch2->setEnabled(false);
    ui->test_pause_ch2->setEnabled(false);

    ui->meter_type_ch2->setEnabled(true);
    ui->measure_count_ch2->setEnabled(true);
    ui->sec_startdelay_ch2->setEnabled(true);
    ui->sec_detoffdelay_ch2->setEnabled(true);
    ui->sec_interval_ch2->setEnabled(true);
    ui->audo_download_ch2->setEnabled(false);

    ui->device_open_ch2->setEnabled(true);
    ui->micro_usb_ch2->setEnabled(true);
    ui->phone_jack_ch2->setEnabled(true);

    ui->meter_info_ch2->append("Test stopped :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));

    ui->test_start_ch2->setStyleSheet("default");
    ui->test_pause_ch2->setStyleSheet("default");
}

void MainWindow::ui_set_measurement_pause_ch2()
{
    ui->test_start_ch2->setEnabled(true);
    ui->test_stop_ch2->setEnabled(true);
    ui->test_pause_ch2->setEnabled(false);

    ui->device_open_ch2->setEnabled(true);
    ui->micro_usb_ch2->setEnabled(true);
    ui->phone_jack_ch2->setEnabled(true);

    ui->test_start_ch2->setStyleSheet("default");
    ui->test_pause_ch2->setStyleSheet("background-color:rgb(244,0,0);border-style:insert");
}

void MainWindow::ui_set_measurement_start_ch3()
{
    ui->test_start_ch3->setEnabled(false);
    ui->device_open_ch3->setEnabled(false);
    ui->device_close_ch3->setEnabled(false);
    ui->test_stop_ch3->setEnabled(true);
    ui->test_pause_ch3->setEnabled(true);

    //Test option disable
    ui->meter_type_ch3->setEnabled(false);
    ui->measure_count_ch3->setEnabled(false);
    ui->sec_startdelay_ch3->setEnabled(false);
    ui->sec_detoffdelay_ch3->setEnabled(false);
    ui->sec_interval_ch3->setEnabled(false);
    ui->audo_download_ch3->setEnabled(false);

    //Comm Interface option
    ui->micro_usb_ch3->setEnabled(false);
    ui->phone_jack_ch3->setEnabled(false);

    ui_test_count_ch3(0);

    //ui->test_start_time_ch1->setText("Test start :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));
    ui->meter_info_ch3->setText("Test Start :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));

    /*button color change*/
    ui->test_pause_ch3->setStyleSheet("default");
    ui->test_start_ch3->setStyleSheet("background-color:rgb(244,0,0);border-style:insert");

}

void MainWindow::ui_set_measurement_stop_ch3()
{
    ui->test_start_ch3->setEnabled(true);
    ui->test_stop_ch3->setEnabled(false);
    ui->test_pause_ch3->setEnabled(false);

    ui->meter_type_ch3->setEnabled(true);
    ui->measure_count_ch3->setEnabled(true);
    ui->sec_startdelay_ch3->setEnabled(true);
    ui->sec_detoffdelay_ch3->setEnabled(true);
    ui->sec_interval_ch3->setEnabled(true);
    ui->audo_download_ch3->setEnabled(false);

    ui->device_open_ch3->setEnabled(true);
    ui->micro_usb_ch3->setEnabled(true);
    ui->phone_jack_ch3->setEnabled(true);

//  ui->test_step_ch1->setText("Action : stopped");
    ui->meter_info_ch3->append("Test stopped :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));

    ui->test_start_ch3->setStyleSheet("default");
    ui->test_pause_ch3->setStyleSheet("default");
}

void MainWindow::ui_set_measurement_pause_ch3()
{
    ui->test_start_ch3->setEnabled(true);
    ui->test_stop_ch3->setEnabled(true);
    ui->test_pause_ch3->setEnabled(false);

    ui->device_open_ch3->setEnabled(true);
    ui->micro_usb_ch3->setEnabled(true);
    ui->phone_jack_ch3->setEnabled(true);

    ui->test_start_ch3->setStyleSheet("default");
    ui->test_pause_ch3->setStyleSheet("background-color:rgb(244,0,0);border-style:insert");
}

void MainWindow::ui_set_measurement_start_ch4()
{
    ui->test_start_ch4->setEnabled(false);
    ui->device_open_ch4->setEnabled(false);
    ui->device_close_ch4->setEnabled(false);
    ui->test_stop_ch4->setEnabled(true);
    ui->test_pause_ch4->setEnabled(true);

    //Test option disable
    ui->meter_type_ch4->setEnabled(false);
    ui->measure_count_ch4->setEnabled(false);
    ui->sec_startdelay_ch4->setEnabled(false);
    ui->sec_detoffdelay_ch4->setEnabled(false);
    ui->sec_interval_ch4->setEnabled(false);
    ui->audo_download_ch4->setEnabled(false);

    //Comm Interface option
    ui->micro_usb_ch4->setEnabled(false);
    ui->phone_jack_ch4->setEnabled(false);

    ui_test_count_ch4(0);

    //ui->test_start_time_ch1->setText("Test start :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));
    ui->meter_info_ch4->setText("Test Start :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));

    /*button color change*/
    ui->test_pause_ch4->setStyleSheet("default");
    ui->test_start_ch4->setStyleSheet("background-color:rgb(244,0,0);border-style:insert");

}

void MainWindow::ui_set_measurement_stop_ch4()
{
    ui->test_start_ch4->setEnabled(true);
    ui->test_stop_ch4->setEnabled(false);
    ui->test_pause_ch4->setEnabled(false);

    ui->meter_type_ch4->setEnabled(true);
    ui->measure_count_ch4->setEnabled(true);
    ui->sec_startdelay_ch4->setEnabled(true);
    ui->sec_detoffdelay_ch4->setEnabled(true);
    ui->sec_interval_ch4->setEnabled(true);
    ui->audo_download_ch4->setEnabled(false);

    ui->device_open_ch4->setEnabled(true);
    ui->micro_usb_ch4->setEnabled(true);
    ui->phone_jack_ch4->setEnabled(true);

//  ui->test_step_ch1->setText("Action : stopped");
    ui->meter_info_ch4->append("Test stopped :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));

    ui->test_start_ch4->setStyleSheet("default");
    ui->test_pause_ch4->setStyleSheet("default");
}

void MainWindow::ui_set_measurement_pause_ch4()
{
    ui->test_start_ch4->setEnabled(true);
    ui->test_stop_ch4->setEnabled(true);
    ui->test_pause_ch4->setEnabled(false);

    ui->device_open_ch4->setEnabled(true);
    ui->micro_usb_ch4->setEnabled(true);
    ui->phone_jack_ch4->setEnabled(true);

    ui->test_start_ch4->setStyleSheet("default");
    ui->test_pause_ch4->setStyleSheet("background-color:rgb(244,0,0);border-style:insert");
}

void MainWindow::ui_set_measurement_start_ch5()
{
    ui->test_start_ch5->setEnabled(false);
    ui->device_open_ch5->setEnabled(false);
    ui->device_close_ch5->setEnabled(false);
    ui->test_stop_ch5->setEnabled(true);
    ui->test_pause_ch5->setEnabled(true);

    //Test option disable
    ui->meter_type_ch5->setEnabled(false);
    ui->measure_count_ch5->setEnabled(false);
    ui->sec_startdelay_ch5->setEnabled(false);
    ui->sec_detoffdelay_ch5->setEnabled(false);
    ui->sec_interval_ch5->setEnabled(false);
    ui->audo_download_ch5->setEnabled(false);

    //Comm Interface option
    ui->micro_usb_ch5->setEnabled(false);
    ui->phone_jack_ch5->setEnabled(false);

    ui_test_count_ch5(0);

    //ui->test_start_time_ch1->setText("Test start :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));
    ui->meter_info_ch5->setText("Test Start :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));

    /*button color change*/
    ui->test_pause_ch5->setStyleSheet("default");
    ui->test_start_ch5->setStyleSheet("background-color:rgb(244,0,0);border-style:insert");

}

void MainWindow::ui_set_measurement_stop_ch5()
{
    ui->test_start_ch5->setEnabled(true);
    ui->test_stop_ch5->setEnabled(false);
    ui->test_pause_ch5->setEnabled(false);

    ui->meter_type_ch5->setEnabled(true);
    ui->measure_count_ch5->setEnabled(true);
    ui->sec_startdelay_ch5->setEnabled(true);
    ui->sec_detoffdelay_ch5->setEnabled(true);
    ui->sec_interval_ch5->setEnabled(true);
    ui->audo_download_ch5->setEnabled(false);

    ui->device_open_ch5->setEnabled(true);
    ui->micro_usb_ch5->setEnabled(true);
    ui->phone_jack_ch5->setEnabled(true);

//  ui->test_step_ch1->setText("Action : stopped");
    ui->meter_info_ch5->append("Test stopped :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));

    ui->test_start_ch5->setStyleSheet("default");
    ui->test_pause_ch5->setStyleSheet("default");
}

void MainWindow::ui_set_measurement_pause_ch5()
{
    ui->test_start_ch5->setEnabled(true);
    ui->test_stop_ch5->setEnabled(true);
    ui->test_pause_ch5->setEnabled(false);

    ui->device_open_ch5->setEnabled(true);
    ui->micro_usb_ch5->setEnabled(true);
    ui->phone_jack_ch5->setEnabled(true);

    ui->test_start_ch5->setStyleSheet("default");
    ui->test_pause_ch5->setStyleSheet("background-color:rgb(244,0,0);border-style:insert");
}


void MainWindow::ui_action_status_ch1(QString status)
{
    ui->test_step_ch1->setText("Current Action :  "+status);    

    if(status=="stop")
        ui_set_measurement_stop_ch1();
}

void MainWindow::ui_test_count_ch1(int count)
{
    ui->test_count_ch1->setText("Test Count :     " +  QString::number(count));
}

void MainWindow::ui_interval_time_update_ch1(int interval_time)
{
   ui->test_interval_ch1->setText("Interval Time (Sec.) :   " + QString::number(interval_time));
}

void MainWindow::ui_action_status_ch2(QString status)
{
    ui->test_step_ch2->setText("Current Action :  " + status);

    if(status=="stop")
        ui_set_measurement_stop_ch2();
}

void MainWindow::ui_test_count_ch2(int count)
{
    ui->test_count_ch2->setText("Count :     " + QString::number(count));
}

void MainWindow::ui_interval_time_update_ch2(int interval_time)
{
   ui->test_interval_ch2->setText("Interval Time (Sec.) :   " + QString::number(interval_time));
}

void MainWindow::ui_action_status_ch3(QString status)
{
    ui->test_step_ch3->setText("Current Action :  " + status);

    if(status=="stop")
        ui_set_measurement_stop_ch3();
}

void MainWindow::ui_test_count_ch3(int count)
{
    ui->test_count_ch3->setText("Count :     " + QString::number(count));
}

void MainWindow::ui_interval_time_update_ch3(int interval_time)
{
   ui->test_interval_ch3->setText("Interval Time (Sec.) :   " + QString::number(interval_time));
}

void MainWindow::ui_action_status_ch4(QString status)
{
    ui->test_step_ch4->setText("Current Action :  " + status);

    if(status=="stop")
        ui_set_measurement_stop_ch4();
}

void MainWindow::ui_test_count_ch4(int count)
{
    ui->test_count_ch4->setText("Count :     " + QString::number(count));
}

void MainWindow::ui_interval_time_update_ch4(int interval_time)
{
   ui->test_interval_ch4->setText("Interval Time (Sec.) :   " + QString::number(interval_time));
}

void MainWindow::ui_action_status_ch5(QString status)
{
    ui->test_step_ch5->setText("Current Action :  " + status);

    if(status=="stop")
        ui_set_measurement_stop_ch5();
}

void MainWindow::ui_test_count_ch5(int count)
{
    ui->test_count_ch5->setText("Count :     " + QString::number(count));
}

void MainWindow::ui_interval_time_update_ch5(int interval_time)
{
   ui->test_interval_ch5->setText("Interval Time (Sec.) :   " + QString::number(interval_time));
}

void MainWindow::on_test_start_ch1_clicked()
{
    //Glucose : detect -> (3.5 sec) -> work/third on -> (7 sec) -> detect off -> (4 sec)

    ui_set_measurement_start_ch1();

    emit measure_start_ch1();

    if(ui->all_channel_select->isChecked())
    {
        on_test_start_ch2_clicked();
        on_test_start_ch3_clicked();
        on_test_start_ch4_clicked();
        on_test_start_ch5_clicked();

        ui->test_stop_ch2->setEnabled(false);
        ui->test_pause_ch2->setEnabled(false);

        ui->test_stop_ch3->setEnabled(false);
        ui->test_pause_ch3->setEnabled(false);

        ui->test_stop_ch4->setEnabled(false);
        ui->test_pause_ch4->setEnabled(false);

        ui->test_stop_ch5->setEnabled(false);
        ui->test_pause_ch5->setEnabled(false);

    }
}

void MainWindow::on_test_stop_ch1_clicked()
{
    Log() << "measurement stop";

    ui_set_measurement_stop_ch1();

    emit measure_stop_ch1();

    if(ui->all_channel_select->isChecked())
    {
        on_test_stop_ch2_clicked();
        on_test_stop_ch3_clicked();
        on_test_stop_ch4_clicked();
        on_test_stop_ch5_clicked();
    }

}

void MainWindow::on_test_pause_ch1_clicked()
{
    ui_set_measurement_pause_ch1();

    emit measure_pause_ch1();

    if(ui->all_channel_select->isChecked())
    {
        on_test_pause_ch2_clicked();
        on_test_pause_ch3_clicked();
        on_test_pause_ch4_clicked();
        on_test_pause_ch5_clicked();
    }
}

void MainWindow::on_test_start_ch2_clicked()
{
    ui_set_measurement_start_ch2();

    emit measure_start_ch2();

}
void MainWindow::on_test_stop_ch2_clicked()
{
    ui_set_measurement_stop_ch2();

    emit measure_stop_ch2();
}

void MainWindow::on_test_pause_ch2_clicked()
{
    ui_set_measurement_pause_ch2();

    emit measure_pause_ch2();
}


void MainWindow::on_test_start_ch3_clicked()
{
    ui_set_measurement_start_ch3();

    emit measure_start_ch3();
}

void MainWindow::on_test_pause_ch3_clicked()
{
    ui_set_measurement_pause_ch3();

    emit measure_pause_ch3();
}

void MainWindow::on_test_stop_ch3_clicked()
{
    ui_set_measurement_stop_ch3();

    emit measure_stop_ch3();
}

void MainWindow::on_test_start_ch4_clicked()
{
    ui_set_measurement_start_ch4();

    emit measure_start_ch4();
}

void MainWindow::on_test_pause_ch4_clicked()
{
    ui_set_measurement_pause_ch4();

    emit measure_pause_ch4();
}

void MainWindow::on_test_stop_ch4_clicked()
{
    ui_set_measurement_stop_ch4();

    emit measure_stop_ch4();
}

void MainWindow::on_test_start_ch5_clicked()
{
    ui_set_measurement_start_ch5();

    emit measure_start_ch5();
}

void MainWindow::on_test_pause_ch5_clicked()
{
    ui_set_measurement_pause_ch5();

    emit measure_pause_ch5();
}

void MainWindow::on_test_stop_ch5_clicked()
{
    ui_set_measurement_stop_ch5();

    emit measure_stop_ch5();
}

void MainWindow::on_meter_type_ch1_activated(const QString &arg1)
{
    Log()<<"meter type : "<<arg1;

    if(ui->all_channel_select->isChecked())
    {
        ui->meter_type_ch2->setCurrentText(arg1);
        ui->meter_type_ch3->setCurrentText(arg1);
        ui->meter_type_ch4->setCurrentText(arg1);
        ui->meter_type_ch5->setCurrentText(arg1);
    }
}

void MainWindow::on_measure_count_ch1_valueChanged(const QString &arg1)
{
     m_test_param_ch1.target_measure_count = arg1.toInt(0,10);

    if(system_init_done)
    {
        if(meter_type_index_selected_measure_count == false)
           emit measure_setup_ch1(m_test_param_ch1);

        meter_type_index_selected_measure_count = false;
    }

     if(ui->all_channel_select->isChecked())
     {
        on_measure_count_ch2_valueChanged(arg1);
        ui->measure_count_ch2->setValue(arg1.toInt(0,10));

        on_measure_count_ch3_valueChanged(arg1);
        ui->measure_count_ch3->setValue(arg1.toInt(0,10));

        on_measure_count_ch4_valueChanged(arg1);
        ui->measure_count_ch4->setValue(arg1.toInt(0,10));

        on_measure_count_ch5_valueChanged(arg1);
        ui->measure_count_ch5->setValue(arg1.toInt(0,10));
     }
}

void MainWindow::on_sec_startdelay_ch1_valueChanged(const QString &arg1)
{
     m_test_param_ch1.work_on_time = arg1.toInt(0,10)*1000;

     if(system_init_done)
     {
        if(meter_type_index_selected_start_delay == false)
             emit measure_setup_ch1(m_test_param_ch1);

        meter_type_index_selected_start_delay = false;
     }

     if(ui->all_channel_select->isChecked())
     {
        on_sec_startdelay_ch2_valueChanged(arg1);
        ui->sec_startdelay_ch2->setValue(arg1.toInt(0,10));

        on_sec_startdelay_ch3_valueChanged(arg1);
        ui->sec_startdelay_ch3->setValue(arg1.toInt(0,10));

        on_sec_startdelay_ch4_valueChanged(arg1);
        ui->sec_startdelay_ch4->setValue(arg1.toInt(0,10));

        on_sec_startdelay_ch5_valueChanged(arg1);
        ui->sec_startdelay_ch5->setValue(arg1.toInt(0,10));
     }

}

void MainWindow::on_sec_detoffdelay_ch1_valueChanged(const QString &arg1)
{
     m_test_param_ch1.detect_off_time = arg1.toInt(0,10)*1000;

     if(system_init_done)
     {
        if(meter_type_index_selected_detoff_delay == false)
             emit measure_setup_ch1(m_test_param_ch1);

        meter_type_index_selected_detoff_delay = false;
     }

     if(ui->all_channel_select->isChecked())
     {
        on_sec_detoffdelay_ch2_valueChanged(arg1);
        ui->sec_detoffdelay_ch2->setValue(arg1.toInt(0,10));

        on_sec_detoffdelay_ch3_valueChanged(arg1);
        ui->sec_detoffdelay_ch3->setValue(arg1.toInt(0,10));

        on_sec_detoffdelay_ch4_valueChanged(arg1);
        ui->sec_detoffdelay_ch4->setValue(arg1.toInt(0,10));

        on_sec_detoffdelay_ch5_valueChanged(arg1);
        ui->sec_detoffdelay_ch5->setValue(arg1.toInt(0,10));
     }
}

void MainWindow::on_sec_interval_ch1_valueChanged(const QString &arg1)
{
     m_test_param_ch1.test_interval_time = arg1.toInt(0,10)*1000;

     if(system_init_done)
     {
        if(meter_type_index_selected_interval == false)
             emit measure_setup_ch1(m_test_param_ch1);

         meter_type_index_selected_interval = false;
     }

     if(ui->all_channel_select->isChecked())
     {
        on_sec_interval_ch2_valueChanged(arg1);
        ui->sec_interval_ch2->setValue(arg1.toInt(0,10));

        on_sec_interval_ch3_valueChanged(arg1);
        ui->sec_interval_ch3->setValue(arg1.toInt(0,10));

        on_sec_interval_ch4_valueChanged(arg1);
        ui->sec_interval_ch4->setValue(arg1.toInt(0,10));

        on_sec_interval_ch5_valueChanged(arg1);
        ui->sec_interval_ch5->setValue(arg1.toInt(0,10));
     }
}

void MainWindow::on_meter_type_ch2_activated(const QString &arg1)
{
    Log()<< arg1;
}

void MainWindow::on_measure_count_ch2_valueChanged(const QString &arg1)
{
     m_test_param_ch2.target_measure_count = arg1.toInt(0,10);

     if(system_init_done)
     {
        if(meter_type_index_selected_measure_count == false)
            emit measure_setup_ch2(m_test_param_ch2);

         meter_type_index_selected_measure_count = false;
     }
}

void MainWindow::on_sec_startdelay_ch2_valueChanged(const QString &arg1)
{
     m_test_param_ch2.work_on_time = arg1.toInt(0,10)*1000;

     if(system_init_done)
     {
           if(meter_type_index_selected_start_delay == false)
                emit measure_setup_ch2(m_test_param_ch2);

           meter_type_index_selected_start_delay = false;
     }
}

void MainWindow::on_sec_detoffdelay_ch2_valueChanged(const QString &arg1)
{
     m_test_param_ch2.detect_off_time = arg1.toInt(0,10)*1000;

     if(system_init_done)
     {
         if(meter_type_index_selected_detoff_delay == false)
            emit measure_setup_ch2(m_test_param_ch2);

          meter_type_index_selected_detoff_delay = false;
     }
}

void MainWindow::on_sec_interval_ch2_valueChanged(const QString &arg1)
{
     m_test_param_ch2.test_interval_time = arg1.toInt(0,10)*1000;

     if(system_init_done)
     {
        if(meter_type_index_selected_interval == false)
            emit measure_setup_ch2(m_test_param_ch2);

        meter_type_index_selected_interval = false;
     }
}

void MainWindow::on_meter_type_ch3_activated(const QString &arg1)
{
    Log()<< arg1;
}

void MainWindow::on_measure_count_ch3_valueChanged(const QString &arg1)
{
    m_test_param_ch3.target_measure_count = arg1.toInt(0,10);

    if(system_init_done)
    {
        if(meter_type_index_selected_measure_count == false)
            emit measure_setup_ch3(m_test_param_ch3);

        meter_type_index_selected_measure_count = false;
    }
}

void MainWindow::on_sec_startdelay_ch3_valueChanged(const QString &arg1)
{
    m_test_param_ch3.work_on_time = arg1.toInt(0,10)*1000;

    if(system_init_done)
    {
         if(meter_type_index_selected_start_delay == false)
            emit measure_setup_ch3(m_test_param_ch3);

         meter_type_index_selected_start_delay = false;
    }
}

void MainWindow::on_sec_detoffdelay_ch3_valueChanged(const QString &arg1)
{
    m_test_param_ch3.detect_off_time = arg1.toInt(0,10)*1000;

    if(system_init_done)
    {

       if(meter_type_index_selected_detoff_delay == false)
           emit measure_setup_ch3(m_test_param_ch3);

        meter_type_index_selected_detoff_delay =false;
    }
}

void MainWindow::on_sec_interval_ch3_valueChanged(const QString &arg1)
{
    m_test_param_ch3.test_interval_time = arg1.toInt(0,10)*1000;

    if(system_init_done)
    {
        if(meter_type_index_selected_interval == false)
            emit measure_setup_ch3(m_test_param_ch3);

        meter_type_index_selected_interval = false;
    }
}

void MainWindow::on_meter_type_ch4_activated(const QString &arg1)
{
    Log()<< arg1;
}

void MainWindow::on_measure_count_ch4_valueChanged(const QString &arg1)
{
    m_test_param_ch4.target_measure_count = arg1.toInt(0,10);

    if(system_init_done)
    {
         if(meter_type_index_selected_measure_count == false)
            emit measure_setup_ch4(m_test_param_ch4);

         meter_type_index_selected_measure_count = false;
    }
}

void MainWindow::on_sec_startdelay_ch4_valueChanged(const QString &arg1)
{
    m_test_param_ch4.work_on_time = arg1.toInt(0,10)*1000;

    if(system_init_done)
    {
        if(meter_type_index_selected_start_delay == false)
            emit measure_setup_ch4(m_test_param_ch4);

        meter_type_index_selected_start_delay = false;
    }
}

void MainWindow::on_sec_detoffdelay_ch4_valueChanged(const QString &arg1)
{
    m_test_param_ch4.detect_off_time = arg1.toInt(0,10)*1000;

    if(system_init_done)
    {
        if(meter_type_index_selected_detoff_delay == false)
            emit measure_setup_ch4(m_test_param_ch4);

        meter_type_index_selected_detoff_delay = false;
    }
}

void MainWindow::on_sec_interval_ch4_valueChanged(const QString &arg1)
{
    m_test_param_ch4.test_interval_time = arg1.toInt(0,10)*1000;

    if(system_init_done)
    {
         if(meter_type_index_selected_interval == false)
            emit measure_setup_ch4(m_test_param_ch4);

         meter_type_index_selected_interval = false;
    }
}

void MainWindow::on_meter_type_ch5_activated(const QString &arg1)
{
    Log()<< arg1;
}

void MainWindow::on_measure_count_ch5_valueChanged(const QString &arg1)
{
    m_test_param_ch5.target_measure_count = arg1.toInt(0,10);

    if(system_init_done)
    {
         if(meter_type_index_selected_measure_count == false)
            emit measure_setup_ch5(m_test_param_ch5);

         meter_type_index_selected_measure_count = false;
    }
}

void MainWindow::on_sec_startdelay_ch5_valueChanged(const QString &arg1)
{
    m_test_param_ch5.work_on_time = arg1.toInt(0,10)*1000;

    if(system_init_done)
    {
        if(meter_type_index_selected_start_delay == false)
           emit measure_setup_ch5(m_test_param_ch5);

        meter_type_index_selected_start_delay = false;
    }
}

void MainWindow::on_sec_detoffdelay_ch5_valueChanged(const QString &arg1)
{
    m_test_param_ch5.detect_off_time = arg1.toInt(0,10)*1000;

    if(system_init_done)
    {
         if(meter_type_index_selected_detoff_delay == false)
            emit measure_setup_ch5(m_test_param_ch5);

         meter_type_index_selected_detoff_delay = false;
    }
}

void MainWindow::on_sec_interval_ch5_valueChanged(const QString &arg1)
{
    m_test_param_ch5.test_interval_time = arg1.toInt(0,10)*1000;

    if(system_init_done)
    {
        if(meter_type_index_selected_interval == false)
            emit measure_setup_ch5(m_test_param_ch5);

        meter_type_index_selected_interval = false;
    }
}

void MainWindow::on_download_ch2_clicked()
{

}

void MainWindow::currentMeterIndexChanged(int index)
{

    switch (index)
    {
          case measurement_param::GLUCOSE_BASIC:
            Log()<<"GLUCOSE selected";

                m_test_param_tmp.meter_memory_capacity = 1000;
                m_test_param_tmp.target_measure_count =1000;
                m_test_param_tmp.work_on_time = 3000;
                m_test_param_tmp.third_on_time =0; // 6000;
                m_test_param_tmp.detect_off_time = 8000; //14000;
                m_test_param_tmp.test_interval_time = 15000;

          break;

          case measurement_param::GLUCOSE_BLE:

            Log()<<"GLUCOSE BLE selected";

                m_test_param_tmp.meter_memory_capacity = 1000;
                m_test_param_tmp.target_measure_count =1000;
                m_test_param_tmp.work_on_time = 3000;
                m_test_param_tmp.third_on_time =0; // 6000;
                m_test_param_tmp.detect_off_time = 8000; //14000;
                m_test_param_tmp.test_interval_time = 25000;

          break;

          case measurement_param::GLUCOSE_VOICE:

            Log()<<"GLUCOSE VOICE selected";

                m_test_param_tmp.meter_memory_capacity = 500;
                m_test_param_tmp.target_measure_count =500;
                m_test_param_tmp.work_on_time = 7000;
                m_test_param_tmp.third_on_time =0; // 6000;
                m_test_param_tmp.detect_off_time = 11000; //14000;
                m_test_param_tmp.test_interval_time = 15000;

          break;

          case measurement_param::GLUCOSE_VOICE_BLE:

            Log()<<"GLUCOSE VOICE BLE selected";

                m_test_param_tmp.meter_memory_capacity = 1000;
                m_test_param_tmp.target_measure_count =1000;
                m_test_param_tmp.work_on_time = 7000;
                m_test_param_tmp.third_on_time =0; // 6000;
                m_test_param_tmp.detect_off_time = 11000; //14000;
                m_test_param_tmp.test_interval_time = 25000;

          break;

          case measurement_param::meter_type::KETONE_BASIC:

            Log()<<"KETONE selected";

                m_test_param_tmp.meter_memory_capacity = 1000;
                m_test_param_tmp.target_measure_count =1000;
                m_test_param_tmp.work_on_time = 7000;
                m_test_param_tmp.third_on_time =0; // 6000;
                m_test_param_tmp.detect_off_time = 11000; //14000;
                m_test_param_tmp.test_interval_time = 25000;

          break;

          case measurement_param::meter_type::KETONE_BLE:

            Log()<<"KETONE BLE selected";

                m_test_param_tmp.meter_memory_capacity = 1000;
                m_test_param_tmp.target_measure_count =1000;
                m_test_param_tmp.work_on_time = 7000;
                m_test_param_tmp.third_on_time =0; // 6000;
                m_test_param_tmp.detect_off_time = 11000; //14000;
                m_test_param_tmp.test_interval_time = 25000;

          break;

          default:

             Log()<<"default Glucose Basic type selected";

                m_test_param_tmp.meter_memory_capacity = 1000;
                m_test_param_tmp.target_measure_count =1000;
                m_test_param_tmp.work_on_time = 3000;
                m_test_param_tmp.third_on_time =0; // 6000;
                m_test_param_tmp.detect_off_time = 8000; //14000;
                m_test_param_tmp.test_interval_time = 15000;

          break;
    }


    if(sender())
    {
        qDebug()<<"received signal";

        /*Set test parameter on spin box*/

        if(sender() == ui->meter_type_ch1)
        {
             meter_type_index_selected_measure_count = true;
             meter_type_index_selected_start_delay = true;
             meter_type_index_selected_detoff_delay = true;
             meter_type_index_selected_interval = true;

             memcpy(&m_test_param_ch1, &m_test_param_tmp, sizeof(m_test_param_tmp));

             ui->measure_count_ch1->setValue(static_cast<int>(m_test_param_ch1.target_measure_count));
             ui->sec_startdelay_ch1->setValue(static_cast<int>(m_test_param_ch1.work_on_time/1000));
             ui->sec_detoffdelay_ch1->setValue(static_cast<int>(m_test_param_ch1.detect_off_time/1000));
             ui->sec_interval_ch1->setValue(static_cast<int>(m_test_param_ch1.test_interval_time/1000));

             Log();

             emit measure_setup_ch1(m_test_param_ch1);

             if(ui->all_channel_select->isChecked())
             {

                emit measure_setup_ch2(m_test_param_ch1);

                emit measure_setup_ch3(m_test_param_ch1);

                emit measure_setup_ch4(m_test_param_ch1);

                emit measure_setup_ch5(m_test_param_ch1);
             }
        }
       else if(sender() == ui->meter_type_ch2)
        {
            meter_type_index_selected_measure_count = true;
            meter_type_index_selected_start_delay = true;
            meter_type_index_selected_detoff_delay = true;
            meter_type_index_selected_interval = true;

            memcpy(&m_test_param_ch2, &m_test_param_tmp, sizeof(m_test_param_tmp));

            ui->measure_count_ch2->setValue(static_cast<int>(m_test_param_ch2.target_measure_count));
            ui->sec_startdelay_ch2->setValue(static_cast<int>(m_test_param_ch2.work_on_time/1000));
            ui->sec_detoffdelay_ch2->setValue(static_cast<int>(m_test_param_ch2.detect_off_time/1000));
            ui->sec_interval_ch2->setValue(static_cast<int>(m_test_param_ch2.test_interval_time/1000));

            emit measure_setup_ch2(m_test_param_ch2);
        }
        else if(sender() == ui->meter_type_ch3)
        {
            meter_type_index_selected_measure_count = true;
            meter_type_index_selected_start_delay = true;
            meter_type_index_selected_detoff_delay = true;
            meter_type_index_selected_interval = true;

            memcpy(&m_test_param_ch3, &m_test_param_tmp, sizeof(m_test_param_tmp));

            ui->measure_count_ch3->setValue(static_cast<int>(m_test_param_ch3.target_measure_count));
            ui->sec_startdelay_ch3->setValue(static_cast<int>(m_test_param_ch3.work_on_time/1000));
            ui->sec_detoffdelay_ch3->setValue(static_cast<int>(m_test_param_ch3.detect_off_time/1000));
            ui->sec_interval_ch3->setValue(static_cast<int>(m_test_param_ch3.test_interval_time/1000));

            emit measure_setup_ch3(m_test_param_ch3);

        }
        else if(sender() == ui->meter_type_ch4)
        {
            meter_type_index_selected_measure_count = true;
            meter_type_index_selected_start_delay = true;
            meter_type_index_selected_detoff_delay = true;
            meter_type_index_selected_interval = true;

            memcpy(&m_test_param_ch4, &m_test_param_tmp, sizeof(m_test_param_tmp));

            ui->measure_count_ch4->setValue(static_cast<int>(m_test_param_ch4.target_measure_count));
            ui->sec_startdelay_ch4->setValue(static_cast<int>(m_test_param_ch4.work_on_time/1000));
            ui->sec_detoffdelay_ch4->setValue(static_cast<int>(m_test_param_ch4.detect_off_time/1000));
            ui->sec_interval_ch4->setValue(static_cast<int>(m_test_param_ch4.test_interval_time/1000));

            emit measure_setup_ch4(m_test_param_ch4);
        }
        else if(sender() == ui->meter_type_ch5)
        {
            meter_type_index_selected_measure_count = true;
            meter_type_index_selected_start_delay = true;
            meter_type_index_selected_detoff_delay = true;
            meter_type_index_selected_interval = true;

            memcpy(&m_test_param_ch5, &m_test_param_tmp, sizeof(m_test_param_tmp));

            ui->measure_count_ch5->setValue(static_cast<int>(m_test_param_ch5.target_measure_count));
            ui->sec_startdelay_ch5->setValue(static_cast<int>(m_test_param_ch5.work_on_time/1000));
            ui->sec_detoffdelay_ch5->setValue(static_cast<int>(m_test_param_ch5.detect_off_time/1000));
            ui->sec_interval_ch5->setValue(static_cast<int>(m_test_param_ch5.test_interval_time/1000));

            emit measure_setup_ch5(m_test_param_ch5);
        }
    }
    else
    {
        qDebug()<<"direct call";

        /*Set test parameter on spin box*/
        if(m_test_param_tmp.channel == measurement_param::CH_1)
        {
             ui->measure_count_ch1->setValue(static_cast<int>(m_test_param_tmp.target_measure_count));
             ui->sec_startdelay_ch1->setValue(static_cast<int>(m_test_param_tmp.work_on_time/1000));
             ui->sec_detoffdelay_ch1->setValue(static_cast<int>(m_test_param_tmp.detect_off_time/1000));
             ui->sec_interval_ch1->setValue(static_cast<int>(m_test_param_tmp.test_interval_time/1000));

             memcpy(&m_test_param_ch1, &m_test_param_tmp, sizeof(m_test_param_tmp));

             Log();

             emit measure_setup_ch1(m_test_param_ch1);

        }
       else if(m_test_param_tmp.channel == measurement_param::CH_2)
        {
            ui->measure_count_ch2->setValue(static_cast<int>(m_test_param_tmp.target_measure_count));
            ui->sec_startdelay_ch2->setValue(static_cast<int>(m_test_param_tmp.work_on_time/1000));
            ui->sec_detoffdelay_ch2->setValue(static_cast<int>(m_test_param_tmp.detect_off_time/1000));
            ui->sec_interval_ch2->setValue(static_cast<int>(m_test_param_tmp.test_interval_time/1000));

            memcpy(&m_test_param_ch2, &m_test_param_tmp, sizeof(m_test_param_tmp));

            emit measure_setup_ch2(m_test_param_ch2);
        }
        else if(m_test_param_tmp.channel == measurement_param::CH_3)
        {
            ui->measure_count_ch3->setValue(static_cast<int>(m_test_param_tmp.target_measure_count));
            ui->sec_startdelay_ch3->setValue(static_cast<int>(m_test_param_tmp.work_on_time/1000));
            ui->sec_detoffdelay_ch3->setValue(static_cast<int>(m_test_param_tmp.detect_off_time/1000));
            ui->sec_interval_ch3->setValue(static_cast<int>(m_test_param_tmp.test_interval_time/1000));

            memcpy(&m_test_param_ch3, &m_test_param_tmp, sizeof(m_test_param_tmp));

            emit measure_setup_ch3(m_test_param_ch3);
        }
        else if(m_test_param_tmp.channel == measurement_param::CH_4)
        {
            ui->measure_count_ch4->setValue(static_cast<int>(m_test_param_tmp.target_measure_count));
            ui->sec_startdelay_ch4->setValue(static_cast<int>(m_test_param_tmp.work_on_time/1000));
            ui->sec_detoffdelay_ch4->setValue(static_cast<int>(m_test_param_tmp.detect_off_time/1000));
            ui->sec_interval_ch4->setValue(static_cast<int>(m_test_param_tmp.test_interval_time/1000));

            memcpy(&m_test_param_ch4, &m_test_param_tmp, sizeof(m_test_param_tmp));

            emit measure_setup_ch4(m_test_param_ch4);
        }
        else if(m_test_param_tmp.channel == measurement_param::CH_5)
        {
            ui->measure_count_ch5->setValue(static_cast<int>(m_test_param_tmp.target_measure_count));
            ui->sec_startdelay_ch5->setValue(static_cast<int>(m_test_param_tmp.work_on_time/1000));
            ui->sec_detoffdelay_ch5->setValue(static_cast<int>(m_test_param_tmp.detect_off_time/1000));
            ui->sec_interval_ch5->setValue(static_cast<int>(m_test_param_tmp.test_interval_time/1000));

            memcpy(&m_test_param_ch5, &m_test_param_tmp, sizeof(m_test_param_tmp));

            emit measure_setup_ch5(m_test_param_ch5);
        }
    }

    memset(&m_test_param_tmp, 0x0, sizeof(m_test_param_tmp));
}

string MainWindow::do_console_command_get_result (char* command)
{
    FILE* pipe = popen(command, "r");		//Send the command, popen exits immediately
        if (!pipe)
            return "ERROR";

        char buffer[256];
        string result = "";
        while(!feof(pipe))						//Wait for the output resulting from the command
        {
            if(fgets(buffer, 256, pipe) != NULL)
                result += buffer;
        }
        pclose(pipe);
        return(result);
}

void MainWindow::ui_system_info_setup()
{
    /*Show Build information*/
    ui->build_date->setText("build date : " + build_date);

    //Print local machine's eth0 IP address

#if 0

    QNetworkInterface *qnet;
    qnet = new QNetworkInterface();
    *qnet = qnet->interfaceFromName(QString("%1").arg("eth0"));
    server_ip = qnet->addressEntries().at(0).ip().toString();
    ui->ip_address->setText("IP Address : " + qnet->addressEntries().at(0).ip().toString());

#else
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (int i = 0; i < interfaces.count(); i++)
    {
        QList<QNetworkAddressEntry> entries = interfaces.at(i).addressEntries();

        for (int j = 0; j < entries.count(); j++)
        {
            if (entries.at(j).ip().protocol() == QAbstractSocket::IPv4Protocol)
            {
                qDebug() << entries.at(j).ip().toString();
                qDebug() << entries.at(j).netmask().toString();

                ui->ip_address->setText("IP Address : " + entries.at(j).ip().toString());

                server_ip = entries.at(j).ip().toString();
            }
        }
    }

#endif

    board_info = QString::fromStdString(do_console_command_get_result ("cat /proc/device-tree/model"));

    if(board_info.contains("Raspberry Pi 3 Model B",Qt::CaseInsensitive))
        board_version = RASPBERRY_PI3_B;
    else if(board_info.contains("Raspberry Pi 3 Model B plus",Qt::CaseInsensitive))
        board_version = RASPBERRY_PI3_B_PLUS;
    else
        board_version = RASPBERRY_PI_UNKNOWN;

//   ui->board->setText("board :" + board_info);

    QObject::connect(timer_sec, SIGNAL(timeout()), this, SLOT(UpdateTime()));
    timer_sec->start(1000);
}

void MainWindow::UpdateTime()
{
      ui->system_time->setText("Time: " + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));
}

void MainWindow::dmm_working_status(QString dmm_status)
{
    //ui->dmm_status->setText("[ " + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap") +" ] " + dmm_status);
    //ui->dmm_status->append("[ " + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap") +" ] " + dmm_status);
}

void MainWindow::on_reboot_clicked()
{
//    QProcess process;
//    process.startDetached("sudo reboot");
      m_console_command->sendCommand("sudo reboot");
}

void MainWindow::on_quit_clicked()
{
     //QProcess process;
     //process.startDetached("sudo poweroff");
     m_console_command->sendCommand("sudo poweroff");
}

void MainWindow::on_dmm_capture_stateChanged(int arg1)
{
    if(system_init_done)
    {

        if(arg1)
           m_test_param_ch1.use_u1272a = true;
        else
           m_test_param_ch1.use_u1272a = false;

        emit measure_setup_ch1(m_test_param_ch1);
    }
}

void MainWindow::on_device_open_ch1_clicked()
{
    if(ui->phone_jack_ch1->isChecked())
    {
        m_hid_uart_comm[0] = new hid_uart_comm(0x0);
        comm_Thread[0] = new QThread(this);
        m_hid_uart_comm[0]->moveToThread(comm_Thread[0]);

        connect(comm_Thread[0], &QThread::finished, m_hid_uart_comm[0], &QObject::deleteLater);
        connect(m_hid_uart_comm[0], SIGNAL(sig_bgms_comm_response(sys_cmd_resp *)), this, SLOT(ui_bgms_comm_ch_1_response(sys_cmd_resp *)));

        comm_Thread[0]->start();
    }
    else
    {

    }

    ui->meter_info_ch01->clear();
}

void MainWindow::on_device_close_ch1_clicked()
{
    disconnect(this, SIGNAL(sig_bgms_comm_cmd(sys_cmd_resp *)), m_hid_uart_comm[0], SLOT(cmd_from_host(sys_cmd_resp *)));
    comm_Thread[0]->exit();

    ui_set_measurement_stop_ch1();

    ui_set_comm_open_close(meter_channel::CH_1, comm_set::SET_CLOSE);

}

void MainWindow::on_time_sync_ch1_clicked()
{
    comm_cmd->m_comm_cmd = sys_cmd_resp::CMD_COMM_SET_TIME;

    emit sig_bgms_comm_cmd(comm_cmd);
}

void MainWindow::on_mem_delete_ch1_clicked()
{
    comm_cmd->m_comm_cmd = sys_cmd_resp::CMD_COMM_MEM_DELETE;

    emit sig_bgms_comm_cmd(comm_cmd);
}

void MainWindow::on_download_ch1_clicked()
{
     comm_cmd->m_comm_cmd = sys_cmd_resp::CMD_COMM_DOWNLOAD;

     emit sig_bgms_comm_cmd(comm_cmd);
}

void MainWindow::on_device_open_ch2_clicked()
{
    if(ui->micro_usb_ch1->isChecked())
    {

    }
    else
    {

    }
}

void MainWindow::on_device_close_ch2_clicked()
{

}

void MainWindow::on_device_open_ch3_clicked()
{
    if(ui->micro_usb_ch1->isChecked())
    {

    }
    else
    {

    }
}


void MainWindow::on_device_close_ch3_clicked()
{

}

void MainWindow::ui_set_comm_open_close(meter_channel channel, comm_set open_close)
{
    switch(channel)
    {
        case meter_channel::CH_1:

            if(open_close == comm_set::SET_CLOSE)
            {
                ui->micro_usb_ch1->setEnabled(true);
                ui->phone_jack_ch1->setEnabled(true);
                ui->device_open_ch1->setEnabled(true);

                ui->time_sync_ch1->setEnabled(false);
                ui->mem_delete_ch1->setEnabled(false);
                ui->download_ch1->setEnabled(false);
                ui->device_close_ch1->setEnabled(false);
            }
            else
            {
                ui->micro_usb_ch1->setEnabled(false);
                ui->phone_jack_ch1->setEnabled(false);
                ui->device_open_ch1->setEnabled(true);

                ui->time_sync_ch1->setEnabled(true);
                ui->mem_delete_ch1->setEnabled(true);
                ui->download_ch1->setEnabled(true);
                ui->device_close_ch1->setEnabled(true);
            }

        break;

        case meter_channel::CH_2:

            if(open_close == comm_set::SET_CLOSE)
            {
                ui->micro_usb_ch2->setEnabled(true);
                ui->phone_jack_ch2->setEnabled(true);
                ui->device_open_ch2->setEnabled(true);

                ui->time_sync_ch2->setEnabled(false);
                ui->mem_delete_ch2->setEnabled(false);
                ui->download_ch2->setEnabled(false);
                ui->device_close_ch2->setEnabled(false);
            }
            else
            {
                ui->micro_usb_ch2->setEnabled(false);
                ui->phone_jack_ch2->setEnabled(false);
                ui->device_open_ch2->setEnabled(true);

                ui->time_sync_ch2->setEnabled(true);
                ui->mem_delete_ch2->setEnabled(true);
                ui->download_ch2->setEnabled(true);
                ui->device_close_ch2->setEnabled(true);
            }

        break;

        case meter_channel::CH_3:

            if(open_close == comm_set::SET_CLOSE)
            {
                ui->micro_usb_ch3->setEnabled(true);
                ui->phone_jack_ch3->setEnabled(true);
                ui->device_open_ch3->setEnabled(true);

                ui->time_sync_ch3->setEnabled(false);
                ui->mem_delete_ch3->setEnabled(false);
                ui->download_ch3->setEnabled(false);
                ui->device_close_ch3->setEnabled(false);
            }
            else
            {
                ui->micro_usb_ch3->setEnabled(false);
                ui->phone_jack_ch3->setEnabled(false);
                ui->device_open_ch3->setEnabled(true);

                ui->time_sync_ch3->setEnabled(true);
                ui->mem_delete_ch3->setEnabled(true);
                ui->download_ch3->setEnabled(true);
                ui->device_close_ch3->setEnabled(true);
            }

        break;

    }
}


void MainWindow::ui_bgms_comm_ch_1_response(sys_cmd_resp *resp_comm)
{

    ui->meter_info_ch01->append(comm_response_msg[static_cast<quint8>(resp_comm->m_comm_resp)]);

    switch (resp_comm->m_comm_resp)
    {
        case  sys_cmd_resp::RESP_COMM_PORT_OPEN_SUCCESS:

            ui_set_measurement_start_ch1();
            ui->test_pause_ch1->setEnabled(false);
            ui->test_stop_ch1->setEnabled(false);

            ui_set_comm_open_close(meter_channel::CH_1, comm_set::SET_OPEN);

            connect(this, SIGNAL(sig_bgms_comm_cmd(sys_cmd_resp *)), m_hid_uart_comm[0], SLOT(cmd_from_host(sys_cmd_resp *)));

            comm_cmd->m_comm_cmd = sys_cmd_resp::CMD_COMM_BGMS_CHECK;
            emit sig_bgms_comm_cmd(comm_cmd);

        break;

        case  sys_cmd_resp::RESP_COMM_PORT_CLOSE_SUCCESS:


        case  sys_cmd_resp::RESP_COMM_PORT_CLOSE_FAIL:
        case  sys_cmd_resp::RESP_COMM_PORT_OPEN_FAIL:
        break;

        case  sys_cmd_resp::RESP_COMM_BGMS_CHECK_FAIL:

            on_device_close_ch1_clicked();

        break;

        case  sys_cmd_resp::RESP_COMM_BGMS_CHECK_SUCCESS:

            ui->meter_info_ch01->append(comm_response_msg[static_cast<quint8>(resp_comm->m_protocol_type)]);

            comm_cmd->m_comm_cmd = sys_cmd_resp::CMD_COMM_GET_TIME;
            emit sig_bgms_comm_cmd(comm_cmd);

            break;

        case  sys_cmd_resp::RESP_COMM_GET_TIME_SUCCESS:

            ui->meter_info_ch01->insertPlainText(resp_comm->current_bgms_time);

            comm_cmd->m_comm_cmd = sys_cmd_resp::CMD_COMM_READ_SERIAL;
            emit sig_bgms_comm_cmd(comm_cmd);

        break;

        case  sys_cmd_resp::RESP_COMM_READ_SERIAL_SUCCESS:

             ui->meter_info_ch01->insertPlainText(resp_comm->serial_number);

             comm_cmd->m_comm_cmd = sys_cmd_resp::CMD_COMM_GET_STORED_VALUE_COUNT;
             emit sig_bgms_comm_cmd(comm_cmd);

            break;

        case  sys_cmd_resp::RESP_COMM_SET_TIME_SUCCESS:
            break;

        case  sys_cmd_resp::RESP_COMM_MEM_DELETE_SUCCESS:

            comm_cmd->m_comm_cmd = sys_cmd_resp::CMD_COMM_GET_STORED_VALUE_COUNT;
            emit sig_bgms_comm_cmd(comm_cmd);

            break;

        case  sys_cmd_resp::RESP_COMM_GET_STORED_VALUE_COUNT_SUCCESS:

            ui->meter_info_ch01->insertPlainText(QString::number(resp_comm->measured_result));

            break;

        case  sys_cmd_resp::RESP_COMM_DOWNLOAD_SUCCESS:
            break;

        case  sys_cmd_resp::RESP_COMM_READ_SERIAL_FAIL:
            break;

        case  sys_cmd_resp::RESP_COMM_GET_TIME_FAIL:
            break;

        case  sys_cmd_resp::RESP_COMM_SET_TIME_FAIL:
            break;

        case  sys_cmd_resp::RESP_COMM_GET_STORED_VALUE_COUNT_FAIL:
            break;

        case  sys_cmd_resp::RESP_COMM_DOWNLOAD_FAIL:
            break;

        case  sys_cmd_resp::RESP_COMM_BGMS_RESP_FAIL:
            break;

        case  sys_cmd_resp::RESP_COMM_MEM_DELETE_FAIL:
            break;

        case  sys_cmd_resp::RESP_COMM_UNKNOWN:
            break;
    }
}

void MainWindow::ui_bgms_comm_ch_2_response(sys_cmd_resp *resp_comm)
{

}

void MainWindow::ui_bgms_comm_ch_3_response(sys_cmd_resp *resp_comm)
{

}

void MainWindow::on_audo_download_ch1_stateChanged(int arg1)
{
    Log()<<arg1;

    if(arg1)
    {
        m_console_command->sendCommand("sudo mount "+dev_path+" /mnt/storage -o uid=pi,gid=pi");

        ui->audo_download_ch2->setCheckState(Qt::Checked);
        ui->audo_download_ch3->setCheckState(Qt::Checked);
        ui->audo_download_ch4->setCheckState(Qt::Checked);
        ui->audo_download_ch5->setCheckState(Qt::Checked);
    }
    else
    {
        m_console_command->sendCommand("sudo umount /mnt/storage");

        ui->audo_download_ch2->setCheckState(Qt::Unchecked);
        ui->audo_download_ch3->setCheckState(Qt::Unchecked);
        ui->audo_download_ch4->setCheckState(Qt::Unchecked);
        ui->audo_download_ch5->setCheckState(Qt::Unchecked);

    }

    ui->audo_download_ch2->setEnabled(false);
    ui->audo_download_ch3->setEnabled(false);
    ui->audo_download_ch4->setEnabled(false);
    ui->audo_download_ch5->setEnabled(false);
}

MainWindow::~MainWindow()
{
      delete m_control;
      delete ui;
}
