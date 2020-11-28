
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "relay_waveshare.h"
#include "relay_seed_ddl.h"
#include "relay_seed.h"
#include "usb_hid_comm.h"
#include "bgm_comm_protocol.h"
#include "settings.h"
#include "setting_flagname_definition.h"
#include "builddatetime.h"
#include <stdlib.h>
#include <iostream>
#include <QTextEdit>
#include <QTextCursor>
#include <QThread>
#include <QDateTime>
#include <QTime>
#include <QApplication>
#include <QProcess>
#include <QRegExp>
#include <QtNetwork/QNetworkInterface>
#include "loggingcategories.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), measure_relay(new relay_seed_ddl)
{
    ui->setupUi(this);

    measure_port_init();

    measurement_timer_setup();

    measurement_ui_setup();

    system_info_setup();

    hub_port_close();
}

void MainWindow::measurement_timer_setup()
{
    serialComm = Q_NULLPTR;

    timer_sec = new QTimer(this);                   //display current time
    detect_on_timer = new QTimer(this);
    work_on_timer = new QTimer(this);
    third_on_timer = new QTimer(this);
    detect_off_timer = new QTimer(this);
//  port_reset_timer = new QTimer(this);
    test_interval_timer = new QTimer(this);
    hub_port_delay_timer = new QTimer(this);
    comm_polling_timer = new QTimer(this);

    detect_on_timer->setSingleShot(true);
    work_on_timer->setSingleShot(true);
    third_on_timer->setSingleShot(true);
    detect_off_timer->setSingleShot(true);
//  port_reset_timer->setSingleShot(true);
    test_interval_timer->setSingleShot(true);
    hub_port_delay_timer->setSingleShot(true);
}

void MainWindow::measurement_ui_setup()
{

    /*Play/Pause/Stop Button*/
    ui->test_start->setIcon(QIcon(":/images/play.png"));
    ui->test_start->setIconSize(QSize(45,45));
    ui->test_stop->setIcon(QIcon(":/images/stop.png"));
    ui->test_stop->setIconSize(QSize(45,45));
    ui->test_pause->setIcon(QIcon(":/images/pause.png"));
    ui->test_pause->setIconSize(QSize(45,45));

    ui->test_stop->setEnabled(false);
    ui->test_pause->setEnabled(false);
    ui->device_open->setEnabled(true);
    ui->device_close->setEnabled(false);

    /* Measurement Condition*/
    ui->sec_startdelay->setEnabled(true);
    ui->sec_detoffdelay->setEnabled(true);
    ui->sec_interval->setEnabled(true);

    /*Measuremet Start Delay*/
    ui->sec_startdelay->setRange(0.0, 180.0);
    ui->sec_startdelay->setSingleStep(1.0);
    ui->sec_startdelay->setValue((int)(work_on_time/1000));

    /*Detect Off Delay*/
    ui->sec_detoffdelay->setRange(0.0, 180.0);
    ui->sec_detoffdelay->setSingleStep(1.0);
    ui->sec_detoffdelay->setValue((int)(detect_off_time/1000));

    /*Test Interval*/
    ui->sec_interval->setRange(0.0, 180.0);
    ui->sec_interval->setSingleStep(1.0);
    ui->sec_interval->setValue((int)(test_interval_time/1000));

    /*Test capacity*/
    ui->times->setEnabled(true);
    ui->times->setRange(0.0, 5000.0);
    ui->times->setSingleStep(100.0);
//  ui->times->setSingleStep(1.0);
    ui->times->setValue(1000.0);
//  ui->times->setValue(2.0);

    target_measure_count_rest = 1000;

    /*USB Interface select*/
    ui->micro_usb->setEnabled(true);
    ui->micro_usb->setChecked(true);

    ui->phone_jack->setEnabled(true);
    ui->micro_usb->setChecked(false);

    ui->time_sync->setEnabled(false);
    ui->mem_delete->setEnabled(false);
    ui->download->setEnabled(false);

    //Meter type select
    ui->meter_type->addItem("GLUCOSE BASIC");
    ui->meter_type->addItem("GLUCOSE BLE");
    ui->meter_type->addItem("GLUCOSE VOICE");
    ui->meter_type->addItem("GLUCOSE VOICE BLE");
    ui->meter_type->addItem("KETONE BASIC");
    ui->meter_type->addItem("KETONE BLE");
    ui->meter_type->setCurrentIndex(0);

    currentMeterIndexChanged(0);

    ui->sec_startdelay->setValue((int)(work_on_time/1000));
    ui->sec_detoffdelay->setValue((int)(detect_off_time/1000));
    ui->sec_interval->setValue((int)(test_interval_time/1000));

    connect(ui->meter_type,SIGNAL(currentIndexChanged(int)), this, SLOT(currentMeterIndexChanged(int)));
    connect(comm_polling_timer, SIGNAL(timeout()), this, SLOT(comm_polling_event()));
}

