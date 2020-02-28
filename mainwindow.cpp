
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "relay_waveshare.h"
#include "relay_seed_ddl.h"
#include "relay_seed.h"
#include "usb_hid_comm.h"
#include "bgm_comm_protocol.h"
#include "settings.h"
#include "setting_flagname_definition.h"
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
#include <QtNetwork/QNetworkInterface>

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

    ui->test_start->setEnabled(false);
    ui->device_open->setEnabled(false);
    ui->device_close->setEnabled(false);
    ui->test_stop->setEnabled(true);

    ui->bluetooth_sel->setEnabled(false);
    ui->times->setEnabled(false);
    ui->sec->setEnabled(false);

    ui->test_start_time->setText("Test start :" + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));

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

    ui->test_step->setText("Action : detect on");
    measure_relay->measure_port_control(measure_relay->relay_channel::CH_1, DDL_CH_ON);

    work_on_timer->start();
}

void MainWindow::work_on()
{
    cout<<"work on : "<< mesure_time_check->elapsed()<<"msec"<<endl;
    ui->test_step->setText("Action : work on");
    measure_relay->measure_port_control(measure_relay->relay_channel::CH_2, DDL_CH_ON);

    third_on_timer->start();
}

void MainWindow::third_on()
{
    cout<<"third on : "<< mesure_time_check->elapsed()<<"msec"<<endl;

    ui->test_step->setText("Action : third on");
    measure_relay->measure_port_control(measure_relay->relay_channel::CH_3, DDL_CH_ON);

    detect_off_timer->start();
}

void MainWindow::detect_off()
{
    cout<<"detect off : "<< mesure_time_check->elapsed()<<"msec"<<endl;

    ui->test_step->setText("Action : detect off");
    measure_relay->measure_port_control(measure_relay->relay_channel::CH_1, DDL_CH_OFF);

    port_reset_timer->start();

}

void MainWindow::measure_port_reset()
{
    cout<<"measure port reset : "<< mesure_time_check->elapsed() <<"msec"<<endl;

    ui->test_step->setText("Action : port reset");
    measure_relay->measure_port_reset();

    emit measure_cnt_check();
}

