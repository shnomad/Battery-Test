
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
#include "control.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
      ui->setupUi(this);

      qRegisterMetaType<measurement_param>();

      timer_sec = new QTimer(this);                   //display system time

      ui_create_measurement();

      ui_init_measurement();

      ui_system_info_setup();
}

void MainWindow::ui_init_measurement()
{

    /*
    * CHANNEL_1 UI SETTINGS
    */

    /*Play/Pause/Stop Button*/
    ui->test_start_ch1->setIcon(QIcon(":/images/play.png"));
    ui->test_start_ch1->setIconSize(QSize(45,45));
    ui->test_stop_ch1->setIcon(QIcon(":/images/stop.png"));
    ui->test_stop_ch1->setIconSize(QSize(45,45));
    ui->test_pause_ch1->setIcon(QIcon(":/images/pause.png"));
    ui->test_pause_ch1->setIconSize(QSize(45,45));

    ui->test_stop_ch1->setEnabled(false);
    ui->test_pause_ch1->setEnabled(false);
    ui->device_open_ch1->setEnabled(true);
    ui->device_close_ch1->setEnabled(false);

    //Meter type select
    ui->meter_type_ch1->addItem("GLUCOSE BASIC");
    ui->meter_type_ch1->addItem("GLUCOSE BLE");
    ui->meter_type_ch1->addItem("GLUCOSE VOICE");
    ui->meter_type_ch1->addItem("GLUCOSE VOICE BLE");
    ui->meter_type_ch1->addItem("KETONE BASIC");
    ui->meter_type_ch1->addItem("KETONE BLE");           
    ui->meter_type_ch1->setCurrentIndex(0);

    /*Measurement Condition*/
    ui->meter_type_ch1->setEnabled(true);
    ui->measure_count_ch1->setEnabled(true);
    ui->sec_startdelay_ch1->setEnabled(true);
    ui->sec_detoffdelay_ch1->setEnabled(true);
    ui->sec_interval_ch1->setEnabled(true);

    /*Setting the Test Count*/
    ui->measure_count_ch1->setEnabled(true);
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
    connect(ui->meter_type_ch1,SIGNAL(currentIndexChanged(int)), this, SLOT(currentMeterIndexChanged(int)));

    /*Test capacity*/

 //  target_measure_count_rest = 1000;

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
    ui->test_start_ch2->setIcon(QIcon(":/images/play.png"));
    ui->test_start_ch2->setIconSize(QSize(45,45));
    ui->test_stop_ch2->setIcon(QIcon(":/images/stop.png"));
    ui->test_stop_ch2->setIconSize(QSize(45,45));
    ui->test_pause_ch2->setIcon(QIcon(":/images/pause.png"));
    ui->test_pause_ch2->setIconSize(QSize(45,45));

    ui->test_stop_ch2->setEnabled(false);
    ui->test_pause_ch2->setEnabled(false);
    ui->device_open_ch2->setEnabled(true);
    ui->device_close_ch2->setEnabled(false);

    //Meter type select
    ui->meter_type_ch2->addItem("GLUCOSE BASIC");
    ui->meter_type_ch2->addItem("GLUCOSE BLE");
    ui->meter_type_ch2->addItem("GLUCOSE VOICE");
    ui->meter_type_ch2->addItem("GLUCOSE VOICE BLE");
    ui->meter_type_ch2->addItem("KETONE BASIC");
    ui->meter_type_ch2->addItem("KETONE BLE");
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
    connect(ui->meter_type_ch2,SIGNAL(currentIndexChanged(int)), this, SLOT(currentMeterIndexChanged(int)));

    /*USB Interface select*/
    ui->micro_usb_ch2->setEnabled(true);
    ui->micro_usb_ch2->setChecked(true);

    ui->phone_jack_ch2->setEnabled(true);
    ui->micro_usb_ch2->setChecked(false);

    ui->time_sync_ch2->setEnabled(false);
    ui->mem_delete_ch2->setEnabled(false);
    ui->download_ch2->setEnabled(false);
}