void MainWindow::system_info_setup()
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

    QObject::connect(timer_sec, SIGNAL(timeout()), this, SLOT(UpdateTime()));
    timer_sec->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ui_set_measurement_start()
{
    ui->test_start->setEnabled(false);
    ui->device_open->setEnabled(false);
    ui->device_close->setEnabled(false);
    ui->test_stop->setEnabled(true);
    ui->test_pause->setEnabled(true);

    //Test option
    ui->meter_type->setEnabled(false);
    ui->times->setEnabled(false);
    ui->sec_startdelay->setEnabled(false);
    ui->sec_detoffdelay->setEnabled(false);
    ui->sec_interval->setEnabled(false);

    //Comm Interface option
    ui->micro_usb->setEnabled(false);
    ui->phone_jack->setEnabled(false);

    ui->test_start_time->setText("Test start :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));
    ui->test_count->setText("Test Count is " + (QString::number(current_measure_count)));
}

void MainWindow::ui_set_measurement_stop()
{
    ui->test_start->setEnabled(true);
    ui->test_stop->setEnabled(false);
    ui->test_pause->setEnabled(false);

    ui->meter_type->setEnabled(true);    
    ui->times->setEnabled(true);
    ui->sec_startdelay->setEnabled(true);
    ui->sec_detoffdelay->setEnabled(true);
    ui->sec_interval->setEnabled(true);

    ui->device_open->setEnabled(true);
    ui->micro_usb->setEnabled(true);
    ui->phone_jack->setEnabled(true);

    ui->test_step->setText("Action : stopped");
}

void MainWindow::ui_set_measurement_pause()
{
    ui->test_start->setEnabled(true);
    ui->test_stop->setEnabled(true);
    ui->test_pause->setEnabled(false);        

    ui->device_open->setEnabled(true);
    ui->micro_usb->setEnabled(true);
    ui->phone_jack->setEnabled(true);

    ui->test_step->setText("Action : pause");
}

void MainWindow::ui_set_comm_start()
{

}

void MainWindow::ui_set_comm_stop()
{

}

void MainWindow::on_test_start_clicked()
{    

    //Glucose : detect -> (3.5 sec) -> work/third on -> (7 sec) -> detect off -> (4 sec)

    detect_on_timer->setInterval(detect_on_time);
    work_on_timer->setInterval(work_on_time);
    third_on_timer->setInterval(third_on_time);
    detect_off_timer->setInterval(detect_off_time);

    //default teset interval            15 Sec
    test_interval_timer->setInterval(test_interval_time);

    connect(this, SIGNAL(measure_start()), this, SLOT(measurement()));
    connect(this, SIGNAL(measure_cnt_check(SIGNAL_SENDER)), this, SLOT(measure_count_check(SIGNAL_SENDER)));
    connect(this, SIGNAL(measure_end()), this, SLOT(on_test_stop_clicked()));

    connect(detect_on_timer, SIGNAL(timeout()),SLOT(detect_on()));
    connect(work_on_timer, SIGNAL(timeout()),SLOT(work_on()));
    connect(third_on_timer, SIGNAL(timeout()),SLOT(third_on()));
    connect(detect_off_timer, SIGNAL(timeout()),SLOT(detect_off()));
    connect(test_interval_timer, &QTimer::timeout,[=]()
        {
            emit measure_cnt_check(SIGNAL_FROM_MEASURE_DETECT_OFF);
        });


    ui_set_measurement_start();

    measure_test_active = true;

    target_measure_count_rest = target_measure_count;

    emit measure_start();
}

void MainWindow::on_test_stop_clicked()
{   
    disconnect(this, SIGNAL(measure_start()), this, SLOT(measurement()));
    disconnect(this, SIGNAL(measure_end()), this, SLOT(on_test_stop_clicked()));

    detect_on_timer->stop();
    work_on_timer->stop();
    third_on_timer->stop();
    detect_off_timer->stop();

    disconnect(detect_on_timer, SIGNAL(timeout()),this,SLOT(detect_on()));
    disconnect(work_on_timer, SIGNAL(timeout()),this,SLOT(work_on()));
    disconnect(third_on_timer, SIGNAL(timeout()),this,SLOT(third_on()));
    disconnect(detect_off_timer, SIGNAL(timeout()),this,SLOT(detect_off()));

    current_measure_count = 0;

    ui_set_measurement_stop();

    measure_port_init();

    target_test_cycle = 0;
    current_test_cycle = 0;
    measure_test_active = false;
}

void MainWindow::on_test_pause_clicked()
{
    detect_on_timer->stop();
    work_on_timer->stop();
    third_on_timer->stop();
    detect_off_timer->stop();
//  port_reset_timer->stop();
    ui_set_measurement_pause();
    measure_port_init();
}

void MainWindow::measure_port_init()
{   
    measure_relay->measure_port_reset();
}

void MainWindow::measurement()
{

    Log() << "measurement start";
    detect_on_timer->start();
}

void MainWindow::detect_on()
{    
    Log() << "detect on";

    ui->test_step->setText("Action : detect on");
    measure_relay->measure_port_control(measure_relay->relay_channel::CH_1, DDL_CH_ON);

    work_on_timer->start();
}

void MainWindow::work_on()
{    
    Log() << "work on";

    ui->test_step->setText("Action : work on");
    measure_relay->measure_port_control(measure_relay->relay_channel::CH_2, DDL_CH_ON);

    third_on_timer->start();
}

void MainWindow::third_on()
{
    Log() << "third on";

    ui->test_step->setText("Action : third on");

    detect_off_timer->start();
}

void MainWindow::detect_off()
{ 
    Log() << "detect off";

    ui->test_step->setText("Action : detect off");
    measure_relay->measure_port_control(measure_relay->relay_channel::CH_1, DDL_CH_OFF);
    measure_relay->measure_port_control(measure_relay->relay_channel::CH_2, DDL_CH_OFF);

    current_measure_count++;

    ui->test_count->setText("Test Count is " + (QString::number(current_measure_count)));

    Log() << "test interval";

    test_interval_timer->start();
}

void MainWindow::measure_port_reset()
{
    Log() << "measure port reset";

    ui->test_step->setText("Action : port reset");
    measure_relay->measure_port_reset();

    emit measure_cnt_check(SIGNAL_FROM_MEASURE_DETECT_OFF);
}

void MainWindow:: measure_count_check(MainWindow::SIGNAL_SENDER sig_orin)
{       

       Log() <<"SIGNAL_SENDER"<< sig_orin;

        switch(sig_orin)
        {
            case SIGNAL_FROM_MEASURE_DETECT_OFF:
            case SIGNAL_FROM_COMM_ERROR:

           Log() << "current_measure_count :" << current_measure_count;
           Log() << "target_measure_count :"  << target_measure_count;
           Log() << "target_measure_count_rest :"  << target_measure_count_rest;

                if(target_measure_count<=1000)
                {
                    //Target count is 1000                             //target count is under 1000
                   if((current_measure_count == meter_mem_capacity) || (current_measure_count == target_measure_count_rest))
                   {
                       meter_comm_measure_count_check_request = true;

                       emit on_device_open_clicked();
                   }
                   else if(current_measure_count < target_measure_count)
                   {
                       emit measure_start();
                   }
                }
                else    //over 1000
                {

                    quint8 th_current = current_measure_count/1000;
                    quint8 hund_current = (current_measure_count/100)%10;
                    quint8 tens_current = (current_measure_count/10)%10;
                    quint8 unit_current = current_measure_count%10;
                    quint8 hund_target = (target_measure_count/100)%10;
                    quint8 tens_target = (target_measure_count/10)%10;
                    quint8 unit_target = target_measure_count%10;

                    //target count is reached at 1000, 2000, 3000, 4000 .....                 //Targeet count reached at 100, 200, 300, ......
                    if((th_current>=1 && !hund_current && !tens_current && !unit_current) || (target_measure_count_rest<1000 && hund_current==hund_target && !tens_target && !unit_target))
                    {
                        Log()<<"on_device_open_clicked";
                        emit on_device_open_clicked();
                    }
                    else
                    {
                        emit measure_start();
                    }

                }

            break;

            case SIGNAL_FROM_FINISH_DO_COMMAND:

                Log() << "measure_count_read_from_meter :" << measure_count_read_from_meter;

                if(!GluecoseResultDataExpanded)
                {
                    if(target_measure_count<=1000)
                    {
                        //Target count is 1000                                       //target count is under 1000
                        if((meter_mem_capacity == measure_count_read_from_meter) || (target_measure_count_rest <= measure_count_read_from_meter))
                        {

                            if(ui->audo_download->isChecked())
                            {
                                Log() <<"protocol->startDownload";

                                meter_comm_measure_count_check_request = true;

                                //ToDo: Download saved data to Host system
                                if(checkProtocol() != true)
                                    return;
                                protocol->startDownload();
                            }
                            else
                            {
                                 GluecoseResultDataExpanded =true;
                                 emit measure_cnt_check(SIGNAL_FROM_FINISH_DO_COMMAND);
                            }
                        }
                        else
                        {
                            current_measure_count = measure_count_read_from_meter;

                            Log() <<"current_measure_count" << current_measure_count;

                            ui->test_count->setText("Test Count is " + (QString::number(current_measure_count)));

                            meter_comm_measure_count_check_request = false;

                            emit on_device_close_clicked();

                            QThread::msleep(1000);

                            emit measure_start();
                        }
                    }
                    else  //target count is over 1000
                    {                      
                       if((meter_mem_capacity == measure_count_read_from_meter) || (target_measure_count_rest == measure_count_read_from_meter))
                       {                          
                              if(ui->audo_download->isChecked())
                              {
                                   Log() <<"protocol->startDownload";

                                    //ToDo: Download saved data to Host system
                                   if(checkProtocol() != true)
                                       return;

                                   meter_comm_measure_count_check_request = true;

                                   protocol->startDownload();
                              }
                              else
                              {
                                   GluecoseResultDataExpanded =true;
                                   emit measure_cnt_check(SIGNAL_FROM_FINISH_DO_COMMAND);
                              }
                       }
                       else
                       {
                           if(current_test_cycle)
                               current_measure_count = current_test_cycle*meter_mem_capacity + measure_count_read_from_meter;
                           else
                               current_measure_count = measure_count_read_from_meter;

//                          Log() <<"current_measure_count" << current_measure_count;

                            ui->test_count->setText("Test Count is " + (QString::number(current_measure_count)));

                            meter_comm_measure_count_check_request = false;

                            emit on_device_close_clicked();

                            QThread::msleep(1000);

                            emit measure_start();
                       }
                    }
                }
                else        //measure data download is complete
                {

                    if(target_measure_count_rest < measure_count_read_from_meter)
                       target_measure_count_rest =0;
                     else
                        target_measure_count_rest -=measure_count_read_from_meter;

                   Log() <<"target_measure_count_rest" << target_measure_count_rest;

                    if(target_measure_count<=1000)
                    {
                       current_measure_count = measure_count_read_from_meter;
                    }
                    else
                    {
                        current_measure_count = target_measure_count-target_measure_count_rest;
                    }

                    Log() <<"current_measure_count" << current_measure_count;

                    ui->test_count->setText("Test Count is " + (QString::number(current_measure_count)));

                    if(!target_measure_count_rest)
                    {
                        Log()<< "measure End";

                        emit on_device_close_clicked();

                        QThread::msleep(1000);

                        emit measure_end();

                    }
                    else
                    {

                        Log()<< "measure Start Again";

                        current_test_cycle++;

                        on_mem_delete_clicked();

                        QThread::msleep(3000);

                        emit on_device_close_clicked();

                        QThread::msleep(1000);

                        emit measure_start();
                    }

                    meter_comm_measure_count_check_request = false;
                    GluecoseResultDataExpanded = false;
                }                

            break;

            default:
                break;
        }
}

void MainWindow::hub_port_open()
{
    hub_port_reset();
}

void MainWindow::hub_port_close()
{
    system("uhubctl -l 1-1 -p 2-3 -a off");         //RPi3B
//  system("uhubctl -l 1-1.1 -p 2-3 -a off");       //RPi3B+
}

void MainWindow::hub_port_reset()
{
    system("uhubctl -l 1-1 -p 2-3 -a off");
//  system("uhubctl -l 1-1.1 -p 2-3 -a off");

    QThread::msleep(500);
    system("uhubctl -l 1-1 -p 2-3 -a on");
//  system("uhubctl -l 1-1.1 -p 2-3 -a on");
}

void MainWindow::on_quit_clicked()
{
    QProcess process;
    process.startDetached("sudo poweroff");
}

void MainWindow::on_reboot_clicked()
{
    QProcess process;
    process.startDetached("sudo reboot");
}

void MainWindow::on_device_open_clicked()
{
    ui->device_open->setEnabled(false);

    /*USB Interface select*/
    ui->micro_usb->setEnabled(false);
    ui->phone_jack->setEnabled(false);

    //Measurement Option
    ui->test_start->setEnabled(false);
    ui->test_stop->setEnabled(false);

    ui->meter_type->setEnabled(false);
    ui->times->setEnabled(false);

    ui->sec_startdelay->setEnabled(false);
    ui->sec_detoffdelay->setEnabled(false);
    ui->sec_interval->setEnabled(false);

    if(ui->phone_jack->isChecked())
    {
        //Phone Jack type meter PC mode enable
        measure_relay->measure_port_control(measure_relay->relay_channel::CH_4, DDL_CH_ON);
    }

    hub_port_open();

    hub_port_delay_timer->setInterval(5000);

    connect(hub_port_delay_timer, SIGNAL(timeout()), this, SLOT(meter_comm_start()));

    hub_port_delay_timer->start();
}

void MainWindow::on_device_close_clicked()
{
    ui->device_close->setEnabled(false);
    ui->time_sync->setEnabled(false);
    ui->mem_delete->setEnabled(false);
    ui->download->setEnabled(false);

    /*USB Interface select*/
    ui->micro_usb->setEnabled(true);
    ui->phone_jack->setEnabled(true);

    meter_comm_end();

    if(ui->phone_jack->isChecked())
    {
        //Phone Jack type meter PC mode enable
        measure_relay->measure_port_control(measure_relay->relay_channel::CH_4, DDL_CH_OFF);
    }

//  system("uhubctl -a off -p 2-5");

    hub_port_close();

    QThread::msleep(1000);

    if(!measure_test_active)
    {
        ui->device_open->setEnabled(true);
        ui->test_start->setEnabled(true);
        ui->meter_type->setEnabled(true);
        ui->times->setEnabled(true);
        ui->sec_startdelay->setEnabled(true);
        ui->sec_detoffdelay->setEnabled(true);
        ui->sec_interval->setEnabled(true);
    }
    else
    {
        ui->test_stop->setEnabled(true);
    }
}

void MainWindow::on_times_valueChanged(const QString &arg1)
{
     target_measure_count = arg1.toInt(0,10);

     target_measure_count_rest = target_measure_count;

     target_test_cycle = target_measure_count/meter_mem_capacity;
}

void MainWindow::UpdateTime()
{
      ui->system_time->setText("Time: " + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));
}

