
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
    measure_port_reset();

    serialComm = Q_NULLPTR;

    timer_sec = new QTimer(this);       //display current time
    detect_on_timer = new QTimer(this);
    work_on_timer = new QTimer(this);
    third_on_timer = new QTimer(this);
    detect_off_timer = new QTimer(this);
    port_reset_timer = new QTimer(this);
    hub_port_delay_timer = new QTimer(this);
    comm_polling_timer = new QTimer(this);

    detect_on_timer->setSingleShot(true);
    work_on_timer->setSingleShot(true);
    third_on_timer->setSingleShot(true);
    detect_off_timer->setSingleShot(true);
    port_reset_timer->setSingleShot(true);
    hub_port_delay_timer->setSingleShot(true);

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

    QObject::connect(timer_sec, SIGNAL(timeout()), this, SLOT(UpdateTime()));
    timer_sec->start(1000);

    connect(comm_polling_timer, SIGNAL(timeout()), this, SLOT(comm_polling_event()));

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

    if(ui->ketone_sel->isChecked())
    {
        detect_off_time = 11000;        //case of KETONE
    }

    //Glucose : detect -> (5 sec) -> work_on -> (1 sec) -> third on -> (8 sec) -> detect off -> (4 sec)
    //KETONE :  detect -> (5 sec) -> work_on -> (1 sec) -> third on -> (10 sec)-> detect off -> (4 sec)

    detect_on_timer->setInterval(detect_on_time);
    work_on_timer->setInterval(work_on_time);
    third_on_timer->setInterval(third_on_time);
    detect_off_timer->setInterval(detect_off_time);
    port_reset_timer->setInterval(port_reset_time + changed_interval + bluetooth_time);    

    connect(this, SIGNAL(measure_start()), this, SLOT(measurement()));
    connect(this, SIGNAL(measure_cnt_check()), this, SLOT(measure_count_check()));
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
     cout<<"measure start"<<endl;

        mesure_time_check->start();

       detect_on_timer->start();
//     work_on_timer->start();
//     third_on_timer->start();
//     detect_off_timer->start();
//     port_reset_timer->start();
//     mesure_time_check->start();
}

void MainWindow::detect_on()
{
    cout<<"detect on : "<< mesure_time_check->elapsed()<<endl;

    if(ui->ketone_sel->isChecked())
    {
        measure_relay->measure_port_control(measure_relay->relay_channel::CH_4, DDL_CH_ON);
    }

//  ui->status->setText("\n\n action : detect on");
    ui->test_step->setText("Action : detect on");
    measure_relay->measure_port_control(measure_relay->relay_channel::CH_1, DDL_CH_ON);

    work_on_timer->start();
}

void MainWindow::work_on()
{
    cout<<"work on : "<< mesure_time_check->elapsed()<<"msec"<<endl;
//    ui->status->setText("\n\n action : work on");
     ui->test_step->setText("Action : work on");
    measure_relay->measure_port_control(measure_relay->relay_channel::CH_2, DDL_CH_ON);

    third_on_timer->start();

}

void MainWindow::third_on()
{
    cout<<"third on : "<< mesure_time_check->elapsed()<<"msec"<<endl;
//    ui->status->setText("\n\n action : third on");
    ui->test_step->setText("Action : third on");
    measure_relay->measure_port_control(measure_relay->relay_channel::CH_3, DDL_CH_ON);

    detect_off_timer->start();
}

void MainWindow::detect_off()
{
    cout<<"detect off : "<< mesure_time_check->elapsed()<<"msec"<<endl;
//  ui->status->setText("\n\n action : detect off");
    ui->test_step->setText("Action : detect off");
    measure_relay->measure_port_control(measure_relay->relay_channel::CH_1, DDL_CH_OFF);

    port_reset_timer->start();

}

void MainWindow::measure_port_reset()
{
    cout<<"measure port reset : "<< mesure_time_check->elapsed() <<"msec"<<endl;
//  ui->status->setText("\n\n action : port reset");
    ui->test_step->setText("Action : port reset");
    measure_relay->measure_port_reset();

    emit measure_cnt_check();
}