void MainWindow:: measure_count_check()
{
      cout<<"measure count check : "<< mesure_time_check->elapsed() <<"msec"<<endl;

      measure_coount++;

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

    meter_comm_end();

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
      ui->system_time->setText("Time: " + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss ap"));
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
    serialComm->close();
    serialComm = Q_NULLPTR;

    ui->meter_info->clear();

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
            Log() << "protocol state" << protocol->state();

            connect(protocol, SIGNAL(timeoutError(Sp::ProtocolCommand)), this, SLOT(timeoutError(Sp::ProtocolCommand)));
            connect(protocol, SIGNAL(errorOccurred(Sp::ProtocolCommand,Sp::ProtocolCommand)), this, SLOT(errorOccurred(Sp::ProtocolCommand,Sp::ProtocolCommand)));
            connect(protocol, SIGNAL(errorCrc()), this, SLOT(errorCrc()));
            connect(protocol, SIGNAL(errorUnresolvedCommand(Sp::ProtocolCommand)), this, SLOT(errorUnresolvedCommand(Sp::ProtocolCommand)));
            connect(protocol, SIGNAL(packetReceived()), this, SLOT(packetReceived()));
//          connect(protocol, SIGNAL(downloadComplete(QJsonArray*)), this, SLOT(downloadComplete(QJsonArray*)));
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
}

void MainWindow::textMessage(QString text)
{
    Log() << text;
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
    if(preCommand == Sp::Unlock) {
        QList<Sp::ProtocolCommand> list;
        list.append(Sp::ReadSerialNumber);
        doCommands(list);
        return;
    }
}

void MainWindow::errorCrc()                                    // 수신데이터 CRC 오류
{
    Log();
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
    Log();
    if(bSuccess == true)
    {
        Log() << "성공";
#if defined(LOOP_FOR_EMC_TEST) || defined(LOOP_FOR_XP_CRADLE)
        ClearListLog();
#endif

        if(lastcommand == Sp::ReadTemperature)
        {
            comm_polling_event_start();
            Log() << "read temperature";
            QString temperaturestr = "현재 미터의 온도: " + Settings::Instance()->getTemperature() + "℃";
            InsertListLog(temperaturestr);
        }
        else if(lastcommand == Sp::Unlock)
        {
            QList<Sp::ProtocolCommand> list;
            list.append(Sp::ReadSerialNumber);
            doCommands(list);
        }
        else
        {
             comm_polling_event_start();

            if(lastcommand == Sp::ReadSerialNumber)
            {
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

                Settings::Instance()->setBleName(INVALID_BLE);
                Log() << "blename = " << Settings::Instance()->getBleName();

                if(checkProtocol() != true)
                    return;

//              int currentIndex = ui->comboBox_meter_type->currentIndex();
                int currentIndex = 1;

                if(currentIndex == 2 || currentIndex == 3) //SMART, N IoT
                {
                    if(checkProtocol() != true)
                        return;
                    QList<Sp::ProtocolCommand> list;
                    list.append(Sp::ReadAESKey);
                    if (currentIndex == 3)  //N IoT
                    {
                        list.append(Sp::ReadServerAddress);
                        list.append(Sp::ReadDeviceToken);
                        list.append(Sp::ReadServiceName);
                        list.append(Sp::ReadIMEI);
                        list.append(Sp::ReadICCID);
                        list.append(Sp::ReadModuleSoftwareVer);
                        list.append(Sp::ReadIMSI);
                        list.append(Sp::ReadSktSN);
                        list.append(Sp::ReadRegistrationURL);
                    }
                    else  //SMART..
                    {
                        list.append(Sp::ReadAPN);
                    }
                    doCommands(list);
                    return;
                }
                else
                {
                    if(currentIndex == 0)  //TI
                    {
                        if(checkProtocol() != true)
                            return;

                        if(isOtgModeVisible())
                        {
                            if(checkProtocol() != true)
                                return;
                            QList<Sp::ProtocolCommand> list;
                            list.append(Sp::ReadOtgMode);
                            protocol->doCommands(list);
                            return;
                        }
                    }
                    else if(currentIndex == 1)  //ST
                    {
                        if(checkProtocol() != true)
                            return;

                        QList<Sp::ProtocolCommand> list;
                        list.append(Sp::ReadBLE);
                        protocol->doCommands(list);
                        return;
                    }
                    else if(currentIndex == 4)  //VetMate
                    {
                        if(checkProtocol() != true)
                            return;

                        QList<Sp::ProtocolCommand> list;
                        list.append(Sp::ReadAnimalType);
                        protocol->doCommands(list);
                        return;
                    }
                }

            }
            else if(lastcommand == Sp::WriteTimeInformation)
            {
                InsertListLog("시간 동기화 완료!");
                //Read Meter Time
                if(checkProtocol() != true)
                    return;
                QList<Sp::ProtocolCommand> list;
                list.append(Sp::ReadTimeInformation);
                protocol->doCommands(list);
                return;
            }
            else if(lastcommand == Sp::DeleteData)
            {
                if(m_qcType == QC_TYPE_DEL) {
                    InsertListLog("All data in the meter is deleted.");
                } else {
                    InsertListLog("메모리 삭제 완료!");
                }
            }
            else if(lastcommand == Sp::SetHour12H)
            {
                InsertListLog("12H로 설정 완료!");
            }
            else if(lastcommand == Sp::SetHour24H)
            {
                InsertListLog("24H로 설정 완료!");
            }
            else if(lastcommand == Sp::SetReset)
            {
                return;
            }
            else if(lastcommand == Sp::SetJIGMode)
            {
                ClearListLog();
                comm_polling_event_start();
                //ui->centralWidget->setEnabled(true);
                if(m_qcType == QC_TYPE_DEL) {
                    InsertListStateLog(0,"A meter is NOT connected.");
                } else {
                    InsertListStateLog(0,"기기 연결이 끊어짐 (1)");
                }

                return;
            }
            else if(lastcommand == Sp::SetBLELog)
            {
                ClearListLog();
                 comm_polling_event_start();
                //ui->centralWidget->setEnabled(true);
                if(m_qcType == QC_TYPE_DEL) {
                    InsertListStateLog(0,"A meter is NOT connected.");
                } else {
                    InsertListStateLog(0,"기기 연결이 끊어짐 (1)");
                }

                return;
            }
            else if(lastcommand == Sp::ReadBLE)
            {
                QString blename = Settings::Instance()->getBleName();

                if(blename == INVALID_BLE)
                {
                    //ui->lineEdit_blename->setText("");
                    EnableBLEControls(false);
                }
                else
                {
                    //TI BLE는 자동 sn가 불가능함.
                    //ui->lineEdit_blename->setText(blename);
                    EnableBLEControls(true);
                    //BLE 설정은 가능하지만 사용하지 않음일 때는 비활성화한다.
#if 0
                    if(ui->comboBox_bleway->currentIndex() == 2)
                    {
                        isEnableBLE = false;
                    }

                    ui->checkBox_setautosn->setChecked(false);
#endif
                    Settings::Instance()->setAutoSN(0);
                }

                //Read Meter Time
                if(checkProtocol() != true)
                    return;
                QList<Sp::ProtocolCommand> list;
                list.append(Sp::ReadTimeInformation);
                protocol->doCommands(list);
                return;
            }
            else if(lastcommand == Sp::ReadTimeInformation)
            {
                ui->meter_info->append("meter time : " + Settings::Instance()->getMeterTime());

                if(Settings::Instance()->isSetdataflag() == true)
                {
                    if(checkProtocol() != true)
                        return;
                    QList<Sp::ProtocolCommand> list;
                    list.append(Sp::GetGlucoseDataFlag);
                    protocol->doCommands(list);
                    return;
                }

                if(Settings::Instance()->isSetAvgDays() == true)  //평균일수 읽기
                {
                    if(checkProtocol() != true)
                        return;
                    QList<Sp::ProtocolCommand> list;
                    list.append(Sp::GetAvgDays);
                    protocol->doCommands(list);
                    return;
                }

                if(isCaresensC())
                {
                    if(checkProtocol() != true)
                        return;
                    QList<Sp::ProtocolCommand> list;
                    list.append(Sp::ReadCodingMode);
                    protocol->doCommands(list);
                    return;
                }

                if(isOtgModeVisible())
                {
                    if(checkProtocol() != true)
                        return;
                    QList<Sp::ProtocolCommand> list;
                    list.append(Sp::ReadOtgMode);
                    protocol->doCommands(list);
                    return;
                }

                Log();

                //Read number of stored measured results
                if(checkProtocol() != true)
                    return;
                QList<Sp::ProtocolCommand> list;
                list.append(Sp::CurrentIndexOfGluecose);
                protocol->doCommands(list);
                return;

            }
            else if(lastcommand == Sp::GetGlucoseDataFlag)
            {
                if(Settings::Instance()->isSetAvgDays() == true)  //평균일수 읽기
                {
                    if(checkProtocol() != true)
                        return;
                    QList<Sp::ProtocolCommand> list;
                    list.append(Sp::GetAvgDays);
                    protocol->doCommands(list);
                    return;
                }
            }
            else if(lastcommand == Sp::GetAvgDays)
            {
                if(isCaresensC())
                {
                    if(checkProtocol() != true)
                        return;
                    QList<Sp::ProtocolCommand> list;
                    list.append(Sp::ReadCodingMode);
                    protocol->doCommands(list);
                    return;
                }

                if(isOtgModeVisible())
                {
                    if(checkProtocol() != true)
                        return;
                    QList<Sp::ProtocolCommand> list;
                    list.append(Sp::ReadOtgMode);
                    protocol->doCommands(list);
                    return;
                }
            }
            else if(lastcommand == Sp::ReadCodingMode)
            {
                if(isOtgModeVisible())
                {
                    if(checkProtocol() != true)
                        return;
                    QList<Sp::ProtocolCommand> list;
                    list.append(Sp::ReadOtgMode);
                    protocol->doCommands(list);
                    return;
                }
            }
            else if(lastcommand == Sp::WriteCodingMode)
            {
                if(checkProtocol() != true)
                    return;
                QList<Sp::ProtocolCommand> list;
                list.append(Sp::ReadCodingMode);
                protocol->doCommands(list);
                return;
            }
            else if(lastcommand == Sp::WriteOtgMode)
            {
                if(checkProtocol() != true)
                    return;
                QList<Sp::ProtocolCommand> list;
                list.append(Sp::ReadOtgMode);
                protocol->doCommands(list);
                return;
            }
            else if(lastcommand == Sp::ChangeBLEMode || lastcommand == Sp::ChangeBLEMode_EXT)
            {
                Log() << "end changeblemode,ext";
                //needReopenSerialComm();
                return;
            }
            else if(lastcommand == Sp::TryChangeBLEMode)
            {
                if(checkProtocol() != true)
                    return;
                QList<Sp::ProtocolCommand> list;
                list.append(Sp::ChangeBLEMode);
                protocol->doCommands(list);
                return;
            }
            else if(lastcommand == Sp::Set91)
            {
                ClearListLog();
                if(Settings::Instance()->getQCValue(SFD_q_output_cal) == "PASS")
                {
                    InsertListLog("OUTPUT CAL Success!!");
                }
                else
                {
                    InsertListLog("OUTPUT CAL Fail!!");
                }
            }
            else if(lastcommand == Sp::Set9A01 ||
                    lastcommand == Sp::Set9A02 ||
                    lastcommand == Sp::Set9A03)
            {
                ClearListLog();
                if(Settings::Instance()->getQCValue(SFD_q_output_verification) == "PASS")
                {
                    InsertListLog("Verification Success!!");
                }
                else
                {
                    InsertListLog("Verification Fail!!");
                }
            }
            else if(lastcommand == Sp::SetOnNet)
            {
                ClearListLog();
                InsertListLog("Network ON!!");
            }
            else if(lastcommand == Sp::SetOffNet)
            {
                ClearListLog();
                InsertListLog("Network OFF!!");
            }
            else if(lastcommand == Sp::ReadSetHypo)
            {
                ClearListLog();
                InsertListLog("SetHypo Value = " + Settings::Instance()->getQCValue(SFD_q_sethypo));
            }
            else if(lastcommand == Sp::SetGlucoseDataFlag)
            {
                ClearListLog();
                if(Settings::Instance()->getQCValue(SFD_q_setglucosedataflag) == "SUCCESS")
                {
                    InsertListLog("Verification Success!!");
                }
                else
                {
                    InsertListStateLog(0,"Verification 실패하여, Flag 값을 다시 읽습니다.");
                    if(checkProtocol() != true)
                        return;

                    QList<Sp::ProtocolCommand> list;
                    list.append(Sp::GetGlucoseDataFlag);
                    doCommands(list);
                    return;
                }
            }
            else if(lastcommand == Sp::SetAvgDays)
            {
                ClearListLog();
                if(Settings::Instance()->getQCValue(SFD_q_setavgdays_result) == "SUCCESS")
                {
                    InsertListLog("평균 일수 설정 성공!!");
                }
                else
                {
                    InsertListStateLog(0,"평균 일수 설정이 실패했습니다.");
//                    if(checkProtocol() != true)
//                        return;

//                    QList<Sp::ProtocolCommand> list;
//                    list.append(Sp::GetAvgDays);
//                    doCommands(list);
//                    return;
                }
            }
            else if(lastcommand == Sp::WriteRegistrationURL)
            {
                 ClearListLog();
                 InsertListLog("Registration URL 설정 성공!!");
            }
            else if(lastcommand == Sp::CurrentIndexOfGluecose)      //Display result number in the meter
            {
                 ui->meter_info->append("number of measured data is " + (QString::number(Settings::Instance()->getNumberofCurrentGlucoseData())));
            }
            else
            {
                Log() << "not yet";
            }

            if(checkProtocol() != true)
                return;

             protocol->readQcData();               //Not use this tool
        }

    }
    else
    {
        Log() << "성공";
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
//        finishReadingQcData_del();
        Log();
    }
    else
    {
        if(modelType == 2 || modelType == 3) //SMART, N IoT
        {
            Log();
//            finishReadingQcData_color();
        }
        else
        {
            Log();
//f         inishReadingQcData_default();
        }
    }

    comm_polling_event_start();
}

void MainWindow::needReopenSerialComm()
{
    Log();
    ClearListLog();
}