void MainWindow::meter_comm_start()
{

     hub_port_delay_timer->stop();

    if(serialComm == Q_NULLPTR)
    {
       Log();

        serialComm = new SerialComm;

        connect(serialComm, SIGNAL(portReady()), this, SLOT(portReady()));
        connect(serialComm, SIGNAL(connectionError()), this, SLOT(connectionError()));
        connect(serialComm, SIGNAL(textMessageSignal(QString)), this, SLOT(textMessage(QString)));
        connect(serialComm, SIGNAL(maintainConnection(bool)), this, SLOT(maintainConnection(bool)));

         if(ui->micro_usb->isChecked())         
             isDeviceOpened = serialComm->open(serialComm->intercface::micro_usb);

         if(ui->phone_jack->isChecked())
             isDeviceOpened = serialComm->open(serialComm->intercface::phone_jack);

         if(!isDeviceOpened)
             emit connectionError();
    }
    else
    {
       Log();
    }
}

void MainWindow::meter_comm_end()
{

    if(isDeviceOpened)
        serialComm->close();

    serialComm = Q_NULLPTR;

    /*USB Interface select*/

    disconnect(serialComm, SIGNAL(portReady()), this, SLOT(portReady()));
    disconnect(serialComm, SIGNAL(connectionError()), this, SLOT(connectionError()));
    disconnect(serialComm, SIGNAL(textMessageSignal(QString)), this, SLOT(textMessage(QString)));
    disconnect(serialComm, SIGNAL(maintainConnection(bool)), this, SLOT(maintainConnection(bool)));

}