void MainWindow::ui_create_measurement()
{

    m_control = new control;

    connect(this, SIGNAL(measure_setup_ch1(measurement_param)), m_control->m_ch[0], SLOT(setup(measurement_param)));
    connect(this, SIGNAL(measure_setup_ch2(measurement_param)), m_control->m_ch[1], SLOT(setup(measurement_param)));

    connect(this, SIGNAL(measure_start_ch1()), m_control->m_ch[0], SLOT(start()));
    connect(this, SIGNAL(measure_start_ch2()), m_control->m_ch[1], SLOT(start()));

    connect(this, SIGNAL(measure_stop_ch1()), m_control->m_ch[0], SLOT(stop()));
    connect(this, SIGNAL(measure_stop_ch2()), m_control->m_ch[1], SLOT(stop()));

    connect(this, SIGNAL(measure_pause_ch1()), m_control->m_ch[0], SLOT(pause()));
    connect(this, SIGNAL(measure_pause_ch2()), m_control->m_ch[1], SLOT(pause()));

    connect(m_control->m_ch[0], SIGNAL(update_test_count(int)), this, SLOT(ui_test_count_ch1(int)));
    connect(m_control->m_ch[1], SIGNAL(update_test_count(int)), this, SLOT(ui_test_count_ch2(int)));

    connect(m_control->m_ch[0], SIGNAL(update_action(QString)), this, SLOT(ui_action_status_ch1(QString)));
    connect(m_control->m_ch[1], SIGNAL(update_action(QString)), this, SLOT(ui_action_status_ch2(QString)));
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

    //Comm Interface option
    ui->micro_usb_ch1->setEnabled(false);
    ui->phone_jack_ch1->setEnabled(false);

    ui_test_count_ch1(0);

    ui->test_start_time_ch1->setText("Test start :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));
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

    ui->device_open_ch1->setEnabled(true);
    ui->micro_usb_ch1->setEnabled(true);
    ui->phone_jack_ch1->setEnabled(true);

//  ui->test_step_ch1->setText("Action : stopped");
}

void MainWindow::ui_set_measurement_pause_ch1()
{
    ui->test_start_ch1->setEnabled(true);
    ui->test_stop_ch1->setEnabled(true);
    ui->test_pause_ch1->setEnabled(false);

    ui->device_open_ch1->setEnabled(true);
    ui->micro_usb_ch1->setEnabled(true);
    ui->phone_jack_ch1->setEnabled(true);
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

    //Comm Interface option
    ui->micro_usb_ch2->setEnabled(false);
    ui->phone_jack_ch2->setEnabled(false);

    ui_test_count_ch2(0);

    ui->test_start_time_ch2->setText("Test start :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));
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

    ui->device_open_ch2->setEnabled(true);
    ui->micro_usb_ch2->setEnabled(true);
    ui->phone_jack_ch2->setEnabled(true);
}

void MainWindow::ui_set_measurement_pause_ch2()
{
    ui->test_start_ch2->setEnabled(true);
    ui->test_stop_ch2->setEnabled(true);
    ui->test_pause_ch2->setEnabled(false);

    ui->device_open_ch2->setEnabled(true);
    ui->micro_usb_ch2->setEnabled(true);
    ui->phone_jack_ch2->setEnabled(true);
}

void MainWindow::ui_action_status_ch1(QString status)
{
    ui->test_step_ch1->setText("Action :  "+status);

    if(status=="stop")
        ui_set_measurement_stop_ch1();
}

void MainWindow::ui_test_count_ch1(int count)
{
    ui->test_count_ch1->setText("Count :     " +  QString::number(count));
}

void MainWindow::ui_action_status_ch2(QString status)
{
    ui->test_step_ch2->setText("Action :  " + status);

    if(status=="stop")
        ui_set_measurement_stop_ch2();
}

void MainWindow::ui_test_count_ch2(int count)
{
    ui->test_count_ch2->setText("Count :     " + QString::number(count));
}

void MainWindow::on_test_start_ch1_clicked()
{
    //Glucose : detect -> (3.5 sec) -> work/third on -> (7 sec) -> detect off -> (4 sec)

    ui_set_measurement_start_ch1();

    emit measure_start_ch1();
}

void MainWindow::on_test_stop_ch1_clicked()
{
    Log() << "measurement stop";

    ui_set_measurement_stop_ch1();

    emit measure_stop_ch1();
}

void MainWindow::on_test_pause_ch1_clicked()
{
    ui_set_measurement_pause_ch1();

    emit measure_pause_ch1();
}

void MainWindow::on_meter_type_ch1_activated(const QString &arg1)
{

}

void MainWindow::on_measure_count_ch1_valueChanged(const QString &arg1)
{
     m_test_param_ch1.target_measure_count = arg1.toInt(0,10);

     emit measure_setup_ch1(m_test_param_ch1);
}

void MainWindow::on_sec_startdelay_ch1_valueChanged(const QString &arg1)
{
     m_test_param_ch1.work_on_time = arg1.toInt(0,10)*1000;

     emit measure_setup_ch1(m_test_param_ch1);
}

void MainWindow::on_sec_detoffdelay_ch1_valueChanged(const QString &arg1)
{
     m_test_param_ch1.detect_off_time = arg1.toInt(0,10)*1000;

     emit measure_setup_ch1(m_test_param_ch1);
}

void MainWindow::on_sec_interval_ch1_valueChanged(const QString &arg1)
{
     m_test_param_ch1.test_interval_time = arg1.toInt(0,10)*1000;

     emit measure_setup_ch1(m_test_param_ch1);
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

void MainWindow::on_meter_type_ch2_activated(const QString &arg1)
{

}

void MainWindow::on_download_ch2_clicked()
{

}

void MainWindow::on_measure_count_ch2_valueChanged(const QString &arg1)
{
     m_test_param_ch2.target_measure_count = arg1.toInt(0,10);

     emit measure_setup_ch2(m_test_param_ch2);
}

void MainWindow::on_sec_startdelay_ch2_valueChanged(const QString &arg1)
{
     m_test_param_ch2.work_on_time = arg1.toInt(0,10)*1000;

     emit measure_setup_ch2(m_test_param_ch2);
}

void MainWindow::on_sec_detoffdelay_ch2_valueChanged(const QString &arg1)
{
     m_test_param_ch2.detect_off_time = arg1.toInt(0,10)*1000;

     emit measure_setup_ch2(m_test_param_ch2);
}

void MainWindow::on_sec_interval_ch2_valueChanged(const QString &arg1)
{
     m_test_param_ch2.test_interval_time = arg1.toInt(0,10)*1000;

     emit measure_setup_ch2(m_test_param_ch2);
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

             memcpy(&m_test_param_ch1, &m_test_param_tmp, sizeof(m_test_param_tmp));

             ui->measure_count_ch1->setValue(static_cast<int>(m_test_param_ch1.target_measure_count));
             ui->sec_startdelay_ch1->setValue(static_cast<int>(m_test_param_ch1.work_on_time/1000));
             ui->sec_detoffdelay_ch1->setValue(static_cast<int>(m_test_param_ch1.detect_off_time/1000));
             ui->sec_interval_ch1->setValue(static_cast<int>(m_test_param_ch1.test_interval_time/1000));

             emit measure_setup_ch1(m_test_param_ch1);

        }
       else if(sender() == ui->meter_type_ch2)
        {
            memcpy(&m_test_param_ch2, &m_test_param_tmp, sizeof(m_test_param_tmp));

            ui->measure_count_ch2->setValue(static_cast<int>(m_test_param_ch2.target_measure_count));
            ui->sec_startdelay_ch2->setValue(static_cast<int>(m_test_param_ch2.work_on_time/1000));
            ui->sec_detoffdelay_ch2->setValue(static_cast<int>(m_test_param_ch2.detect_off_time/1000));
            ui->sec_interval_ch2->setValue(static_cast<int>(m_test_param_ch2.test_interval_time/1000));

            emit measure_setup_ch2(m_test_param_ch2);
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

    //Print local machine's IP address
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
            }
        }
    }

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

MainWindow::~MainWindow()
{
      delete m_control;
      delete ui;
}

void MainWindow::on_reboot_clicked()
{

    QProcess process;
    process.startDetached("sudo reboot");
}

void MainWindow::on_quit_clicked()
{
     QProcess process;
     process.startDetached("sudo poweroff");
}