void MainWindow:: measure_count_check()
{
      cout<<"measure count check : "<< mesure_time_check->elapsed() <<"msec"<<endl;

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

void MainWindow::hub_port_open()
{
    hub_port_reset();
}

void MainWindow::hub_port_close()
{
    system("uhubctl -a off -p 2-5");
}

void MainWindow::hub_port_reset()
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

    hub_port_open();

    hub_port_delay_timer->setInterval(hub_port_delay_time);
    connect(hub_port_delay_timer, SIGNAL(timeout()), this, SLOT(meter_comm_start()));
    hub_port_delay_timer->start();
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

void MainWindow::meter_comm_start()
{

     hub_port_delay_timer->stop();

    if(serialComm == Q_NULLPTR)
    {
        serialComm = new SerialComm;

        connect(serialComm, SIGNAL(portReady()), this, SLOT(portReady()));
        connect(serialComm, SIGNAL(connectionError()), this, SLOT(connectionError()));
        connect(serialComm, SIGNAL(textMessageSignal(QString)), this, SLOT(textMessage(QString)));
        connect(serialComm, SIGNAL(maintainConnection(bool)), this, SLOT(maintainConnection(bool)));

        serialComm->open();
    }
}

void MainWindow::meter_comm_end()
{
    comm_polling_event_stop();

    serialComm->close();

    disconnect(serialComm, SIGNAL(portReady()), this, SLOT(portReady()));
    disconnect(serialComm, SIGNAL(connectionError()), this, SLOT(connectionError()));
    disconnect(serialComm, SIGNAL(textMessageSignal(QString)), this, SLOT(textMessage(QString)));
    disconnect(serialComm, SIGNAL(maintainConnection(bool)), this, SLOT(maintainConnection(bool)));
}

void MainWindow::comm_polling_event()
{
    bool result;

    if(serialComm != Q_NULLPTR)
    {
        Log() << "serialComm check!";
        serialComm->check();
    }
    else
    {
        serialComm = new SerialComm;

        connect(serialComm, SIGNAL(portReady()), this, SLOT(portReady()));
        connect(serialComm, SIGNAL(connectionError()), this, SLOT(connectionError()));
        connect(serialComm, SIGNAL(textMessageSignal(QString)), this, SLOT(textMessage(QString)));
        connect(serialComm, SIGNAL(maintainConnection(bool)), this, SLOT(maintainConnection(bool)));

        serialComm->open();
    }
}

void MainWindow::comm_polling_event_start()          //periodical polling for device
{
    comm_polling_event_stop();
    comm_polling_timer->start(5000);                //5 Sec
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
            Log() << "protocol state" << protocol->state();
#if 0
            connect(protocol, SIGNAL(timeoutError(Sp::ProtocolCommand)), this, SLOT(timeoutError(Sp::ProtocolCommand)));
            connect(protocol, SIGNAL(errorOccurred(Sp::ProtocolCommand,Sp::ProtocolCommand)), this, SLOT(errorOccurred(Sp::ProtocolCommand,Sp::ProtocolCommand)));
            connect(protocol, SIGNAL(errorCrc()), this, SLOT(errorCrc()));
            connect(protocol, SIGNAL(errorUnresolvedCommand(Sp::ProtocolCommand)), this, SLOT(errorUnresolvedCommand(Sp::ProtocolCommand)));
            connect(protocol, SIGNAL(packetReceived()), this, SLOT(packetReceived()));
            connect(protocol, SIGNAL(downloadComplete(QJsonArray*)), this, SLOT(downloadComplete(QJsonArray*)));
            connect(protocol, SIGNAL(needReopenSerialComm()), this, SLOT(needReopenSerialComm()));
            connect(protocol, SIGNAL(finishReadingQcData()), this, SLOT(finishReadingQcData()));
            connect(protocol, SIGNAL(finishDoCommands(bool, Sp::ProtocolCommand )), this, SLOT(finishDoCommands(bool,Sp::ProtocolCommand)));
#endif
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
        cout<<"connection success"<<endl;
    }
    else
    {
        cout<<"connection fail"<<endl;
    }
}

void MainWindow::connectionError()
{
    Log();
    if(serialComm)
        delete serialComm;

    serialComm = Q_NULLPTR;
}

void MainWindow::textMessage(QString text)
{
    Log() << text;
}

// 다운로드 과정에서 protocol에서 보내오는 시그널과 연결되는 함수
void MainWindow::timeoutError(Sp::ProtocolCommand command)
{

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
#if 0
    //#if defined(LOOP_FOR_EMC_TEST) || defined(LOOP_FOR_XP_CRADLE)

    if(m_qcType != QC_TYPE_DEL) {
        ClearListLog();
        InsertListLog("처리 중... 잠시만 기다려 주세요.");
    }
//#endif
#endif
    protocol->doCommands(list);
}