void MainWindow::comm_polling_event()
{    
    if(serialComm != Q_NULLPTR)
    {
        Log() << "serialComm check!";
        serialComm->check();
    }
}

void MainWindow::comm_polling_event_start()          //periodical polling for device
{   
    comm_polling_event_stop();
    comm_polling_timer->start(3000);                //3 Sec
}

void MainWindow::comm_polling_event_stop()
{
    if(comm_polling_timer->isActive())
        comm_polling_timer->stop();
}

void MainWindow::portReady()
{
    // 여기서 프로토콜을 연결한다.
    comm_polling_event_stop();
    //ui->centralWidget->setEnabled(false);
    QList<Sp::ProtocolCommand> list;

    switch(serialComm->protocol())
    {
        case Sp::CommProtocolUnknown:
        {
            break;
        }

        case Sp::CommProtocol1:
        {
            break;
        }

        case Sp::CommProtocol2:
        {
            break;
        }

        case Sp::CommProtocol3:
        {
            disconnect(serialComm, SIGNAL(portReady()), this, SLOT(portReady()));
            disconnect(serialComm, SIGNAL(connectionError()), this, SLOT(connectionError()));

            protocol = new SerialProtocol3(serialComm, this);
//          Log() << "protocol state" << protocol->state();

            connect(protocol, SIGNAL(timeoutError(Sp::ProtocolCommand)), this, SLOT(timeoutError(Sp::ProtocolCommand)));
            connect(protocol, SIGNAL(errorOccurred(Sp::ProtocolCommand,Sp::ProtocolCommand)), this, SLOT(errorOccurred(Sp::ProtocolCommand,Sp::ProtocolCommand)));
            connect(protocol, SIGNAL(errorCrc()), this, SLOT(errorCrc()));
            connect(protocol, SIGNAL(errorUnresolvedCommand(Sp::ProtocolCommand)), this, SLOT(errorUnresolvedCommand(Sp::ProtocolCommand)));
            connect(protocol, SIGNAL(packetReceived()), this, SLOT(packetReceived()));
            connect(protocol, SIGNAL(downloadComplete(QJsonArray*)), this, SLOT(downloadComplete(QJsonArray*)));
            connect(protocol, SIGNAL(needReopenSerialComm()), this, SLOT(needReopenSerialComm()));
            connect(protocol, SIGNAL(finishReadingQcData()), this, SLOT(finishReadingQcData()));
            connect(protocol, SIGNAL(finishDoCommands(bool, Sp::ProtocolCommand )), this, SLOT(finishDoCommands(bool,Sp::ProtocolCommand)));

            if(checkProtocol() != true)
                return;

            bAutoSetSN = true;
            //Settings::Instance()->setSerialNumber(MSG_NoSN,"");

            list.append(Sp::Unlock);
            doCommands(list);
            break;
        }
        default:
        {
            Q_ASSERT(0);
        }
    }
    //emit portReadySignal();
}

void MainWindow::maintainConnection(bool isOK)
{
    if(isOK == true)
    {
        Log();
        cout<<"connection success"<< endl;
    }
    else
    {
        cout<<"connection fail"<<endl;
    }
}

void MainWindow::connectionError()
{
    Log();

    if(meter_comm_measure_count_check_request)
    {
        if(comm_retry_count < 6)
        {
            Log();
            on_device_close_clicked();
//          port_reset_timer->start();
            comm_retry_count++;
        }
        else
        {
            Log();
            emit serialComm->textMessageSignal("Meter connection failed!!");
            comm_retry_count = 0;
        }
    }
    else
    {
        Log();
        on_device_close_clicked();
    }

}

void MainWindow::textMessage(QString text)
{
    ui->meter_info->clear();
    ui->meter_info->append(text);
}

// 다운로드 과정에서 protocol에서 보내오는 시그널과 연결되는 함수
void MainWindow::timeoutError(Sp::ProtocolCommand command)
{
    Q_UNUSED(command);
    Log() << "DownloadView::timeoutError" << command;

    if(command == Sp::GetGlucoseDataFlag)
        {
            Log() << "No Flag Settings" << command;
        }
        else if(command == Sp::Unlock)
        {
            QList<Sp::ProtocolCommand> list;
            list.append(Sp::ReadSerialNumber);
            doCommands(list);
        }
        else if(command == Sp::SetAvgDays || command == Sp::GetAvgDays)
        {
            Log() << "No Average Days settings" << command;
        }
        else
        {
            Log() << "needReopenSerialComm" << command;
//          needReopenSerialComm();
        }
}

void MainWindow::errorOccurred(Sp::ProtocolCommand command, Sp::ProtocolCommand preCommand)// 미터에서 보내온 오류
{
    Q_UNUSED(command);
    Log() << command;

    if(preCommand == Sp::Unlock)
    {
        QList<Sp::ProtocolCommand> list;
        list.append(Sp::ReadSerialNumber);
        doCommands(list);
        return;
    }
}

void MainWindow::errorCrc()                                    // 수신데이터 CRC 오류
{
    Log();

    if(meter_comm_measure_count_check_request)
    {
        if(comm_retry_count < 6)
        {
           Log();

            on_device_close_clicked();

//          port_reset_timer->start();
            comm_retry_count++;
        }
        else
        {
           emit serialComm->textMessageSignal("Meter CRC Fail!!");
           comm_retry_count = 0;
        }
    }
    else
    {
        on_device_close_clicked();
    }
}

void MainWindow::errorUnresolvedCommand(Sp::ProtocolCommand command)     // 다운로드 과정에서 수행하지 않는 커맨드 패킷 수신
{
    Q_UNUSED(command);
    Log() << command;
}

void MainWindow::packetReceived()
{
    Log();
}

void MainWindow::finishDoCommands(bool bSuccess, Sp::ProtocolCommand lastcommand)
{

    if(bSuccess == true)
    {
        Log();

        if(lastcommand == Sp::Unlock)
        {

          if(checkProtocol() != true)
            return;

          QList<Sp::ProtocolCommand> list;
          list.append(Sp::ReadSerialNumber);
          doCommands(list);
        }
        else
        {
           Log()<<"check last command";

            comm_polling_event_start();

            if(lastcommand == Sp::ReadSerialNumber)
            {
               Log()<<"ReadSerialNumber";

                ui->meter_info->clear();

                QString temp_sn = Settings::Instance()->getSerialNumber();

                if(temp_sn == "")
                {
                    Log() << "temp_sn = " <<MSG_NoSN;
                    ui->meter_info->append("Read Fail!!");
                }
                else
                {
                    ui->meter_info->append("Serial Number :" + temp_sn);
                }             

                if(checkProtocol() != true)
                    return;
                QList<Sp::ProtocolCommand> list;
                list.append(Sp::ReadTimeInformation);
                protocol->doCommands(list);
                return;
            }
            else if(lastcommand == Sp::ReadTimeInformation)
            {
               Log()<<"ReadTimeInformation";

                 ui->meter_info->append("meter time : " + Settings::Instance()->getMeterTime());

                 //Read number of stored measured results
                 if(checkProtocol() != true)
                     return;
                 QList<Sp::ProtocolCommand> list;
                 list.append(Sp::CurrentIndexOfGluecose);
                 protocol->doCommands(list);
                 return;

            }
            else if(lastcommand == Sp::CurrentIndexOfGluecose)
            {
               Log()<<"CurrentIndexOfGluecose";

                measure_count_read_from_meter = Settings::Instance()->getNumberofCurrentGlucoseData();

                Log()<<"number of measured data is"<< measure_count_read_from_meter;

                ui->meter_info->append("number of measured data is " + (QString::number(measure_count_read_from_meter)));
                ui->time_sync->setEnabled(true);
                ui->mem_delete->setEnabled(true);
                ui->download->setEnabled(true);
                ui->device_close->setEnabled(true);

                if(measure_test_active)
                    emit measure_cnt_check(SIGNAL_FROM_FINISH_DO_COMMAND);
            }
            else if(lastcommand == Sp::GluecoseResultDataTxExpanded)
            {
                Log()<<"GluecoseResultDataTxExpanded";

                 GluecoseResultDataExpanded = true;         //data download done

                 emit measure_cnt_check(SIGNAL_FROM_FINISH_DO_COMMAND);
            }
            else if((lastcommand == Sp::WriteTimeInformation) || (lastcommand == Sp::DeleteData))
            {
                 Log()<<"WriteTimeInformation || DeleteData" << lastcommand ;

                //Read Meter Time
                if(checkProtocol() != true)
                    return;
                QList<Sp::ProtocolCommand> list;
                list.append(Sp::ReadSerialNumber);
                protocol->doCommands(list);
                return;
            }
        }

       Log();
    }
    else
    {
       Log() << "Success";
    }


}


bool MainWindow::checkProtocol()
{
    if(protocol == Q_NULLPTR || serialComm == Q_NULLPTR || serialComm->isAvailable() != true)
    {
#if 0
        ClearListLog();

        if(m_qcType == QC_TYPE_DEL)
        {
            InsertListStateLog(0,"A meter is NOT connected.");
        }
        else
        {
            InsertListStateLog(0, "연결된 기기가 없습니다.");
        }
#endif
        return false;
    }

    comm_polling_event_stop();
    serialComm->unsetCheckState();

    return true;
}

void MainWindow::doCommands(QList<Sp::ProtocolCommand> list)
{

    //#if defined(LOOP_FOR_EMC_TEST) || defined(LOOP_FOR_XP_CRADLE)

    if(m_qcType != QC_TYPE_DEL) {
        ClearListLog();
        InsertListLog("processing, please wait");
    }

    protocol->doCommands(list);
}

void MainWindow::InsertListLog(QString str)
{

}

void MainWindow::InsertListStateLog(int state, QString str)
{

}

void MainWindow::ClearListLog()
{

}

bool MainWindow::isOtgModeVisible()
{
    //CareSens N Premier BLE – V89.110.x.x, CareSens N(N-ISO) – V39.200.x.x, CareSens N(N-ISO) Notch – V129.100.x.x
    QStringList fwstrings = m_settings.value(SFD_q_fw_version).toString().split(" ");
    if(fwstrings.length() > 2)
    {
        return (fwstrings[0] == "89" && fwstrings[1] == "110") || (fwstrings[0] == "39" && fwstrings[1] == "200") || (fwstrings[0] == "129" && fwstrings[1] == "100");
    }
    else
    {
        return false;
    }
}

void MainWindow::EnableBLEControls(bool value)
{
    isEnableBLE = value;
}

bool MainWindow::isCaresensC()
{
    QStringList fwstrings = m_settings.value(SFD_q_fw_version).toString().split(" ");
    if(fwstrings.length() > 2)
    {
        return fwstrings[0] == "176" && fwstrings[1] == "110";
    }
    else
    {
        return false;
    }
}

void MainWindow::finishReadingQcData()
{
    m_settings = Settings::Instance()->getSettings();
    ClearListLog();

    int modelType = m_settings.value(SFD_meter_type).toInt();

    if(m_qcType == QC_TYPE_DEL)
    {
//      finishReadingQcData_del();
        Log();
    }
    else
    {
        if(modelType == 2 || modelType == 3) //SMART, N IoT
        {
            Log();
//          finishReadingQcData_color();
        }
        else
        {
            Log();
//          finishReadingQcData_default();
        }
    }

    comm_polling_event_start();
}

void MainWindow::needReopenSerialComm()
{
    Log();
    ClearListLog();
}

void MainWindow::on_mem_delete_clicked()
{
    if(checkProtocol() != true)
        return;

    Log() << "on_mem_delete_clicked";

    QList<Sp::ProtocolCommand> list;
    list.append(Sp::DeleteData);
    protocol->doCommands(list);

    return;
}

void MainWindow::on_time_sync_clicked()
{  
    if(checkProtocol() != true)
        return;

   Log() << "on_time_sync_clicked";

    QList<Sp::ProtocolCommand> list;
    list.append(Sp::WriteTimeInformation);
    protocol->doCommands(list);
    return;
}


void MainWindow::on_download_clicked()
{
    if(checkProtocol() != true)
        return;

    Log() << "on_download_clicked";

    protocol->startDownload();

}

void MainWindow::downloadProgress(float progress)
{
    Q_UNUSED(progress);
    Log()<< "download progress :" <<progress;
}

QString MainWindow::parseTime(QString timevalue)
{
    QDateTime datevalue = QDateTime::fromMSecsSinceEpoch(timevalue.toLongLong());

    return datevalue.toString("yyyy/MM/dd AP h:mm");

}

static unsigned long loop_count = 0;
void MainWindow::makeDownloadCompleteView(QJsonArray datalist)
{
    //ClearListLog();

    QString sn = "Serial Number: ";

    if(datalist.count() > 1)
    {
        sn += datalist[0].toObject()["sn"].toString();
    }

    int glucosedatacount = datalist.count()-1; // except sn

    InsertListLog(sn);
    InsertListLog(QString().sprintf("Count of downloaded data: %d", glucosedatacount ));

    for(int i = 1; i < glucosedatacount+1; i++)
    {
       QString timeString= "", datastring = "";
       timeString = parseTime(datalist[i].toObject()["dDate"].toString());

       datastring += QString().sprintf("[%03d] ", i)+ timeString + ", value(";

       datastring += datalist[i].toObject()["glucose_data"].toString();

       datastring = datastring + "), HiLo(" + datalist[i].toObject()["flag_hilo"].toString() +
                        + "), Ketone(" + datalist[i].toObject()["flag_ketone"].toString() +")";
        InsertListLog(datastring);

       Log() <<datastring;
    }

    QString currenttimestring = QDateTime::currentDateTime().toString("yyyy/MM/dd AP h:mm:ss");
    QString doneString = QString().sprintf("[%ld] ", loop_count) + "Download succeeded at " + currenttimestring;
    InsertListStateLog(m_logcount, doneString);
    m_logcount++;

    comm_polling_event_start();
}

void MainWindow::downloadComplete(QJsonArray* datalist)
{
     Log() << "downloadComplete";

     SaveCSVFile(*datalist);

     emit finishDoCommands(true, Sp::GluecoseResultDataTxExpanded);
}

void MainWindow::SaveCSVFile(QJsonArray datalist)
{

    QString folderpath = ("/home/pi/raw_data/");
    QString working_date = QDateTime::currentDateTime().toString("yyyyMMdd");

    QDir fdir(folderpath);

    if(fdir.exists() == false)
    {
        bool result = fdir.mkdir(folderpath);
        Log() << " directory = " << result;
    }

    QDir fdir_date(folderpath + working_date);

    if(fdir_date.exists() == false)
    {
        bool result = fdir_date.mkdir(folderpath + working_date + "/");
        Log() << " directory = " << result;
    }

    QString filepath, meter_sn;

    meter_sn = Settings::Instance()->getSerialNumber().remove(" ");

    filepath = folderpath + working_date + "/" + meter_sn + "_" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss")+ ".csv";

    SaveCSVFile_default(filepath, datalist);
}

void MainWindow::SaveCSVFile_default(QString filepath, QJsonArray datalist)
{
    int i = 0;
    quint32 list_del;

    QFile outputFile(filepath);

    //creat a CSV file with base header
    if(outputFile.exists() != true)
    {
        QStringList tableheaderlist, meter_info_list, measure_result_list;

        //first row
        tableheaderlist << "name" << " " << "birthday" << "sex" << "serial number" << "data unit" << "user idx" << "Insurance Number" << "date_format" <<"email" << "version";

        if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            Log() << "Open file error for csv header";
            return ;
        }

        QTextStream outstream(&outputFile);

        for( i = 0 ; i < tableheaderlist.count(); i++)
        {
            outstream << tableheaderlist[i] << ",";
        }

         outstream << "\n";

        //second row
         meter_info_list << Settings::Instance()->getSerialNumber().remove(" ") << " " << QDateTime::currentDateTime().toString("yyyy-MM-dd") << "M" << Settings::Instance()->getSerialNumber().remove(" ") << "mg/dL" << " " << " " << "YYYY/MM/DD" << " " << "2.49";

         for( i = 0 ; i < meter_info_list.count(); i++)
         {
             outstream << meter_info_list[i] << ",";
         }

         outstream << "\n";

         //third row
         measure_result_list << "time" << "org_glucose value(mg/dL)" << "glucose value(mg/dL)" << "manual" << "cs" << "memo" << "exercise" << "meal" << "insulin_type" << "insulin_amount";
         measure_result_list << "flag_hilo" << "flag_fasting" << "flag_nomark" << "flag_ketone" << "flag_ext" << "cloud" << "flag_meal" << "exercise_name" << "exercise_unit" << "meal_unit";

         for( i = 0 ; i < measure_result_list.count(); i++)
         {
             outstream << measure_result_list[i] << ",";
         }

         outstream << "\n";

         outputFile.close();

         QFile contentFile(filepath);

        if (!contentFile.open(QIODevice::Append))
        {
           Log() << "Open file error for csv header";

           return;
        }
    }

    QFile contentFile(filepath);

    if (!contentFile.open(QIODevice::Append))
    {
       Log() << "Open file error for csv header";
        return;
    }

    //append measured results
    quint32 glucosedatacount;

    if(datalist.count())
         glucosedatacount = datalist.count()-1;
    else
        return;

    QTextStream contentstream(&contentFile);

    for(quint32 result_count=1; result_count<glucosedatacount; result_count++)
    {
         contentstream << datalist[result_count].toObject()["time"].toString("yyyy-MM-dd hh-mm-ss")<< ",";
         contentstream << datalist[result_count].toObject()["glucose_value"].toString()<< ",";
         contentstream << datalist[result_count].toObject()["glucose_value"].toString()<< ",";
         contentstream << datalist[result_count].toObject()["manual"].toString().remove(" ") << ",";
         contentstream << datalist[result_count].toObject()["cs"].toString()<< ",";
         contentstream << datalist[result_count].toObject()["memo"].toString()<< ",";
         contentstream << datalist[result_count].toObject()["exercise"].toString()<< ",";
         contentstream << datalist[result_count].toObject()["meal"].toString()<< ",";
         contentstream << datalist[result_count].toObject()["insulin_type"].toString()<< ",";
         contentstream << datalist[result_count].toObject()["insulin_amount"].toString()<< ",";
         contentstream << datalist[result_count].toObject()["flag_hilo"].toString()<< ",";
         contentstream << datalist[result_count].toObject()["flag_fasting"].toString()<< ",";
         contentstream << datalist[result_count].toObject()["flag_nomark"].toString()<< ",";
         contentstream << datalist[result_count].toObject()["flag_ketone"].toString()<<"\n";
    }

    contentFile.close();

    Log();
}

void MainWindow::currentMeterIndexChanged(int index)
{
    switch (index)
    {
        case GLUCOSE_BASIC:

            Log()<<"GLUCOSE selected";

            work_on_time = 3000;
            third_on_time =0; // 6000;
            detect_off_time = 8000; //14000;
            test_interval_time = 15000;

            break;

        case GLUCOSE_BLE:

            Log()<<"GLUCOSE BLE selected";

            work_on_time = 3000;
            third_on_time =0; // 6000;
            detect_off_time = 8000; //14000;
            test_interval_time = 25000;

            break;

        case GLUCOSE_VOICE:

            Log()<<"GLUCOSE VOICE selected";

            work_on_time = 7000;
            third_on_time =0; // 6000;
            detect_off_time = 11000; //14000;
            test_interval_time = 15000;

            break;

        case GLUCOSE_VOICE_BLE :

            Log()<<"GLUCOSE VOICE BLE selected";

            work_on_time = 7000;
            third_on_time =0; // 6000;
            detect_off_time = 11000; //14000;
            test_interval_time = 25000;

            break;

        case KETONE_BASIC:

            Log()<<"KETONE selected";

            break;

        case KETONE_BLE:

            Log()<<"KETONE BLE selected";

            break;

        default:
        break;
    }


    /*Set delay time on spin box*/
    ui->sec_startdelay->setValue((int)(work_on_time/1000));
    ui->sec_detoffdelay->setValue((int)(detect_off_time/1000));
    ui->sec_interval->setValue((int)(test_interval_time/1000));
}

string MainWindow::do_console_command_get_result (char* command)
{
    FILE* pipe = popen(command, "r");		//Send the command, popen exits immediately
        if (!pipe)
            return "ERROR";

        char buffer[128];
        string result = "";
        while(!feof(pipe))						//Wait for the output resulting from the command
        {
            if(fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
        pclose(pipe);
        return(result);
}

void MainWindow::on_sec_startdelay_valueChanged(const QString &arg1)
{
     work_on_time = arg1.toInt(0,10)*1000;
}

void MainWindow::on_sec_detoffdelay_valueChanged(const QString &arg1)
{
    detect_off_time = arg1.toInt(0,10)*1000;
}

void MainWindow::on_sec_interval_valueChanged(const QString &arg1)
{
    test_interval_time = arg1.toInt(0,10)*1000;
}

void MainWindow::on_checkBox_stateChanged(int arg1)
{

}
