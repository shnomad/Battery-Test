#include "serialprotocol3.h"
#include <QTimer>
#include "settings.h"
#include "setting_flagname_definition.h"

#define timerStop() \
    {if(timeoutTimerID) { killTimer(timeoutTimerID); timeoutTimerID = 0; }}
#define timerStart() \
    { if(timeoutTimerID) { killTimer(timeoutTimerID); timeoutTimerID = 0; }\
    timeoutTimerID = startTimer(kProtocol3TimeoutDuration);}

SerialProtocol3::SerialProtocol3(SerialComm *serialComm, QObject *parent) : SerialProtocolAbstract(serialComm, parent), mOnlyReadSN(false)
{
    Log();
    timeoutTimerID = 0;
    //m_settings = Settings::Instance()->getSettings();
    downloadInfo.setProcotol(Sp::CommProtocol3);
    connect(&downloadInfo, SIGNAL(downloadProgress(float)), this, SIGNAL(downloadProgress(float)));
    connectSignals();
    modeltype = Sp::DefaultMeter;
}

SerialProtocol3::~SerialProtocol3()
{
    Log();
    timerStop();
}

void SerialProtocol3::setCommObject(SerialComm *serialComm) {
    if(comm) {
        disconnectSignals();
    }
    comm = serialComm;
    connectSignals();
}
void SerialProtocol3::connectSignals() {
    if(comm) {
        connect(comm, SIGNAL(readyRead()), this, SLOT(readyRead()));
//        connect(comm, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(error(QSerialPort::SerialPortError)));
    }
}
void SerialProtocol3::disconnectSignals() {
    if(comm) {
        disconnect(comm, SIGNAL(readyRead()), this, SLOT(readyRead()));
//        disconnect(comm, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(error(QSerialPort::SerialPortError)));
    }
}

Sp::ProtocolState SerialProtocol3::startDownload()
{
    if(currentState == Sp::Idle)
    {
        downloadInfo.setNumberOfGluecose(0);
        requestCommand(Sp::ReadSerialNumber);
    } else
    {
        return currentState;
    }
    return Sp::Idle;
}

Sp::ProtocolState SerialProtocol3::syncTime()
{
    requestCommand(Sp::WriteTimeInformation);

    return Sp::Idle;
}

Sp::ProtocolState SerialProtocol3::readTime()
{
    m_current_time = QDateTime::currentDateTime();
    requestCommand(Sp::ReadTimeInformation);

    return Sp::Idle;
}

Sp::ProtocolState SerialProtocol3::readSerialNumber()
{
    requestCommand(Sp::ReadSerialNumber);
    mOnlyReadSN = true;
    return Sp::Idle;
}

Sp::ProtocolState SerialProtocol3::deleteData()
{
    requestCommand(Sp::DeleteData);

    return Sp::Idle;
}

void SerialProtocol3::cancelDownload()
{

}

void SerialProtocol3::readBleData()
{
    m_settings = Settings::Instance()->getSettings();

    QList<Sp::ProtocolCommand> list;
    list.append(Sp::ReadBLE);
    doCommands(list);
}

void SerialProtocol3::readQcData()
{
    Log();
    m_settings = Settings::Instance()->getSettings();
    int modelType = m_settings.value(SFD_meter_type).toInt();
    if(modelType == 2 || modelType == 3) //SMART, N IoT
    {
        modeltype = Sp::ColorMeter;
        m_data_address = 0x1800;
    }
    else
    {
         m_data_address = 0x1000;
    }

    requestNextQcData();
}

void SerialProtocol3::requestNextQcData()
{
    Log();
    QByteArray requestData;
    requestData.append(0x8B);
    requestData.append((1 << 4) | ((m_data_address & 0xF000) >> 12));
    requestData.append((2 << 4) | ((m_data_address & 0x0F00) >> 8));
    requestData.append((1 << 4) | ((m_data_address & 0x00F0) >> 4));
    requestData.append((2 << 4) | (m_data_address & 0x000F));
    requestData.append(0x10);
    requestData.append(0x2c);

    lastCommand = Sp::ReadQcData;
    timerStart();
    qint64 value = comm->writeData(requestData);
    Log() << value;
}

const QByteArray &SerialProtocol3::lastReceivePacket() {
    return lastRcvPacket;
}

qint64 SerialProtocol3::requestCommand(const Sp::ProtocolCommand &command, QByteArray *arg1, QByteArray *arg2, QByteArray *arg3)
{
    Log();
    timerStop();

#ifdef SERIALCOM_SMARTLOG
    if(currentState == Sp::RequestWaiting) {
        Log() << "Request Waiting......" << lastCommand;
        return 0;
    }
#endif

    int tempindex;
    int year_flag;

    isBleCmd = false;
    isDBLE = false;
    currentState = Sp::RequestWaiting;
    QByteArray requestData = beginCreatePacket();
    switch(command)
    {
        case Sp::GluecoseResultDataTx:
        {
            Q_UNUSED(arg2); Q_UNUSED(arg3);
            Log() << "request GluecoseResultDataTx index =" << *(ushort *)arg1->data();
            requestData.append(makeGluecoseResultDataTx(*arg1));
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::GluecoseResultDataTxExpanded:
        {
            Q_UNUSED(arg3);
            requestData.append(makeGluecoseResultDataTxExpanded(*arg1, *arg2));
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::CurrentIndexOfGluecose:
        {
            Q_UNUSED(arg1); Q_UNUSED(arg2); Q_UNUSED(arg3);
            requestData.append(makeCurrentIndexOfGluecose());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::ReadSerialNumber:
        {
            Q_UNUSED(arg1); Q_UNUSED(arg2); Q_UNUSED(arg3);
            requestData.append(makeReadSerialNumber());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::WriteSerialNumber:
        {
            Q_UNUSED(arg2); Q_UNUSED(arg3);
            requestData.append(makeWriteSerialNumber(*arg1));
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::ReadTimeInformation:
        {
            Q_UNUSED(arg1); Q_UNUSED(arg2); Q_UNUSED(arg3);
            requestData.append(makeReadTimeInformation());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::WriteTimeInformation:
        {
            Q_UNUSED(arg2); Q_UNUSED(arg3);
            requestData.append(makeWriteTimeInformation());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::SaveData:
        {
            Q_UNUSED(arg1); Q_UNUSED(arg2); Q_UNUSED(arg3);
            requestData.append(makeSaveData());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::DeleteData:
        {
            Q_UNUSED(arg1); Q_UNUSED(arg2); Q_UNUSED(arg3);
            requestData.append(makeDeleteData());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::ReadTemperature:
        {
            requestData.clear();
            requestData.append(0x81);
            requestData.append(0x86);
            break;
        }
        case Sp::ReadAnimalType:
        {
            requestData.clear();
            requestData.append(0x8b);
            requestData.append(0x11);
            requestData.append(0x20);
            requestData.append(0x12);
            requestData.append(0x23);
            requestData.append(0x10);
            requestData.append(0x21);
            break;
        }
        case Sp::WriteAnimalType:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x11);
            requestData.append(0x21);
            requestData.append(0x10);
            //set animal type(dog, cat)
            tempindex = m_settings.value(SFD_q_animal_type).toInt();
            if(tempindex == 0)
            {
                requestData.append(0x20);
            }
            else
            {
                requestData.append(0x21);
            }
            requestData.append(0x10);
            requestData.append(0x20);
            break;
        }
        case Sp::SetHour12H:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(m_settings.value(SFD_q_debugflag_upper).toChar());
            requestData.append(m_settings.value(SFD_q_debugflag_lower).toChar());
            break;
        }
        case Sp::SetHour24H:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(m_settings.value(SFD_q_debugflag_upper).toChar());
            requestData.append(m_settings.value(SFD_q_debugflag_lower).toChar());
            break;
        }
        case Sp::SetReset:
        {
            requestData.clear();
            requestData.append(0x8d);
            requestData.append(0x10);
            requestData.append(0x20);
            break;
        }
        case Sp::SetMultichart:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x11);
            requestData.append(0x22);
            break;
        }
        //SetJIGMode
        case Sp::SetJIGMode:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(m_settings.value(SFD_q_debugflag_upper).toChar());
            requestData.append(m_settings.value(SFD_q_debugflag_lower).toChar());
            requestData.append(0x11);
            requestData.append(0x20);
            break;
        }
        case Sp::SetJIG_color:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x10);
            requestData.append(0x2A);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x11);
            requestData.append(0x20);
            break;
        }
        case Sp::SetBLELog:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x11);
            requestData.append(0x25);
            break;
        }
        //setCSMode
        case Sp::SetCSMode:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(m_settings.value(SFD_q_debugflag_upper).toChar());
            requestData.append(m_settings.value(SFD_q_debugflag_lower).toChar());
            requestData.append(0x11);
            requestData.append(0x21);
            break;
        }
        //SetDebugOFF
        case Sp::SetDebugOFF:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(m_settings.value(SFD_q_hourmodeflag_upper).toChar());
            requestData.append(m_settings.value(SFD_q_hourmodeflag_lower).toChar());
            requestData.append(0x10);
            requestData.append(0x20);
            break;
        }
        //SetDebugON
        case Sp::SetDebugON:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(m_settings.value(SFD_q_hourmodeflag_upper).toChar());
            requestData.append(m_settings.value(SFD_q_hourmodeflag_lower).toChar());
            requestData.append(0x10);
            requestData.append(0x21);
            break;
        }
        case Sp::WriteUnitTemp:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x10);
            requestData.append(0x2a);
            requestData.append(0x10);
            //set temperature
            tempindex = m_settings.value(SFD_q_tempset).toInt();
            if(tempindex == 1)
            {
                requestData.append(0x21);
            }
            else if(tempindex == 2)
            {
                requestData.append(0x22);
            }
            else
            {
                requestData.append(0x20);
            }

            //set data unit
            requestData.append(0x10);
            tempindex = m_settings.value(SFD_q_unitset).toInt();
            if(tempindex == 1)
            {
                requestData.append(0x21);
            }
            else if(tempindex == 2)
            {
                requestData.append(0x22);
            }
            else
            {
                requestData.append(0x20);
            }

            break;
        }
        case Sp::WriteDateforamt:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x10);
            requestData.append(0x2b);

            tempindex = m_settings.value(SFD_meter_type).toInt();
            year_flag = m_settings.value(SFD_q_flag_year).toInt();
            if(tempindex == 1 || (tempindex == 0 && year_flag == 1) || tempindex == 4)
            {
                int startyear = m_settings.value(SFD_q_startyear).toInt() - 2000;
                requestData.append(16 | (startyear >> 4));
                requestData.append(32 | (startyear & 15));
            }
            else{
                requestData.append(0x10);
                requestData.append(0x20);
            }

            //dateformat
            requestData.append(0x10);
            tempindex = m_settings.value(SFD_q_dateformat).toInt();
            if(tempindex == 1)
            {
                requestData.append(0x21);
            }
            else
            {
                requestData.append(0x20);
            }

            break;
        }
        case Sp::WriteYear:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x10);
            requestData.append(0x2c);
            //memory low
            tempindex = m_settings.value(SFD_q_memory_count).toInt();
            requestData.append(16 | ((tempindex & 0xff) >> 4));
            requestData.append(32 | ((tempindex & 0xff) & 15));

            //end year
            tempindex = m_settings.value(SFD_q_endyear).toInt() - 2000;
            requestData.append(16 | ( tempindex >> 4));
            requestData.append(32 | (tempindex & 15));

            break;
        }
        case Sp::WriteMemoryCount:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x10);
            requestData.append(0x2d);
            requestData.append(0x10);
            requestData.append(0x20);
            //memory high
            tempindex = m_settings.value(SFD_q_memory_count).toInt();
            requestData.append(16 | ((tempindex >> 8) >> 4));
            requestData.append(32 | ((tempindex >> 8) & 15));
            break;
        }
        case Sp::WriteBLEWay:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x11);
            requestData.append(0x21);
            requestData.append(0x10);
            requestData.append(0x20);
            tempindex = m_settings.value(SFD_q_bleway).toInt();
            if(tempindex == 2)
            {
                requestData.append(0x1D);
                requestData.append(0x2D);
            }
            else
            {
                requestData.append(0x10);
                //ble way

                if(tempindex == 1)
                    requestData.append(0x21);
                else
                    requestData.append(0x20);
            }
            break;
        }

        case Sp::ReadBLE:
        {
            isBleCmd = true;
            Q_UNUSED(arg1); Q_UNUSED(arg2); Q_UNUSED(arg3);
            requestData.append(makeReadBLE());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::WriteBLE:
        {
            Q_UNUSED(arg1); Q_UNUSED(arg2); Q_UNUSED(arg3);
            requestData.append(makeWriteBLE());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::ChangeBLEMode:
        {
            Q_UNUSED(arg1); Q_UNUSED(arg2); Q_UNUSED(arg3);
            requestData.append(makeChangeBLEMODE());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::ChangeBLEMode_EXT:
        {
            isDBLE = true;
            Q_UNUSED(arg1); Q_UNUSED(arg2); Q_UNUSED(arg3);
            requestData.append(makeChangeBLEMODE_EXT());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::Set91:
        {
            requestData.clear();
            requestData.append(0x91);
            break;
        }
        case Sp::Set9A01:
        {
            requestData.clear();
            requestData.append(0x9A);
            requestData.append(0x01);
            break;
        }
        case Sp::Set9A02:
        {
            requestData.clear();
            requestData.append(0x9A);
            requestData.append(0x02);
            break;
        }
        case Sp::Set9A03:
        {
            requestData.clear();
            requestData.append(0x9A);
            requestData.append(0x03);
            break;
        }
        case Sp::SetOffNet:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x22);
            requestData.append(0x11);
            requestData.append(0x2E);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x10);
            requestData.append(0x20);
            break;
        }
        case Sp::SetOnNet:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x22);
            requestData.append(0x11);
            requestData.append(0x2E);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x10);
            requestData.append(0x21);
            break;
        }
        case Sp::SetQCFlag_1:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x10);
            requestData.append(0x27);
            requestData.append(0x1f);
            requestData.append(0x2f);
            requestData.append(0x1f);
            requestData.append(0x2f);
            break;
        }
        case Sp::SetQCFlag_2:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x10);
            requestData.append(0x28);
            requestData.append(0x1f);
            requestData.append(0x2f);
            requestData.append(0x1f);
            requestData.append(0x2f);
            break;
        }
        case Sp::SetQCFlag_3:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x10);
            requestData.append(0x29);
            requestData.append(0x1f);
            requestData.append(0x2f);
            requestData.append(0x1f);
            requestData.append(0x2f);
            break;
        }
        case Sp::ReadAESKey:
        {
            requestData.append(makeReadAESKey());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::WriteAESKey:
        {
            requestData.append(makeWriteAESkey());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::ReadAPN:
        {
            Log();
            requestData.append(makeReadAPNNumber());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::WriteAPN:
        {
            Log();
            requestData.append(makeWriteAPNNumber());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::ReadServerAddress:
        {
            Log();
            requestData.append(makeReadServerAddress());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::WriteServerAddress:
        {
            Log();
            requestData.append(makeWriteServerAddress());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::ReadDeviceToken:
        {
            Log();
            requestData.append(makeReadDeviceToken());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::WriteDeviceToken:
        {
            Log();
            requestData.append(makeWriteDeviceToken());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::ReadServiceName:
        {
            Log();
            requestData.append(makeReadServiceName());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::WriteServiceName:
        {
            Log();
            requestData.append(makeWriteServiceName());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::ReadIMEI:
        {
            Log();
            requestData.append(makeReadIMEI());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::ReadICCID:
        {
            Log();
            requestData.append(makeReadICCID());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::ReadModuleSoftwareVer:
        {
            Log();
            requestData.append(makeReadModuleSoftwareVer());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::ReadIMSI:
        {
            Log();
            requestData.append(makeReadIMSI());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::ReadSktSN:
        {
            Log();
            requestData.append(makeReadSktSN());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::WriteSktSN:
        {
            Log();
            requestData.append(makeWriteSktSN());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::ReadRegistrationURL:
        {
            Log();
            requestData.append(makeReadRegistrationURL());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::WriteRegistrationURL:
        {
            Log();
            requestData.append(makeWriteRegistrationURL());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::WriteCodingMode:
        {
            Log();
            requestData.append(makeWriteCodingMode());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::ReadCodingMode:
        {
            Log();
            requestData.append(makeReadCodingMode());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::WriteOtgMode:
        {
            Log();
            requestData.append(makeWriteOtgMode());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::ReadOtgMode:
        {
            Log();
            requestData.append(makeReadOtgMode());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        case Sp::SetHour12H_color:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x22);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(m_settings.value(SFD_q_timeformat_upper).toChar());
            requestData.append(m_settings.value(SFD_q_timeformat_lower).toChar());
            requestData.append(0x10);
            requestData.append(0x20);
            break;
        }
        case Sp::SetHour24H_color:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x22);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(m_settings.value(SFD_q_timeformat_upper).toChar());
            requestData.append(m_settings.value(SFD_q_timeformat_lower).toChar());
            requestData.append(0x10);
            requestData.append(0x21);
            break;
        }
        case Sp::WriteSetHypo:
        {
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x13);
            requestData.append(0x23);
            requestData.append(0x10);
            requestData.append(0x20);
            tempindex = m_settings.value(SFD_q_sethypo).toInt();
            requestData.append(16 | ((tempindex & 0xff) >> 4));
            requestData.append(32 | ((tempindex & 0xff) & 15));
            Log() << "Sethypo = " << tempindex;
            break;
        }
        case Sp::ReadSetHypo:
        {
            requestData.clear();
            requestData.append(0x8b);
            requestData.append(0x11);
            requestData.append(0x20);
            requestData.append(0x1e);
            requestData.append(0x26);
            requestData.append(0x10);
            requestData.append(0x22);
            break;
        }
        case Sp::GetGlucoseDataFlag:
        {
            requestData.clear();
            requestData.append(0xe0);
            requestData.append(0x01);
            requestData.append(0xe0 ^ 0x01);
            break;
        }
        case Sp::SetGlucoseDataFlag:
        {
            requestData.clear();
            requestData.append(0xe1);
            //type
            requestData.append(0x01);
            //size
            requestData.append(0x05);
            //default
            requestData.append(m_settings.value(SFD_q_flag_default).toInt());
            //nomark
            requestData.append(m_settings.value(SFD_q_flag_nomark).toInt());
            //premeal
            requestData.append(m_settings.value(SFD_q_flag_premeal).toInt());
            //postmeal
            requestData.append(m_settings.value(SFD_q_flag_postmeal).toInt());
            //fasting
            requestData.append(m_settings.value(SFD_q_flag_fasting).toInt());

            int value = 0xe1;
            for(tempindex = 1; tempindex < requestData.count(); tempindex++)
            {
                value = value ^ requestData[tempindex];
            }

            requestData.append(value);

            break;
        }
        case Sp::SetAvgDays:
        {
            requestData.clear();
            requestData.append(0xf1);
            //type
            requestData.append(0x01);
            //size
            requestData.append(0x07);
            //count of average days
            requestData.append(m_settings.value(SFD_q_enabled_avg_days_count).toInt());
            //average 1 day
            requestData.append(m_settings.value(SFD_q_flag_avg_days_1).toInt());
            //average 7 days
            requestData.append(m_settings.value(SFD_q_flag_avg_days_7).toInt());
            //average 14 days
            requestData.append(m_settings.value(SFD_q_flag_avg_days_14).toInt());
            //average 30 days
            requestData.append(m_settings.value(SFD_q_flag_avg_days_30).toInt());
            //average 60 days
            requestData.append(m_settings.value(SFD_q_flag_avg_days_60).toInt());
            //average 90 days
            requestData.append(m_settings.value(SFD_q_flag_avg_days_90).toInt());

            int value = 0xf1;
            for(tempindex = 1; tempindex < requestData.count(); tempindex++)
            {
                value = value ^ requestData[tempindex];
            }

            requestData.append(value);

            break;
        }
        case Sp::GetAvgDays:
        {
            requestData.clear();
            requestData.append(0xf0);
            requestData.append(0x01);
            requestData.append(0xf0 ^ 0x01);
            break;
        }
        case Sp::Write_Temperature_Range_Low:
        {
            //0x1025 0x1024
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x11);
            requestData.append(0x22);
            //temperature_range_low_highbyte
            tempindex = m_settings.value(SFD_q_temperature_range_low).toInt() * 10;
            requestData.append(16 | ((tempindex >> 8) >> 4));
            requestData.append(32 | ((tempindex >> 8) & 15));
            //temperature_range_low_lowbyte
            requestData.append(16 | ((tempindex & 0xff) >> 4));
            requestData.append(32 | ((tempindex & 0xff) & 15));
            break;
        }
        case Sp::Write_Temperature_Range_High:
        {
            //0x1027 0x1026
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x11);
            requestData.append(0x23);
            //temperature_range_high_highbyte
            tempindex = m_settings.value(SFD_q_temperature_range_high).toInt() * 10;
            requestData.append(16 | ((tempindex >> 8) >> 4));
            requestData.append(32 | ((tempindex >> 8) & 15));
            //temperature_range_high_lowbyte
            requestData.append(16 | ((tempindex & 0xff) >> 4));
            requestData.append(32 | ((tempindex & 0xff) & 15));
            break;
        }

        case Sp::Write_Measure_Range_Low:
        {
            //0x1029 0x1028
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x11);
            requestData.append(0x24);
            //measure_range_low_highbyte
            tempindex = m_settings.value(SFD_q_measure_range_low).toInt() - 1;
            requestData.append(16 | ((tempindex >> 8) >> 4));
            requestData.append(32 | ((tempindex >> 8) & 15));
            //measure_range_low_lowbyte
            requestData.append(16 | ((tempindex & 0xff) >> 4));
            requestData.append(32 | ((tempindex & 0xff) & 15));
            break;
        }
        case Sp::Write_Measure_Range_High:
        {
            //0x102B 0x102A
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x11);
            requestData.append(0x25);
            //measure_range_high_highbyte
            tempindex = m_settings.value(SFD_q_measure_range_high).toInt() + 1;
            requestData.append(16 | ((tempindex >> 8) >> 4));
            requestData.append(32 | ((tempindex >> 8) & 15));
            //measure_range_high_lowbyte
            requestData.append(16 | ((tempindex & 0xff) >> 4));
            requestData.append(32 | ((tempindex & 0xff) & 15));
            break;
        }
        case Sp::Write_Hypo_initial:
        {
            //0x102D 0x102C
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x11);
            requestData.append(0x26);
            //Hypo_initial_highbyte
            tempindex = m_settings.value(SFD_q_hypo_initial).toInt();
            requestData.append(16 | ((tempindex >> 8) >> 4));
            requestData.append(32 | ((tempindex >> 8) & 15));

            //Hypo_initial_lowbyte
            tempindex = m_settings.value(SFD_q_hypo_initial).toInt();
            requestData.append(16 | ((tempindex & 0xff) >> 4));
            requestData.append(32 | ((tempindex & 0xff) & 15));
            break;
        }

        case Sp::Write_Hyper_initial:
        {
            //0x102F 0x102E
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x11);
            requestData.append(0x27);
            //Hyper_initial_highbyte
            tempindex = m_settings.value(SFD_q_hyper_initial).toInt();
            requestData.append(16 | ((tempindex >> 8) >> 4));
            requestData.append(32 | ((tempindex >> 8) & 15));

            //Hyper_initial_lowbyte
            tempindex = m_settings.value(SFD_q_hyper_initial).toInt();
            requestData.append(16 | ((tempindex & 0xff) >> 4));
            requestData.append(32 | ((tempindex & 0xff) & 15));
            break;
        }

        case Sp::Write_Hypo_on_off:
        case Sp::Write_Hyper_on_off:
        {
            //0x1031 0x1030
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x11);
            requestData.append(0x28);
            //Hyper_on_off
            tempindex = m_settings.value(SFD_q_hyper_on_off).toInt();
            requestData.append(0x10);
            if(tempindex == 0)
            {
                requestData.append(0x20);
            }
            else
            {
                requestData.append(0x21);
            }

            //Hypo_on_off
            tempindex = m_settings.value(SFD_q_hypo_on_off).toInt();
            requestData.append(0x10);
            if(tempindex == 0)
            {
                requestData.append(0x20);
            }
            else
            {
                requestData.append(0x21);
            }
            break;
        }

        case Sp::Write_Buzzer_on_off:
        case Sp::Write_Strip_TYPE_INFO_1:
        {
            //0x1033 - strip procotol version
            //0x1032 - buzzer on/off
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x11);
            requestData.append(0x29);

            //Protocol version
            tempindex = 1;
            requestData.append(16 | ((tempindex & 0xff) >> 4));
            requestData.append(32 | ((tempindex & 0xff) & 15));

            //buzzer_on_off
            tempindex = m_settings.value(SFD_q_buzzer_on_off).toInt();
            requestData.append(0x10);
            if(tempindex == 0)
            {
                requestData.append(0x20);
            }
            else
            {
                requestData.append(0x21);
            }
            break;
        }
        case Sp::Write_Strip_TYPE_INFO_2:
        {

            //0x1035 Use CareSens Pro
            //0x1034 Use BAROZen
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x11);
            requestData.append(0x2A);

            //sse CareSens Pro
            tempindex = m_settings.value("g_strip_type_cspro").toInt();
            requestData.append(16 | ((tempindex & 0xff) >> 4));
            requestData.append(32 | ((tempindex & 0xff) & 15));

            //use barozen
            tempindex = m_settings.value(SFD_g_strip_type_barozen).toInt();
            requestData.append(16 | ((tempindex & 0xff) >> 4));
            requestData.append(32 | ((tempindex & 0xff) & 15));



            break;
        }
        case Sp::Write_Strip_TYPE_INFO_3:
        {
            //0x1037 Use Barozen Ketone
            //0x1036 Use CareSens Pro Ketone
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x21);
            requestData.append(0x11);
            requestData.append(0x2B);

            // Use Barozen Ketone
            tempindex = m_settings.value(SFD_g_strip_type_barozen_k).toInt();
            requestData.append(16 | ((tempindex & 0xff) >> 4));
            requestData.append(32 | ((tempindex & 0xff) & 15));

            // Use CareSens Pro Ketone
            tempindex = m_settings.value("g_strip_type_cspro_k").toInt();
            requestData.append(16 | ((tempindex & 0xff) >> 4));
            requestData.append(32 | ((tempindex & 0xff) & 15));

            break;
        }

        case Sp::Write_TI_SetCode:
        {
            //0x1000
            requestData.clear();
            requestData.append(0x8c);
            requestData.append(0x10);
            requestData.append(0x20);
            requestData.append(0x10);
            requestData.append(0x20);
            //set code
            tempindex = m_settings.value(SFD_u_ti_setcode).toInt() + 1;
            requestData.append(16 | (tempindex >> 4));
            requestData.append(32 | (tempindex & 15));
            requestData.append(0x10);
            requestData.append(0x20);
            break;
        }
        case Sp::Write_AudioMode_Volume:
        {
             //0x108b
             requestData.clear();
             requestData.append(0x8c);
             requestData.append(0x10);
             requestData.append(0x20);//segment
             requestData.append(0x10);
             requestData.append(0x25);
             //set audio volume
             tempindex = m_settings.value(SFD_q_setaudiovolume).toInt();
             requestData.append(16 | (tempindex >> 4));
             requestData.append(32 | (tempindex & 15));

             requestData.append(0x10);
             requestData.append(0x20);

             break;
        }
        case Sp::Write_Audio_LangCount:
        {
             requestData.clear();
             requestData.append(0x8c);
             requestData.append(0x10);
             requestData.append(0x20);//segment
             requestData.append(0x10);
             requestData.append(0x26);

             //set audio support language count
             tempindex = m_settings.value(SFD_q_setaudiolangcount).toInt();
             requestData.append(16 | (tempindex >> 4));
             requestData.append(32 | (tempindex & 15));

             //set audio mode
             requestData.append(0x10);
             tempindex = m_settings.value(SFD_q_setaudiomode).toInt();
             if(tempindex == 0)//Off, Beep mode
             {
                 requestData.append(0x20);
             }
             else //volume 1-5
             {
                 requestData.append(0x21);
             }

             break;
        }
        case Sp::Write_AudioMode_Volume_TI:
        {
             requestData.clear();
             requestData.append(0x8c);
             requestData.append(0x10);
             requestData.append(0x20);
             requestData.append(0x13);
             requestData.append(0x24);
             requestData.append(0x10);
             requestData.append(0x20);
             requestData.append(0x10);
             requestData.append(0x20);

             //set audio volume
             tempindex = m_settings.value(SFD_q_setaudiovolume).toInt();

             if (tempindex == 1)
             {
                 requestData[8] = 0x25;
             }
             else if (tempindex == 2)
             {
                 requestData[6] = 0x21;
                 requestData[8] = 0x23;
             }
             else if (tempindex == 3)
             {
                 requestData[6] = 0x21;
                 requestData[8] = 0x24;
             }
             else if (tempindex == 4)
             {
                 requestData[6] = 0x21;
                 requestData[8] = 0x25;
             }
             else if (tempindex == 5)
             {
                 requestData[6] = 0x21;
                 requestData[8] = 0x26;
             }
             else if (tempindex == 6)
             {
                 requestData[6] = 0x21;
                 requestData[8] = 0x27;
             }

             break;
        }
        case Sp::Write_Audio_LangCount_TI:
        {
             requestData.clear();
             requestData.append(0x8c);
             requestData.append(0x10);
             requestData.append(0x20);
             requestData.append(0x13);
             requestData.append(0x25);
             requestData.append(0x10);
             requestData.append(0x20);
             requestData.append(0x10);
             requestData.append(0x20);

            //set audio support language count
             tempindex = m_settings.value(SFD_q_setaudiolangcount).toInt();
             if (tempindex == 1)
             {
                requestData[8] = 0x21;
             }
             else if (tempindex == 2)
             {
                 requestData[8] = 0x22;
             }
             else if (tempindex == 3)
             {
                 requestData[8] = 0x23;
             }
             else if (tempindex == 4)
             {
                 requestData[8] = 0x24;
             }
             else if (tempindex == 5)
             {
                 requestData[8] = 0x25;
             }
             else if (tempindex == 6)
             {
                 requestData[8] = 0x26;
             }
             else if (tempindex == 7)
             {
                 requestData[8] = 0x27;
             }
             else if (tempindex == 8)
             {
                 requestData[8] = 0x28;
             }

             break;
        }
        case Sp::Unlock:
        {
            Log();
            requestData.append(makeUnlockData());
            endCreatePacket((QByteArray *)&requestData);
            break;
        }
        default:
        {
            Q_ASSERT(0);
            return 0;
        }
    }
    Log() << "lastcommand = " <<  command << "request" << requestData.toHex();
    lastCommand = command;
    m_current_cmd_index++;
    timerStart();

#ifdef Q_OS_MACX
    if(command == Sp::ReadTimeInformation) {
        QThread::msleep(1000);
    }
#endif
    return comm->writeData(requestData);
}

QByteArray SerialProtocol3::beginCreatePacket() {
    QByteArray array;
    array.append(0x02);
    if (m_settings.value(SFD_meter_type)  == 4)  //VetMate
    {
        array.append("iSVT");
    }
    else
    {
        array.append("iSPc");
    }

    return array;
}

void SerialProtocol3::endCreatePacket(QByteArray *array) {
    QByteArray crc = argUShort(calcCrc(*array));
    int count = array->count();
    array->insert(count-1, crc);
}
QByteArray SerialProtocol3::makeGluecoseResultDataTx(QByteArray indexArray) {
    QByteArray array;
    array.append("GLUC");
    array.append(indexArray);
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}
QByteArray SerialProtocol3::makeGluecoseResultDataTxExpanded(QByteArray indexArray, QByteArray countArray)
{
    QByteArray array;
    array.append("GLUE");
    array.append(indexArray);
    array.append(countArray);
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}
QByteArray SerialProtocol3::makeCurrentIndexOfGluecose() {
    QByteArray array;
    array.append("NCOT");
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeReadSerialNumber() {
    QByteArray array;
    array.append("RSNB");
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}
QByteArray SerialProtocol3::makeWriteSerialNumber(QByteArray sn) {
    QByteArray array;
    array.append("WSNB");
    array.append(sn);
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}
QByteArray SerialProtocol3::createWriteSerialNumber()
{
    QString sn = m_settings.value(SFD_q_new_serialnumber).toString();
    m_settings.insert(SFD_q_serialnumber, QVariant(sn));
    m_settings.insert(SFD_q_new_serialnumber, QVariant(""));
    Settings::Instance()->setSerialNumber(sn, "");
    sn = sn.replace(" ","");
    QByteArray temp = sn.toLocal8Bit();
    QByteArray array;
    array.append("WSNB");
    array.append(0x0d);
    array.append(temp);
    array.append((char)0x00);
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}
QByteArray SerialProtocol3::makeReadTimeInformation() {
    QByteArray array;
    array.append("RTIM");
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}
QByteArray SerialProtocol3::makeWriteTimeInformation() {
    QByteArray array;
    array.append("WTIM");

    QDate date = QDate::currentDate();

    array.append(date.year() - 2000);
    array.append(date.month());
    array.append(date.day());

    QTime currenttime = QTime::currentTime();

    array.append(currenttime.hour());
    array.append(currenttime.minute());
    array.append(currenttime.second());

    m_current_time.setDate(date);
    m_current_time.setTime(currenttime);

    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}
QByteArray SerialProtocol3::makeSaveData() {
    QByteArray array;
    array.append("SVDT");
#ifdef FROM_METERMEMORY
    // 3 lines from metermemory
    int dataCnt = Settings::Instance()->getSaveDataCnt();
    array.append((dataCnt & 0xff00) >> 8);
    array.append(dataCnt & 0xff);
#endif

    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}
QByteArray SerialProtocol3::makeDeleteData() {
    QByteArray array;
    array.append("DELD");
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeReadBLE()
{
    QByteArray array;
    array.append("RBLE");
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeWriteBLE()
{
    QString name = m_settings.value(SFD_q_blename).toString();
    QByteArray temp = name.toLocal8Bit();
    QByteArray array;
    array.append("WBLE");
    array.append(name.length() + 1);//길이
    array.append(temp);
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeChangeBLEMODE()
{
    QByteArray array;
    array.append("CBLE");
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeChangeBLEMODE_EXT()
{
    QByteArray array;
    array.append("DBLE");
    int value = m_settings.value(SFD_u_setbleonoff).toInt();
    array.append(value);
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeReadAESKey()
{
    QByteArray array;
    array.append("RKY1");
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeWriteAESkey()
{
    QByteArray array;
    array.append("WKY1");
    array.append(0x27);//길이

    //random
    for(int i = 0; i < 32; i++)
    {
        array.append(qrand() % 256);
    }

    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeReadAPNNumber()
{
    QByteArray array;
    array.append("RAPN");
    array.append(0x03);
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeWriteAPNNumber()
{
    QByteArray array;
    array.append("WAPN");
    QString apnkey = m_settings.value(SFD_q_apnkey).toString();
    Log() << m_settings.value(SFD_q_apnkey).toString();
    array.append(apnkey.length() + 1);//길이
    array.append(m_settings.value(SFD_q_apnkey).toString().toLocal8Bit());
    array.append((char)0x00);
    array.append(0x03);
    Log() <<  array;
    //패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeReadServerAddress()
{
    QByteArray array;
    array.append("RSAD");
    array.append(0x03);
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeWriteServerAddress()
{
    QByteArray array;
    array.append("WSAD");
    QString server_address = m_settings.value(SFD_q_server_address).toString();
    Log() << m_settings.value(SFD_q_server_address).toString();
    array.append(server_address.length() + 1);//길이
    array.append(server_address.toLocal8Bit());
    array.append((char)0x00);
    array.append(0x03);
    Log() <<  array;
    //패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeReadDeviceToken()
{
    QByteArray array;
    array.append("RDTO");
    array.append(0x03);
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeWriteDeviceToken()
{
    QByteArray array;
    array.append("WDTO");
    QString device_token = m_settings.value(SFD_q_device_token).toString();
    Log() << m_settings.value(SFD_q_device_token).toString();
    array.append(device_token.length() + 1);//길이
    array.append(device_token.toLocal8Bit());
    array.append((char)0x00);
    array.append(0x03);
    Log() <<  array;
    //패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeReadServiceName()
{
    QByteArray array;
    array.append("RSNA");
    array.append(0x03);
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeWriteServiceName()
{
    QByteArray array;
    array.append("WSNA");
    QString service_name = m_settings.value(SFD_q_service_name).toString();
    Log() << m_settings.value(SFD_q_service_name).toString();
    array.append(service_name.length() + 1);//길이
    array.append(service_name.toLocal8Bit());
    array.append((char)0x00);
    array.append(0x03);
    Log() <<  array;
    //패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeWriteSktSN()
{
    QByteArray array;
    array.append("WSKN");
    QString skt_sn = m_settings.value(SFD_q_skt_sn).toString();
    Log() << m_settings.value(SFD_q_skt_sn).toString();
    array.append(skt_sn.length() + 1);//길이
    array.append(skt_sn.toLocal8Bit());
    array.append((char)0x00);
    array.append(0x03);
    Log() <<  array;
    //패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeReadIMEI()
{
    QByteArray array;
    array.append("RIME");
    array.append(0x03);
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeReadICCID()
{
    QByteArray array;
    array.append("RICC");
    array.append(0x03);
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeReadModuleSoftwareVer()
{
    QByteArray array;
    array.append("RMSW");
    array.append(0x03);
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeReadIMSI()
{
    QByteArray array;
    array.append("RIMS");
    array.append(0x03);
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeReadSktSN()
{
    QByteArray array;
    array.append("RSKN");
    array.append(0x03);
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeLockData()
{
    QByteArray array;
    array.append("LOCK");
    array.append(0x55);
    array.append(0xAA);
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeUnlockData()
{
    QByteArray array;
    array.append("LOCK");
    array.append(0xAA);
    array.append(0x55);
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeReadRegistrationURL()
{
    QByteArray array;
    array.append("RURL");
    array.append(0x03);
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeWriteRegistrationURL()
{
    QByteArray array;
    array.append("WURL");
    QString url = m_settings.value(SFD_q_registration_url).toString();
    Log() << m_settings.value(SFD_q_registration_url).toString();
    array.append(url.length() + 1);//길이
    array.append(url.toLocal8Bit());
    array.append((char)0x00);
    array.append(0x03);
    Log() <<  array;
    //패킷 사이즈s
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeWriteCodingMode()
{
    QByteArray array;
    array.append("WCOD");
    array.append(m_settings.value(SFD_q_code_mode).toInt());
    array.append(0x03);
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeReadCodingMode()
{
    QByteArray array;
    array.append("RCOD");
    array.append(0x03);
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeWriteOtgMode()
{
    QByteArray array;
    array.append("WCNM");
    array.append(m_settings.value(SFD_q_otg_mode).toInt());
    array.append(0x03);
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol3::makeReadOtgMode()
{
    QByteArray array;
    array.append("RCNM");
    array.append(0x03);
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

bool SerialProtocol3::isCompletePacketReceved() {
    bufferMutex.lock();
    if(readBuffer.count() > 5) {
        uchar size = readBuffer.at(headerSize());
        if(readBuffer.count() >= size+headerSize()+1 || lastCommand == Sp::ReadQcData || lastCommand == Sp::ReadBLE) {   // stx 부터 크기까지의 고정 바이트수 == headerSize()+1
            bufferMutex.unlock();
            return true;
        }
    }
    bufferMutex.unlock();
    return false;
}
const QByteArray SerialProtocol3::getReceivePacket() {

    bufferMutex.lock();
    int size = dataSize(readBuffer);
    int length =  size+headerSize()+1;
    if(readBuffer.count() >= length) {   // stx 부터 크기까지의 고정 바이트수 == headerSize()+1
        QByteArray rcvPacket;
        rcvPacket.append(readBuffer.left(length));
        readBuffer.remove(0, length);
        bufferMutex.unlock();
        return rcvPacket;
    }
    bufferMutex.unlock();
    return QByteArray();
}
Sp::ProtocolCommand SerialProtocol3::getCommand(QByteArray rcvPacket) {
    QString cmd = QString(rcvPacket.mid(6, 4));
    if(cmd == "NCOT")
        return Sp::CurrentIndexOfGluecose;
    if(cmd == "GLUC")
        return Sp::GluecoseResultDataTx;
    if(cmd == "GLUE")
        return Sp::GluecoseResultDataTxExpanded;
    if(cmd == "RSNB")
        return Sp::ReadSerialNumber;
    if(cmd == "WSNB")
        return Sp::WriteSerialNumber;
    if(cmd == "RTIM")
        return Sp::ReadTimeInformation;
    if(cmd == "WTIM")
        return Sp::WriteTimeInformation;
    if(cmd == "TOUT")
        return Sp::CommunicationTimeout;
    if(cmd == "HEAD")
        return Sp::HeaderPacketVerifyError;
    if(cmd == "SIZE")
        return Sp::SizeOfPacketVerifyError;
    if(cmd == "ECRC")
        return Sp::CRCPacketVerifyError;
    if(cmd == "RBLE")
        return Sp::ReadBLE;
    if(cmd == "CBLE")
        return Sp::ChangeBLEMode;
    if(cmd == "DBLE")
        return Sp::ChangeBLEMode_EXT;
    if(cmd == "CMND")
        return Sp::CommandPacketVerifyError;
    if(cmd == "RKY1")
        return Sp::ReadAESKey;
    if(cmd == "WKY1")
        return Sp::WriteAESKey;
    if(cmd == "RAPN")
        return Sp::ReadAPN;
    if(cmd == "WAPN")
        return Sp::WriteAPN;
    if(cmd == "RSAD")
        return Sp::ReadServerAddress;
    if(cmd == "WSAD")
        return Sp::WriteServerAddress;
    if(cmd == "RDTO")
        return Sp::ReadDeviceToken;
    if(cmd == "WDTO")
        return Sp::WriteDeviceToken;
    if(cmd == "RSNA")
        return Sp::ReadServiceName;
    if(cmd == "WSNA")
        return Sp::WriteServiceName;
    if(cmd == "RIME")
        return Sp::ReadIMEI;
    if(cmd == "RICC")
        return Sp::ReadICCID;
    if(cmd == "RMSW")
        return Sp::ReadModuleSoftwareVer;
    if(cmd == "RIMS")
        return Sp::ReadIMSI;
    if(cmd == "WSKN")
        return Sp::WriteSktSN;
    if(cmd == "RSKN")
        return Sp::ReadSktSN;
    if(cmd == "RURL")
        return Sp::ReadRegistrationURL;
    if(cmd == "WURL")
        return Sp::WriteRegistrationURL;
    if(cmd == "DELD")
        return Sp::DeleteData;
    if(cmd == "LOCK")
        return Sp::Unlock;
    if(cmd=="WCOD")
        return Sp::WriteCodingMode;
    if(cmd=="RCOD")
        return Sp::ReadCodingMode;
    if(cmd=="WCNM")
        return Sp::WriteOtgMode;
    if(cmd=="RCNM")
        return Sp::ReadOtgMode;
    return Sp::ProtocolCommandNone;
}
ushort SerialProtocol3::getIndexOfGluecose(QByteArray rcvPacket) {
    ushort index = *(ushort *)rcvPacket.mid(10, 2).data();
    index = byteswap(index);
    return index;
}
ushort SerialProtocol3::getGluecoseCount(QByteArray rcvPacket) {
    ushort cnt = rcvPacket.count()-13;
    if(cnt%9) {
        Q_ASSERT(0);
    }
    return ushort(cnt/9);
}

void SerialProtocol3::produceError(Sp::ProtocolCommand receivedCommand, QByteArray rcvPacket) {
    Log() << "Error Command received" << rcvPacket.mid(6, 4);
    timerStop();
    emit errorOccurred(receivedCommand, pLastCommand);

}
void SerialProtocol3::processPacket(QByteArray rcvPacket)
{
    Log();
    timerStop();
    currentState = Sp::GluecoseDownloading;

    // crc check
    QByteArray checkPacket = QByteArray(rcvPacket.left(rcvPacket.count()-3));
    checkPacket.append(rcvPacket.at(rcvPacket.count()-1));
    QByteArray crcArray = rcvPacket.mid(rcvPacket.count()-3, 2);
    ushort rcvCrc = *(ushort *)(crcArray.data());
    rcvCrc = byteswap(rcvCrc);
    if((lastCommand == Sp::ReadSerialNumber && (int)rcvPacket[rcvPacket.count() - 1] == kETX) //RSNB는 CRC 체크하지 않는다.
       || isEqualCrc(checkPacket, rcvCrc)
       || lastCommand == Sp::GluecoseResultDataTxExpanded)
    {
        pLastCommand = lastCommand;
        lastCommand = Sp::ProtocolCommandNone;
        Sp::ProtocolCommand cmd = getCommand(rcvPacket);

        if(cmd >= Sp::CommunicationTimeout)  //isErrorResponse
        {
            produceError(cmd, rcvPacket);
            currentState = Sp::Idle;
            return;
        }

        parseReceivedData(rcvPacket);

#ifdef SERIALCOM_SMARTLOG
        if(cmd == Sp::ReadSerialNumber)
        {
            currentState = Sp::GluecoseDownloading;

            downloadInfo.setNumberOfGluecose(0);

            if(mOnlyReadSN)
            {
                emit completeReadSN(m_serialnumber);
            }
            else
            {
                emit downloadProgress(downloadInfo.progress());
                requestCommand(Sp::CurrentIndexOfGluecose);
            }
        }
        else if(cmd == Sp::CurrentIndexOfGluecose)
        {
            // 다운로드할 글루코스 데이터 개수 설정: 다운로드가 시작됨
            downloadInfo.setNumberOfGluecose(getIndexOfGluecose(rcvPacket));

            emit downloadProgress(downloadInfo.progress());

            QByteArray arg1 = argUShort(ushort(downloadInfo.index()));
            QByteArray arg2 = argByte(char(downloadInfo.downloadableCount()));
            requestCommand(Sp::GluecoseResultDataTxExpanded, &arg1, &arg2);
        }
        else if(cmd == Sp::GluecoseResultDataTxExpanded)
        {
            // TODO: 다운로드 된 데이터 처리 한다.
            emit packetReceived();

            downloadInfo.setDownloadedCount(getGluecoseCount(rcvPacket));

            if(downloadInfo.downloadableCount())
            {
                currentState = Sp::GluecoseDownloading;
                QByteArray arg1 = argUShort(ushort(downloadInfo.index()));
                QByteArray arg2 = argByte(char(downloadInfo.downloadableCount()));
                requestCommand(Sp::GluecoseResultDataTxExpanded, &arg1, &arg2);
            }
            else
            {
                QJsonObject sn;
                sn["sn"] = m_serialnumber;
                m_dataArray.push_front(sn);
                Log() << "DataDownload Complete total = " << downloadInfo.getNunberOfGluecose() << m_dataArray.count();

                for(int i=0; i<m_dataArray.count(); i++)
                {
                    Log() <<m_dataArray.at(i);
                }

                emit downloadComplete(&m_dataArray);
            }
        }
        else if(cmd == Sp::WriteTimeInformation)
        {
            //requestCommand(Sp::ReadTimeInformation);
        }
        else if(cmd == Sp::ReadTimeInformation)
        {
            //emit meter time & current
            //emit completeTimeSync(&m_meter_datatime, &m_current_time);
        }
        else if(cmd == Sp::DeleteData)
        {
            emit completeDelData();
        }
        else {
            // 다운로드 과정에서 처리하지 않는 커멘드의 수신
             emit errorUnresolvedCommand(cmd);
        }
#endif

#ifdef SERIALCOM_QC
        if(m_current_cmd_index < m_commands.length())
        {

            Log();
            //go ahead
            requestCommand(m_commands[m_current_cmd_index]);
        }
        else
        {
            if(cmd == Sp::GluecoseResultDataTxExpanded)
            {
                Log();

                if(downloadInfo.downloadableCount())
                {
                    m_current_cmd_index--;
                    requestCommand(m_commands[m_current_cmd_index]);
                }
                else
                {
                    emit downloadComplete(&m_dataArray);
                }

            }
            else
            {
                emit finishDoCommands(true, m_commands[m_current_cmd_index - 1]);
            }
        }
#endif
    }
    else
    {
        Log() << "CRC error";
        currentState = Sp::Idle;
        emit errorCrc();
    }
}

void SerialProtocol3::parseQcData(QByteArray rcvPacket)
{
    if(modeltype == Sp::ColorMeter)
    {
        parseQcData_cs_color(rcvPacket);
    }
    else
    {
        parseQcData_default(rcvPacket);
    }

}


void SerialProtocol3::parseQcData_cs_color(QByteArray rcvPacket)
{
    int temp_deci = 0;

    if(m_data_address == 0xf998 + 12)
    {
        Log() << "End";
        timerStop();
        Log() << m_settings.value(SFD_q_serialnumber).toString();
        Log() << m_settings.value(SFD_q_new_serialnumber).toString();
        Settings::Instance()->setSettings(m_settings);
        pLastCommand = lastCommand;
        lastCommand = Sp::ProtocolCommandNone;
        emit finishReadingQcData();
    }
    else
    {
        Log() << "memory = " << ((m_data_address - 0x1800)/12 + 1);
        if(((m_data_address - 0x1800)/12 + 1) == 1)
        {
            int nAvRef = ((rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F))+ ((rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F)) * 256;
            m_settings.insert(SFD_q_avref, QVariant(nAvRef));
            int nDacCalWork = ((rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F)) + ((rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F)) * 256;
            m_settings.insert(SFD_q_daccalwork, QVariant(nDacCalWork));
            int nDacCalThird = ((rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F)) + ((rcvPacket[16] & 0x0F)*16 + (rcvPacket[17] & 0x0F)) * 256;
            m_settings.insert(SFD_q_daccalthird, QVariant(nDacCalThird));
            Log() << "nAvRef = " << nAvRef << " nDacCalWork = " << nDacCalWork << " nDacCalThird" << nDacCalThird;
            Log() << "nAvRef setting = " << m_settings.value(SFD_q_avref).toString();

            QByteArray temparr;
            temparr.append(((rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F)));
            temparr.append(((rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F)));
            temparr.append(((rcvPacket[25] & 0x0F)*16 + (rcvPacket[26] & 0x0F)));
            temparr.append(((rcvPacket[28] & 0x0F)*16 + (rcvPacket[29] & 0x0F)));

            float gainslope = bytesToFloat(temparr[3], temparr[2], temparr[1], temparr[0]);
            m_settings.insert(SFD_q_gainslope, QVariant(QString().sprintf("%.3f", gainslope)));

            next_temparr.clear();
            next_temparr.append(((rcvPacket[31] & 0x0F)*16 + (rcvPacket[32] & 0x0F)));
            next_temparr.append(((rcvPacket[34] & 0x0F)*16 + (rcvPacket[35] & 0x0F)));


        }
        else if(((m_data_address - 0x1800)/12 + 1) == 2)
        {
            next_temparr.append(((rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F)));
            next_temparr.append(((rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F)));
            float gainYcut = bytesToFloat(next_temparr[3], next_temparr[2], next_temparr[1], next_temparr[0]);
            m_settings.insert(SFD_q_gainYcut, QVariant(QString().sprintf("%.3f", gainYcut)));

            temp_deci = ((qint8(rcvPacket[7] & 0x0F) * 16) + qint8(rcvPacket[8] & 0x0F)) % 3;
            m_settings.insert(SFD_q_qc_5, temp_deci);

            temp_deci = ((qint8(rcvPacket[10] & 0x0F) * 16) + qint8(rcvPacket[11] & 0x0F)) % 3;
            m_settings.insert(SFD_q_qc_10, temp_deci);

            temp_deci = ((qint8(rcvPacket[13] & 0x0F) * 16) + qint8(rcvPacket[14] & 0x0F)) % 3;
            m_settings.insert(SFD_q_current_qc, temp_deci);

            temp_deci = ((qint8(rcvPacket[16] & 0x0F) * 16) + qint8(rcvPacket[17] & 0x0F)) % 3;
            m_settings.insert(SFD_q_lcd, temp_deci);

            temp_deci = ((qint8(rcvPacket[19] & 0x0F) * 16) + qint8(rcvPacket[20] & 0x0F)) % 3;
            m_settings.insert(SFD_q_fuel_gage, temp_deci);

            temp_deci = ((qint8(rcvPacket[22] & 0x0F) * 16) + qint8(rcvPacket[23] & 0x0F)) % 3;
            m_settings.insert(SFD_q_sound, temp_deci);


        }
        else if(((m_data_address - 0x1800)/12 + 1) == 12)
        {
           //considering

        }
        else if(((m_data_address - 0x1800)/12 + 1) == 22)
        {
            temp_deci = (qint8(rcvPacket[13] & 0x0F) * 16) + qint8(rcvPacket[14] & 0x0F);
            m_settings.insert(SFD_q_hourmodeflag, QVariant(temp_deci));

            m_settings.insert(SFD_q_timeformat_upper, QVariant(rcvPacket[16]));
            m_settings.insert(SFD_q_timeformat_lower, QVariant(rcvPacket[17]));

        }
        else if(((m_data_address - 0x1800)/12 + 1) == 24)
        {

             //considering
            m_data_address = 0xF600 - 12;

            Log() << "start qc address";
        }
        else
        {
            if(((m_data_address - 0xF600)/12 + 1) == 1)
            {
                Log() << "F/W date";
                tempstring = QString().sprintf("%c%c%c%c%c%c%c%c%c%c%c%c",
                    (rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F), (rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F),
                    (rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F), (rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F),
                    (rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F), (rcvPacket[16] & 0x0F)*16 + (rcvPacket[17] & 0x0F),
                    (rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F), (rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F),
                    (rcvPacket[25] & 0x0F)*16 + (rcvPacket[26] & 0x0F), (rcvPacket[28] & 0x0F)*16 + (rcvPacket[29] & 0x0F),
                    (rcvPacket[31] & 0x0F)*16 + (rcvPacket[32] & 0x0F), (rcvPacket[34] & 0x0F)*16 + (rcvPacket[35] & 0x0F));
                m_settings.insert(SFD_q_fwdate, QVariant(tempstring));
            }
            else if(((m_data_address - 0xF600)/12 + 1) == 2)
            {
                Log() << "F/W Time";
                QString fwstring = QString().sprintf("%c%c%c%c%c%c%c%c%c%c%c%c",
                    (rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F), (rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F),
                    (rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F), (rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F),
                    (rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F), (rcvPacket[16] & 0x0F)*16 + (rcvPacket[17] & 0x0F),
                    (rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F), (rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F),
                    (rcvPacket[25] & 0x0F)*16 + (rcvPacket[26] & 0x0F), (rcvPacket[28] & 0x0F)*16 + (rcvPacket[29] & 0x0F),
                    (rcvPacket[31] & 0x0F)*16 + (rcvPacket[32] & 0x0F), (rcvPacket[34] & 0x0F)*16 + (rcvPacket[35] & 0x0F));
                Log() << "fwstring = " << fwstring;
                m_settings.insert(SFD_q_fwtime, QVariant(fwstring));

            }
            else if(((m_data_address - 0xF600)/12 + 1) == 3)
            {
                Log() << "withoutIR0 ~ withIR5_1";
                temp_deci  = ((rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F))+ ((rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F)) * 256;
                m_settings.insert(SFD_q_withoutIR_0, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F)) + ((rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F)) * 256;
                m_settings.insert(SFD_q_withoutIR_1, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F)) + ((rcvPacket[16] & 0x0F)*16 + (rcvPacket[17] & 0x0F)) * 256;
                m_settings.insert(SFD_q_withoutIR_2, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F)) + ((rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F)) * 256;
                m_settings.insert(SFD_q_withoutIR_3, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[25] & 0x0F)*16 + (rcvPacket[26] & 0x0F)) + ((rcvPacket[28] & 0x0F)*16 + (rcvPacket[29] & 0x0F)) * 256;
                m_settings.insert(SFD_q_withIR_code5_1, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[31] & 0x0F)*16 + (rcvPacket[32] & 0x0F)) + ((rcvPacket[34] & 0x0F)*16 + (rcvPacket[35] & 0x0F)) * 256;
                m_settings.insert(SFD_q_withIR_code5_2, QVariant(temp_deci));

            }
            else if(((m_data_address - 0xF600)/12 + 1) == 4)
            {
                Log() << "withIR_code5_3~ q_withIR_code10_4";
                temp_deci  = ((rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F))+ ((rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F)) * 256;
                m_settings.insert(SFD_q_withIR_code5_3, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F)) + ((rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F)) * 256;
                m_settings.insert(SFD_q_withIR_code5_4, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F)) + ((rcvPacket[16] & 0x0F)*16 + (rcvPacket[17] & 0x0F)) * 256;
                m_settings.insert(SFD_q_withIR_code10_1, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F)) + ((rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F)) * 256;
                m_settings.insert(SFD_q_withIR_code10_2, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[25] & 0x0F)*16 + (rcvPacket[26] & 0x0F)) + ((rcvPacket[28] & 0x0F)*16 + (rcvPacket[29] & 0x0F)) * 256;
                m_settings.insert(SFD_q_withIR_code10_3, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[31] & 0x0F)*16 + (rcvPacket[32] & 0x0F)) + ((rcvPacket[34] & 0x0F)*16 + (rcvPacket[35] & 0x0F)) * 256;
                m_settings.insert(SFD_q_withIR_code10_4, QVariant(temp_deci));
            }
            else if(((m_data_address - 0xF600)/12 + 1) == 5)
            {
                Log() << "error code";
                temp_deci  = ((rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F))+ ((rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F)) * 256;
                m_settings.insert(SFD_q_current_data, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F)) + ((rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F)) * 256;
                m_settings.insert(SFD_q_temp_data, QVariant(temp_deci));

                int errvalue  = ((rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F)) + ((rcvPacket[22] & 0x0F)*16 + (rcvPacket[11] & 0x0F)) * 256;
                temp_deci = errvalue;

                QList<QString> errcheck;
                errcheck.append("O");
                errcheck.append("X");

                QList<int> errresult;
                for(int i = 0; i < 4; i++)
                {
                    errresult.append(temp_deci % 2);
                    temp_deci = temp_deci / 2;
                }

                if(errvalue != 1)
                {
                    m_settings.insert(SFD_q_errcode_8, QVariant(errcheck[errresult.at(3)]));
                    m_settings.insert(SFD_q_errcode_4, QVariant(errcheck[errresult.at(2)]));
                    m_settings.insert(SFD_q_errcode_2, QVariant(errcheck[errresult.at(1)]));
                    m_settings.insert(SFD_q_errcode_1, QVariant(errcheck[errresult.at(0)]));
                }
                else
                {
                    m_settings.insert(SFD_q_errcode_8, QVariant("."));
                    m_settings.insert(SFD_q_errcode_4, QVariant("."));
                    m_settings.insert(SFD_q_errcode_2, QVariant("."));
                    m_settings.insert(SFD_q_errcode_1, QVariant("X"));
                }

                temp_deci  = ((rcvPacket[25] & 0x0F)*16 + (rcvPacket[26] & 0x0F)) + ((rcvPacket[28] & 0x0F)*16 + (rcvPacket[29] & 0x0F)) * 256;
                m_settings.insert(SFD_q_r1, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[31] & 0x0F)*16 + (rcvPacket[32] & 0x0F)) + ((rcvPacket[34] & 0x0F)*16 + (rcvPacket[35] & 0x0F)) * 256;
                m_settings.insert(SFD_q_r2, QVariant(temp_deci));
            }
            else if(((m_data_address - 0xF600)/12 + 1) == 6)
            {
                Log() << "R3~P2P";
                temp_deci  = ((rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F))+ ((rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F)) * 256;
                m_settings.insert(SFD_q_r3, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F))+ ((rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F)) * 256;
                m_settings.insert(SFD_q_r4, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F))+ ((rcvPacket[16] & 0x0F)*16 + (rcvPacket[17] & 0x0F)) * 256;
                m_settings.insert(SFD_q_r5, QVariant(temp_deci));

                temp_deci  = ((rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F)) + ((rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F)) * 256;
                m_settings.insert(SFD_q_niso_qc_p623, QVariant(temp_deci));
                Log() << "niso_qc_p623 = " << temp_deci;
                temp_deci  = ((rcvPacket[25] & 0x0F)*16 + (rcvPacket[26] & 0x0F)) + ((rcvPacket[28] & 0x0F)*16 + (rcvPacket[29] & 0x0F)) * 256;
                m_settings.insert(SFD_q_niso_qc_p628, QVariant(temp_deci));
                Log() << "niso_qc_p628 = " << temp_deci;
                temp_deci  = (((rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F)) + ((rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F)) * 256) -
                        (((rcvPacket[25] & 0x0F)*16 + (rcvPacket[26] & 0x0F)) + ((rcvPacket[28] & 0x0F)*16 + (rcvPacket[29] & 0x0F)) * 256);
                Log() << "niso_qc_pDiff = " << temp_deci;
                m_settings.insert(SFD_q_niso_qc_pDiff, QVariant(temp_deci));

            }
            else if(((m_data_address - 0xF600)/12 + 1) == 7)
            {
                Log() << "error square sum";
                temp_deci  = ((rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F)) + ((rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F)) * 256;
                m_settings.insert(SFD_q_square_sum, QVariant(temp_deci));
                m_data_address = 0xf998 - 12;

            }
            else if(m_data_address == 0xf998)
            {
                Log() << "F/W Info";
                QString fwversion = "";
                if((rcvPacket[16] & 0x0F) == 0xF &&(rcvPacket[17] & 0x0F) == 0x0)
                {
                    //fw 4자리
                    fwversion = QString().sprintf("%d %d %d %d",
                        (rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F) + (rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F),
                        (rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F) +  (rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F),
                        (rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F),
                        (rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F) +  (rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F)
                            );
                }
                else
                {
                    //기존 3자리
                    fwversion = QString().sprintf("%d %d %d",
                        (rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F) + (rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F),
                        (rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F) +  (rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F),
                        (rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F) + (rcvPacket[16] & 0x0F)*16 + (rcvPacket[17] & 0x0F)
                            );
                }


                m_settings.insert(SFD_q_fw_version, QVariant(fwversion));
            }
            else
            {
                Log()<< "not yet";
            }
        }

        m_data_address += 12;
        requestNextQcData();
    }
}


void SerialProtocol3::parseQcData_default(QByteArray rcvPacket)
{
    int temp_deci = 0;

    if(m_data_address == 0xf998 + 12)
    {
        Log() << "End";
        timerStop();
        Log() << m_settings.value(SFD_q_serialnumber).toString();
        Log() << m_settings.value(SFD_q_new_serialnumber).toString();
        Settings::Instance()->setSettings(m_settings);
        pLastCommand = lastCommand;
        lastCommand = Sp::ProtocolCommandNone;
        emit finishReadingQcData();
    }
    else
    {
        if(m_data_address < 0xF600 - 12)
        {
            if(((m_data_address - 0x1000)/12 + 1) == 1)
            {
                int nAvRef = ((rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F))+ ((rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F)) * 256;
                m_settings.insert(SFD_q_avref, QVariant(nAvRef));
                int nDacCalWork = ((rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F)) + ((rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F)) * 256;
                m_settings.insert(SFD_q_daccalwork, QVariant(nDacCalWork));
                int nDacCalThird = ((rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F)) + ((rcvPacket[16] & 0x0F)*16 + (rcvPacket[17] & 0x0F)) * 256;
                m_settings.insert(SFD_q_daccalthird, QVariant(nDacCalThird));
                Log() << "nAvRef = " << nAvRef << " nDacCalWork = " << nDacCalWork << " nDacCalThird" << nDacCalThird;
                Log() << "nAvRef setting = " << m_settings.value(SFD_q_avref).toString();

            }
            else if(((m_data_address - 0x1000)/12 + 1) == 2)
            {
                QByteArray temparr;
                temparr.append(((rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F)));
                temparr.append(((rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F)));
                temparr.append(((rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F)));
                temparr.append(((rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F)));

                float gainslope = bytesToFloat(temparr[3], temparr[2], temparr[1], temparr[0]);
                m_settings.insert(SFD_q_gainslope, QVariant(QString().sprintf("%.3f", gainslope)));

                temparr.clear();
                temparr.append(((rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F)));
                temparr.append(((rcvPacket[16] & 0x0F)*16 + (rcvPacket[17] & 0x0F)));
                temparr.append(((rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F)));
                temparr.append(((rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F)));

                float gainYcut = bytesToFloat(temparr[3], temparr[2], temparr[1], temparr[0]);
                m_settings.insert(SFD_q_gainYcut, QVariant(QString().sprintf("%.3f", gainYcut)));
                //0x1014
                int unit = (qint8(rcvPacket[25] & 0x0F) * 16) + qint8(rcvPacket[26] & 0x0F);
                m_settings.insert(SFD_q_unitset, unit);
                //0x1015
                int temp = ((rcvPacket[28] & 0x0F) * 16) + (rcvPacket[29] & 0x0F);
                m_settings.insert(SFD_q_tempset, temp);
                //0x1016
                int dateFormat =( (rcvPacket[31] & 0x0F) * 16) + (rcvPacket[32] & 0x0F);
                m_settings.insert(SFD_q_dateformat, dateFormat);
                //0x1017
                //start year
                int startyear =( (rcvPacket[34] & 0x0F) * 16) + (rcvPacket[35] & 0x0F) + 2000;
                m_settings.insert(SFD_q_startyear, startyear);

                Log() << "gainslope = " << gainslope << " gainYcut = " << gainYcut << " unit" << unit;

                Log() << "temp = " << temp << " dateFormat = " << dateFormat << " unit" << unit;

            }
            else if(((m_data_address - 0x1000)/12 + 1) == 3)
            {
                //0x1018~0x1023
                //end year 0x1018
                Log() << "end year";
                int endyear =( (rcvPacket[1] & 0x0F) * 16) + (rcvPacket[2] & 0x0F) + 2000;
                m_settings.insert(SFD_q_endyear, endyear);
                //memory count(2bytes) 0x1019-0x1020
                int memory_count = ((rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F)) + ((rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F)) * 256;
                m_settings.insert(SFD_q_memory_count, memory_count);

                //ble module set 0x1022
                //bleway 0 - pass key 1 - just works 2 - disabled
                int bleway = 0;
                if((rcvPacket[31] & 0x0F) == 0xD &&(rcvPacket[32] & 0x0F) == 0x0D)
                {
                    //disabled
                    bleway = 2;
                }
                else
                {
                    bleway = ((rcvPacket[31] & 0x0F) * 16) + (rcvPacket[32] & 0x0F);
                }

                m_settings.insert(SFD_q_bleway, bleway);

            }
            else if(((m_data_address - 0x1000)/12 + 1) == 4)
            {
                //0x1024~0x102F
                //0x1024 temprature_range_low_lowbyte
                //0x1025 temprature_range_low_highbyte
                int temp = ((rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F)) + ((rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F)) * 256;
                m_settings.insert(SFD_q_temperature_range_low, temp / 10);

                //0x1026 temprature_range_high_lowhbyte
                //0x1027 temprature_range_high_highbyte
                temp = ((rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F)) + ((rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F)) * 256;
                m_settings.insert(SFD_q_temperature_range_high, temp / 10);

                //0x1028 measure_range_low_lowhbyte
                //0x1029 measure_range_low_highbyte
                temp = ((rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F)) + ((rcvPacket[16] & 0x0F)*16 + (rcvPacket[17] & 0x0F)) * 256;
                m_settings.insert(SFD_q_measure_range_low, temp + 1);

                //0x102A measure_range_high_lowhbyte
                //0x102B measure_range_high_highbyte
                temp = ((rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F)) + ((rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F)) * 256;
                m_settings.insert(SFD_q_measure_range_high, temp - 1);

                //0x102C hypo_initial_lowhbyte
                //0x102D hypo_initial_highbyte
                temp = ((rcvPacket[25] & 0x0F)*16 + (rcvPacket[26] & 0x0F)) + ((rcvPacket[28] & 0x0F)*16 + (rcvPacket[29] & 0x0F)) * 256;
                m_settings.insert(SFD_q_hypo_initial, temp);

                //0x102E hyper_initial_lowhbyte
                //0x102F hyper_initial__highbyte
                temp = ((rcvPacket[31] & 0x0F)*16 + (rcvPacket[32] & 0x0F)) + ((rcvPacket[34] & 0x0F)*16 + (rcvPacket[35] & 0x0F)) * 256;
                m_settings.insert(SFD_q_hyper_initial, temp);

            }
            else if(((m_data_address - 0x1000)/12 + 1) == 5)
            {
                //0x1030~103B
                //0x1030 hypo_on_off
                int temp = ((rcvPacket[1] & 0x0F) * 16) + (rcvPacket[2] & 0x0F);
                m_settings.insert(SFD_q_hypo_on_off, temp);

                //0x1031 hyper_on_off
                temp = ((rcvPacket[4] & 0x0F) * 16) + (rcvPacket[5] & 0x0F);
                m_settings.insert(SFD_q_hyper_on_off, temp);

                //0x1032 buzzer_on_off
                temp = ((rcvPacket[7] & 0x0F) * 16) + (rcvPacket[8] & 0x0F);
                m_settings.insert(SFD_q_buzzer_on_off, temp);

                //0x1033 Protocol version
                temp = ((rcvPacket[10] & 0x0F) * 16) + (rcvPacket[11] & 0x0F);
                m_settings.insert(SFD_g_strip_type_protocol_version, temp);

                //0x1034 Use Barozen
                temp = ((rcvPacket[13] & 0x0F) * 16) + (rcvPacket[14] & 0x0F);
                m_settings.insert(SFD_g_strip_type_barozen, temp);

                //0x1035 Use CareSens Pro
                temp = ((rcvPacket[16] & 0x0F) * 16) + (rcvPacket[17] & 0x0F);
                m_settings.insert("g_strip_type_cspro", temp);

                //0x1036 Use CareSens Pro Ketone
                temp = ((rcvPacket[19] & 0x0F) * 16) + (rcvPacket[20] & 0x0F);
                m_settings.insert("g_strip_type_cspro_k", temp);

                //0x1037 Use Barozen Ketone
                temp = ((rcvPacket[22] & 0x0F) * 16) + (rcvPacket[23] & 0x0F);
                m_settings.insert(SFD_g_strip_type_barozen_k, temp);
            }

            else if(((m_data_address - 0x1000)/12 + 1) == 11)
            {
                //0x1081
                int codevalue = ((rcvPacket[28] & 0x0F) * 16) + (rcvPacket[29] & 0x0F);
                m_settings.insert(SFD_q_codevalue, QVariant(codevalue));
                //0x1082
                int debugflag = ((rcvPacket[31] & 0x0F) * 16) + (rcvPacket[32] & 0x0F);
                if(debugflag == 0)
                {
                     m_settings.insert(SFD_q_debugflag, QVariant("OFF"));
                }
                else if(debugflag == 1)
                {
                    m_settings.insert(SFD_q_debugflag, QVariant("ON(1)"));
                }
                else if(debugflag == 2)
                {
                    m_settings.insert(SFD_q_debugflag, QVariant("ON(2)"));
                }
                else if(debugflag == 3)
                {
                    m_settings.insert(SFD_q_debugflag, QVariant("불량"));
                }
                else
                {
                    m_settings.insert(SFD_q_debugflag, QVariant("N/A"));
                }
                m_settings.insert(SFD_q_debugflag_upper, QVariant(rcvPacket[31]));
                m_settings.insert(SFD_q_debugflag_lower, QVariant(rcvPacket[32]));

                //0x1083
                int hourmodeflag = ((rcvPacket[34] & 0x0F) * 16) + (rcvPacket[35] & 0x0F);
                m_settings.insert(SFD_q_hourmodeflag, QVariant(hourmodeflag));
                m_settings.insert(SFD_q_hourmodeflag_upper, QVariant(rcvPacket[34]));
                m_settings.insert(SFD_q_hourmodeflag_lower, QVariant(rcvPacket[35]));


                Log() << "debugflag = " << debugflag;

            }
            else if(((m_data_address - 0x100B)/12 + 1) == 11)
            {
                //0x1084

                if (m_settings.value(SFD_meter_type).toInt() != 0) //TI가 아닐때만 셋팅
                {
                    //0x108B
                    int volume = ((rcvPacket[22] & 0x0F) * 16) + (rcvPacket[23] & 0x0F);
                    m_settings.insert(SFD_q_setaudiovolume, QVariant(volume));

                    //0x108C
                    int audiomode = ((rcvPacket[25] & 0x0F) * 16) + (rcvPacket[26] & 0x0F);
                    m_settings.insert(SFD_q_setaudiomode, QVariant(audiomode));

                    //0x108D
                    int langcount = ((rcvPacket[28] & 0x0F) * 16) + (rcvPacket[29] & 0x0F);
                    m_settings.insert(SFD_q_setaudiolangcount, QVariant(langcount));
                }

                //TI, voice
                if (m_settings.value(SFD_meter_type).toInt() == 0 && Settings::Instance()->isSetAudioSettings())
                {
                       m_data_address += 12*7;
                }
                else
                {
                    m_data_address = 0xF600 - 12;
                }

                Log() << "start qc address";
            }
            else if(((m_data_address - 0x100B)/12 + 1) == 19)  //0x10E3
            {
                //0x10E8
                int volume = ((rcvPacket[13] & 0x0F) * 16) + (rcvPacket[14] & 0x0F);
                m_settings.insert(SFD_q_setaudiovolume, QVariant(volume));
                //0x10E9
                int audiomode = ((rcvPacket[16] & 0x0F) * 16) + (rcvPacket[17] & 0x0F);
                m_settings.insert(SFD_q_setaudiomode, QVariant(audiomode));
                //0x10EA
                int langcount = ((rcvPacket[19] & 0x0F) * 16) + (rcvPacket[20] & 0x0F);
                m_settings.insert(SFD_q_setaudiolangcount, QVariant(langcount));

                m_data_address = 0xF600 - 12;
            }
        }
        else
        {
            if(((m_data_address - 0xF600)/12 + 1) == 1)
            {
                Log() << "F/W date";
                tempstring = QString().sprintf("%c%c%c%c%c%c%c%c%c%c%c%c",
                    (rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F), (rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F),
                    (rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F), (rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F),
                    (rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F), (rcvPacket[16] & 0x0F)*16 + (rcvPacket[17] & 0x0F),
                    (rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F), (rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F),
                    (rcvPacket[25] & 0x0F)*16 + (rcvPacket[26] & 0x0F), (rcvPacket[28] & 0x0F)*16 + (rcvPacket[29] & 0x0F),
                    (rcvPacket[31] & 0x0F)*16 + (rcvPacket[32] & 0x0F), (rcvPacket[34] & 0x0F)*16 + (rcvPacket[35] & 0x0F));
                m_settings.insert(SFD_q_fwdate, QVariant(tempstring));
            }
            else if(((m_data_address - 0xF600)/12 + 1) == 2)
            {
                Log() << "F/W Time";
                QString fwstring = QString().sprintf("%c%c%c%c%c%c%c%c%c%c%c%c",
                    (rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F), (rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F),
                    (rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F), (rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F),
                    (rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F), (rcvPacket[16] & 0x0F)*16 + (rcvPacket[17] & 0x0F),
                    (rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F), (rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F),
                    (rcvPacket[25] & 0x0F)*16 + (rcvPacket[26] & 0x0F), (rcvPacket[28] & 0x0F)*16 + (rcvPacket[29] & 0x0F),
                    (rcvPacket[31] & 0x0F)*16 + (rcvPacket[32] & 0x0F), (rcvPacket[34] & 0x0F)*16 + (rcvPacket[35] & 0x0F));
                Log() << "fwstring = " << fwstring;
                m_settings.insert(SFD_q_fwtime, QVariant(fwstring));

            }
            else if(((m_data_address - 0xF600)/12 + 1) == 3)
            {
                Log() << "LED OFF 1~ LED ON_Code5-2";
                temp_deci  = ((rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F))+ ((rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F)) * 256;
                m_settings.insert(SFD_q_ledoff_1, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F)) + ((rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F)) * 256;
                m_settings.insert(SFD_q_ledoff_2, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F)) + ((rcvPacket[16] & 0x0F)*16 + (rcvPacket[17] & 0x0F)) * 256;
                m_settings.insert(SFD_q_ledoff_3, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F)) + ((rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F)) * 256;
                m_settings.insert(SFD_q_ledoff_4, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[25] & 0x0F)*16 + (rcvPacket[26] & 0x0F)) + ((rcvPacket[28] & 0x0F)*16 + (rcvPacket[29] & 0x0F)) * 256;
                m_settings.insert(SFD_q_ledon_code5_1, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[31] & 0x0F)*16 + (rcvPacket[32] & 0x0F)) + ((rcvPacket[34] & 0x0F)*16 + (rcvPacket[35] & 0x0F)) * 256;
                m_settings.insert(SFD_q_ledon_code5_2, QVariant(temp_deci));

            }
            else if(((m_data_address - 0xF600)/12 + 1) == 4)
            {
                Log() << "LED ON_Code5-3~ LED ON Code10-4";
                temp_deci  = ((rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F))+ ((rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F)) * 256;
                m_settings.insert(SFD_q_ledon_code5_3, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F)) + ((rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F)) * 256;
                m_settings.insert(SFD_q_ledon_code5_4, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F)) + ((rcvPacket[16] & 0x0F)*16 + (rcvPacket[17] & 0x0F)) * 256;
                m_settings.insert(SFD_q_ledon_code10_1, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F)) + ((rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F)) * 256;
                m_settings.insert(SFD_q_ledon_code10_2, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[25] & 0x0F)*16 + (rcvPacket[26] & 0x0F)) + ((rcvPacket[28] & 0x0F)*16 + (rcvPacket[29] & 0x0F)) * 256;
                m_settings.insert(SFD_q_ledon_code10_3, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[31] & 0x0F)*16 + (rcvPacket[32] & 0x0F)) + ((rcvPacket[34] & 0x0F)*16 + (rcvPacket[35] & 0x0F)) * 256;
                m_settings.insert(SFD_q_ledon_code10_4, QVariant(temp_deci));
            }
            else if(((m_data_address - 0xF600)/12 + 1) == 5)
            {
                Log() << "error code";
                temp_deci  = ((rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F))+ ((rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F)) * 256;
                m_settings.insert(SFD_q_current_data, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F)) + ((rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F)) * 256;
                m_settings.insert(SFD_q_temp_data, QVariant(temp_deci));

                int errvalue  = ((rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F)) + ((rcvPacket[22] & 0x0F)*16 + (rcvPacket[11] & 0x0F)) * 256;
                temp_deci = errvalue;

                QList<QString> errcheck;
                errcheck.append("O");
                errcheck.append("X");

                QList<int> errresult;
                for(int i = 0; i < 4; i++)
                {
                    errresult.append(temp_deci % 2);
                    temp_deci = temp_deci / 2;
                }

                if(errvalue != 1)
                {
                    m_settings.insert(SFD_q_errcode_8, QVariant(errcheck[errresult.at(3)]));
                    m_settings.insert(SFD_q_errcode_4, QVariant(errcheck[errresult.at(2)]));
                    m_settings.insert(SFD_q_errcode_2, QVariant(errcheck[errresult.at(1)]));
                    m_settings.insert(SFD_q_errcode_1, QVariant(errcheck[errresult.at(0)]));
                }
                else
                {
                    m_settings.insert(SFD_q_errcode_8, QVariant("."));
                    m_settings.insert(SFD_q_errcode_4, QVariant("."));
                    m_settings.insert(SFD_q_errcode_2, QVariant("."));
                    m_settings.insert(SFD_q_errcode_1, QVariant("X"));
                }

                temp_deci  = ((rcvPacket[25] & 0x0F)*16 + (rcvPacket[26] & 0x0F)) + ((rcvPacket[28] & 0x0F)*16 + (rcvPacket[29] & 0x0F)) * 256;
                m_settings.insert(SFD_q_current_outputCal_verify_1, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[31] & 0x0F)*16 + (rcvPacket[32] & 0x0F)) + ((rcvPacket[34] & 0x0F)*16 + (rcvPacket[35] & 0x0F)) * 256;
                m_settings.insert(SFD_q_current_outputCal_verify_2, QVariant(temp_deci));
            }
            else if(((m_data_address - 0xF600)/12 + 1) == 6)
            {
                Log() << "OutputVerify2~ 623/628/DIFF";
                temp_deci  = ((rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F))+ ((rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F)) * 256;
                m_settings.insert(SFD_q_current_outputCal_verify_3, QVariant(temp_deci));
                // R4, R5
                temp_deci  = ((rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F))+ ((rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F)) * 256;
                m_settings.insert(SFD_q_current_outputCal_verify_4, QVariant(temp_deci));
                temp_deci  = ((rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F))+ ((rcvPacket[16] & 0x0F)*16 + (rcvPacket[17] & 0x0F)) * 256;
                m_settings.insert(SFD_q_current_outputCal_verify_5, QVariant(temp_deci));
                //
                float osc_fault = (rcvPacket[7] & 0x0F) * 16 + (rcvPacket[8] & 0x0F) + (rcvPacket[10] & 0x0F) * pow(16.0,3) + (rcvPacket[11] & 0x0F) * pow(16.0,2)
                        + (rcvPacket[13] & 0x0F) * pow(16.0,5) + (rcvPacket[14] & 0x0F) * pow(16.0,4) + (rcvPacket[16] & 0x0F) * pow(16.0,7) + (rcvPacket[17] & 0x0F) * pow(16.0,6);
                Log() << QString().sprintf("%f", osc_fault);
                m_settings.insert(SFD_q_osc_fault, QVariant(QString().sprintf("%f", osc_fault)));
                Log() << "osc_fault = " << osc_fault;
                temp_deci  = ((rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F)) + ((rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F)) * 256;
                m_settings.insert(SFD_q_niso_qc_p623, QVariant(temp_deci));
                Log() << "niso_qc_p623 = " << temp_deci;
                temp_deci  = ((rcvPacket[25] & 0x0F)*16 + (rcvPacket[26] & 0x0F)) + ((rcvPacket[28] & 0x0F)*16 + (rcvPacket[29] & 0x0F)) * 256;
                m_settings.insert(SFD_q_niso_qc_p628, QVariant(temp_deci));
                Log() << "niso_qc_p628 = " << temp_deci;
                //bmwe3 160307 서민우 과장님 요청으로 계산 대신 직접 읽게 수정함.
                /*
                temp_deci  = (((rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F)) + ((rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F)) * 256) -
                        (((rcvPacket[25] & 0x0F)*16 + (rcvPacket[26] & 0x0F)) + ((rcvPacket[28] & 0x0F)*16 + (rcvPacket[29] & 0x0F)) * 256);
                Log() << "old p2p" << temp_deci;
                */
                temp_deci  = (((rcvPacket[31] & 0x0F)*16 + (rcvPacket[32] & 0x0F)) + ((rcvPacket[34] & 0x0F)*16 + (rcvPacket[35] & 0x0F)) * 256);
                Log() << "direct/q_niso_qc_pDiff = " << temp_deci;
                m_settings.insert(SFD_q_niso_qc_pDiff, QVariant(temp_deci));

            }
            else if(((m_data_address - 0xF600)/12 + 1) == 7)
            {
                Log() << "error square sum";
                temp_deci  = ((rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F)) + ((rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F)) * 256;
                m_settings.insert(SFD_q_square_sum, QVariant(temp_deci));
                m_data_address = 0xf998 - 12;

            }
            else if(m_data_address == 0xf998)
            {
                Log() << "F/W Info";
                QString fwversion = "";

                //if((rcvPacket[16] & 0x0F)*16 + (rcvPacket[17] & 0x0F) == 240)
                // **** **** **** **** F0
                if((rcvPacket[16] & 0x0F) == 0x0F && (rcvPacket[17] & 0x0F) == 0x00)
                {
                    //fw 4자리
                    fwversion = QString().sprintf("%d %d %d %d",
                        (rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F) + (rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F),
                        (rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F) +  (rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F),
                        (rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F),
                        (rcvPacket[19] & 0x0F)*16 + (rcvPacket[20] & 0x0F) +  (rcvPacket[22] & 0x0F)*16 + (rcvPacket[23] & 0x0F)
                            );
                }
                // **** **** **** **** F1 // VE
                else if((rcvPacket[16] & 0x0F) == 0x0F && (rcvPacket[17] & 0x0F) == 0x01)
                {
                    //fw 4자리
                    fwversion = QString().sprintf("%d %d %d %d",
                        (rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F) + (rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F),
                        (rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F),
                        (rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F),
                        (rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F)
                            );
                }
                else
                {
                    //기존 3자리
                    fwversion = QString().sprintf("%d %d %d",
                        (rcvPacket[1] & 0x0F)*16 + (rcvPacket[2] & 0x0F) + (rcvPacket[4] & 0x0F)*16 + (rcvPacket[5] & 0x0F),
                        (rcvPacket[7] & 0x0F)*16 + (rcvPacket[8] & 0x0F) +  (rcvPacket[10] & 0x0F)*16 + (rcvPacket[11] & 0x0F),
                        (rcvPacket[13] & 0x0F)*16 + (rcvPacket[14] & 0x0F) + (rcvPacket[16] & 0x0F)*16 + (rcvPacket[17] & 0x0F)
                            );
                }


                m_settings.insert(SFD_q_fw_version, QVariant(fwversion));
            }
            else
            {
                Log()<< "not yet";
            }
        }

        m_data_address += 12;
        requestNextQcData();
    }
}


void SerialProtocol3::readyRead() {
    timerStop();
    bufferMutex.lock();
    QByteArray readData = comm->readAll();
    if(lastCommand != Sp::ProtocolCommandNone) {
        if(readData.count())
        {
            try {
                readBuffer.append(readData);
            } catch (...) {
                return;
            }
        }

        Log() << "readBuffer" << readBuffer.toHex();
        Log() << "readBuffer len" << readBuffer.length();

#ifdef SERIALCOM_SMARTLOG
        int flushIndex = 0;
         while(readBuffer.at(flushIndex) != kSTX) {
             flushIndex ++;
             if(flushIndex > readBuffer.count()) {
                 flushIndex  = readBuffer.count();
                 break;
             }
         }
         readBuffer.remove(0, flushIndex);

         // STM32_DEBUG
         // Sp::ReadQcData 읽다 말고 멈춤 1 (회색화면)
         Log() << "readBuffer.length()1" << readBuffer.length();
         bufferMutex.unlock();

         Log() << "readBuffer.length()2" << readBuffer.length();
         if(readBuffer.length() < 1 || (int)readBuffer[readBuffer.length() - 1] != kETX)
         {
             timerStart();
             return;
         }
         Log();
         if(isCompletePacketReceved()) {
             QByteArray rcvPacket = getReceivePacket();
             lastRcvPacket.remove(0, lastRcvPacket.count());
             lastRcvPacket.append(rcvPacket);
             processPacket(rcvPacket);
         } else {
             timerStart();
         }
#endif

#ifdef SERIALCOM_QC
#ifdef FROM_METERMEMORY
        if(lastCommand == Sp::ReadBLE)
        {
            if(readBuffer.length() >= 11)
            {
//                int namesize = readBuffer[5] - 9;
//                QString blename = INVALID_BLE;
                if(readBuffer.contains("RBLE"))
                {
                    Settings::Instance()->setBleMeter(1);
//                    blename = QString(readBuffer.mid(11,namesize));
                }
                else {
                    Settings::Instance()->setBleMeter(0);
                }
                bufferMutex.unlock();
                pLastCommand = lastCommand;
                lastCommand = Sp::ProtocolCommandNone;
                emit finishReadingQcData();
            }
            else
            {
                bufferMutex.unlock();
                return;
            }
        }
        else if(lastCommand == Sp::SaveData)
        {
            if(readBuffer.contains("SVDT"))
            {
                pLastCommand = lastCommand;
                lastCommand = Sp::ProtocolCommandNone;
                emit finishDoCommands(true, m_commands[m_current_cmd_index - 1]);
                bufferMutex.unlock();
            }
            else
            {
                bufferMutex.unlock();
                return;
            }
        }
        else if(lastCommand == Sp::DeleteData)
        {
            if(readBuffer.contains("DELD"))
            {
                pLastCommand = lastCommand;
                lastCommand = Sp::ProtocolCommandNone;
                emit finishDoCommands(true, m_commands[m_current_cmd_index - 1]);
                bufferMutex.unlock();
            }
            else
            {
                bufferMutex.unlock();
                return;
            }
        }
        else
#endif
        if(lastCommand == Sp::ReadQcData)
        {
            if(readBuffer.length() >= 36)
            {
                lastRcvPacket.remove(0, readBuffer.count());
                lastRcvPacket.append(readBuffer);
                parseQcData(readBuffer);
                readBuffer.remove(0, readBuffer.count());
                bufferMutex.unlock();
            }
            else
            {
                bufferMutex.unlock();
                return;
            }
        }
        else if(lastCommand == Sp::Set91)
        {
            if(readBuffer[0] == (char)0x92 ||
                    readBuffer[0] == (char)0x94)
            {
                Settings::Instance()->setQcStringValue(SFD_q_output_cal, "PASS");
            }
            else
            {
                Settings::Instance()->setQcStringValue(SFD_q_output_cal, "FAIL");
            }
            emit finishDoCommands(true, m_commands[m_current_cmd_index - 1]);

            bufferMutex.unlock();

        }
        else if(lastCommand == Sp::Set9A01 ||
                lastCommand == Sp::Set9A02 ||
                lastCommand == Sp::Set9A03)
        {
            if(readBuffer[2] == (char)0x21)
            {
                Settings::Instance()->setQcStringValue(SFD_q_output_verification, "PASS");
            }
            else
            {
                Settings::Instance()->setQcStringValue(SFD_q_output_verification, "FAIL");
            }
            emit finishDoCommands(true, m_commands[m_current_cmd_index - 1]);

            bufferMutex.unlock();

        }
        else if(lastCommand == Sp::ReadSetHypo)
        {
            if(readBuffer.length() >= 3)
            {
                int value = (qint8(readBuffer[1] & 0x0F) * 16) + qint8(readBuffer[2] & 0x0F);
                Log() << "Hypo : " << value;
                Settings::Instance()->setQcStringValue(SFD_q_sethypo, QString().sprintf("%d", value));
                emit finishDoCommands(true, m_commands[m_current_cmd_index - 1]);
                bufferMutex.unlock();
            }
            else
            {
                bufferMutex.unlock();
                return;
            }
        }
        else if(lastCommand == Sp::ReadAnimalType)
        {
            if(readBuffer.length() >= 3)
            {
                int value = (qint8(readBuffer[1] & 0x0F) * 16) + qint8(readBuffer[2] & 0x0F);
                Log() << "Animal Type : " << value;
                m_settings.insert(SFD_q_animal_type, QVariant(value));
                emit finishDoCommands(true, m_commands[m_current_cmd_index - 1]);
                readBuffer.remove(0, readBuffer.count());
                bufferMutex.unlock();
            }
            else
            {
                bufferMutex.unlock();
                return;
            }
        }
        else if(readBuffer[0] == (char)0xe0)
        {
            //get glucose data flag
            if(readBuffer.length() >= 9)
            {
                int temp = qint8(readBuffer[3] & 0x0F);
                Settings::Instance()->setQcStringValue(SFD_q_flag_default, QString().sprintf("%d", temp));
                //nomark
                temp = qint8(readBuffer[4] & 0x0F);
                Settings::Instance()->setQcStringValue(SFD_q_flag_nomark, QString().sprintf("%d", temp));
                //premeal
                temp = qint8(readBuffer[5] & 0x0F);
                Settings::Instance()->setQcStringValue(SFD_q_flag_premeal, QString().sprintf("%d", temp));
                //postmeal
                temp = qint8(readBuffer[6] & 0x0F);
                Settings::Instance()->setQcStringValue(SFD_q_flag_postmeal, QString().sprintf("%d", temp));
                //fasting
                temp = qint8(readBuffer[7] & 0x0F);
                Settings::Instance()->setQcStringValue(SFD_q_flag_fasting, QString().sprintf("%d", temp));

                emit finishDoCommands(true, m_commands[m_current_cmd_index - 1]);
                readBuffer.remove(0, readBuffer.count());
                bufferMutex.unlock();
            }
            else
            {
                bufferMutex.unlock();
                return;
            }
        }
        else if(readBuffer[0] == (char)0xe1)
        {
            //set glucose data flag
            if(readBuffer.length() >= 4)
            {
                if(readBuffer[2] == (char)0x01)
                {
                    Settings::Instance()->setQcStringValue(SFD_q_setglucosedataflag, "SUCCESS");
                }
                else
                {
                    Settings::Instance()->setQcStringValue(SFD_q_setglucosedataflag, "FAIL");
                }

                readBuffer.remove(0, readBuffer.count());
                if(m_current_cmd_index < m_commands.length())
                {
                    //go ahead
                    requestCommand(m_commands[m_current_cmd_index]);
                }
                else
                {
                    emit finishDoCommands(true, m_commands[m_current_cmd_index - 1]);
                }
                bufferMutex.unlock();
            }
            else
            {
                bufferMutex.unlock();
                return;
            }
        }
        else if(readBuffer[0] == (char)0xf0)
        {
            //get average days
            if(readBuffer.length() >= 11)
            {
                int temp = qint8(readBuffer[3] & 0x0F);
                Settings::Instance()->setQcStringValue(SFD_q_enabled_avg_days_count, QString().sprintf("%d", temp));

                temp = qint8(readBuffer[4] & 0x0F);
                Settings::Instance()->setQcStringValue(SFD_q_flag_avg_days_1, QString().sprintf("%d", temp));
                //nomark
                temp = qint8(readBuffer[5] & 0x0F);
                Settings::Instance()->setQcStringValue(SFD_q_flag_avg_days_7, QString().sprintf("%d", temp));
                //premeal
                temp = qint8(readBuffer[6] & 0x0F);
                Settings::Instance()->setQcStringValue(SFD_q_flag_avg_days_14, QString().sprintf("%d", temp));
                //postmeal
                temp = qint8(readBuffer[7] & 0x0F);
                Settings::Instance()->setQcStringValue(SFD_q_flag_avg_days_30, QString().sprintf("%d", temp));
                //fasting
                temp = qint8(readBuffer[8] & 0x0F);
                Settings::Instance()->setQcStringValue(SFD_q_flag_avg_days_60, QString().sprintf("%d", temp));
                //fasting
                temp = qint8(readBuffer[9] & 0x0F);
                Settings::Instance()->setQcStringValue(SFD_q_flag_avg_days_90, QString().sprintf("%d", temp));

                emit finishDoCommands(true, m_commands[m_current_cmd_index - 1]);
                readBuffer.remove(0, readBuffer.count());
                bufferMutex.unlock();
            }
            else
            {
                bufferMutex.unlock();
                return;
            }
        }
        else if(readBuffer[0] == (char)0xf1)
        {
            //set average days
            if(readBuffer.length() >= 4)
            {
                if(readBuffer[2] == (char)0x01)
                {
                    Settings::Instance()->setQcStringValue(SFD_q_setavgdays_result, "SUCCESS");
                }
                else
                {
                    Settings::Instance()->setQcStringValue(SFD_q_setavgdays_result, "FAIL");
                }

                readBuffer.remove(0, readBuffer.count());
                if(m_current_cmd_index < m_commands.length())
                {
                    //go ahead
                    requestCommand(m_commands[m_current_cmd_index]);
                }
                else
                {
                    emit finishDoCommands(true, m_commands[m_current_cmd_index - 1]);
                }
                bufferMutex.unlock();
            }
            else
            {
                bufferMutex.unlock();
                return;
            }
        }
        else if(readBuffer[0] == (char)0x8c || readBuffer[0] == (char)0x8d)
        {
            if(readBuffer.length() >= 3)
            {

                lastRcvPacket.remove(0, readBuffer.count());
                lastRcvPacket.append(readBuffer);
                readBuffer.remove(0, readBuffer.count());
                Log() << "buffer len = " << readBuffer.count();
                if(m_current_cmd_index < m_commands.length())
                {
                    //go ahead
                    requestCommand(m_commands[m_current_cmd_index]);
                }
                else
                {
                    emit finishDoCommands(true, m_commands[m_current_cmd_index - 1]);
                }
                bufferMutex.unlock();
            }
            else
            {
                bufferMutex.unlock();
                return;
            }
        }
        else if(readBuffer[0] == (char)0x81)
        {
            Log() << "readBuffer" << readBuffer.toHex();
            Log() << "readBuffer len" << readBuffer.length();

            if(readBuffer.length() >= 8)
            {
                lastRcvPacket.remove(0, readBuffer.count());
                lastRcvPacket.append(readBuffer);
                timerStop();

                float currentTemperature = (((readBuffer[4] & 0x0f) * 4096) + ((readBuffer[5] & 0x0f) * 256) + ((readBuffer[6] & 0x0f) * 16) + (readBuffer[7] & 0x0f)) / 10.0;
                Settings::Instance()->setTemperature(QString().sprintf("%2.1f", currentTemperature));
                pLastCommand = lastCommand;
                lastCommand = Sp::ProtocolCommandNone;
                emit finishDoCommands(true, m_commands[m_current_cmd_index - 1]);
                readBuffer.remove(0, 8);
                bufferMutex.unlock();
            }
            else
            {
                bufferMutex.unlock();
                return;
            }
        }
        else
        {
            int flushIndex = 0;
            if(readBuffer.length() > 0)
            {

                while(flushIndex < readBuffer.count() && readBuffer.at(flushIndex) != kSTX)
                {
                    Log() << "flushIndex = " << flushIndex << " readBuffer.count() = " << readBuffer.count();
                    flushIndex ++;
                    if(flushIndex > readBuffer.count())
                    {
                        flushIndex  = readBuffer.count();
                        break;
                    }
                }
                readBuffer.remove(0, flushIndex);
            }
            else
            {
                bufferMutex.unlock();
                return;

            }

            // STM32_DEBUG
            // Sp::ReadQcData 읽다 말고 멈춤 1 (회색화면)
            Log() << "readBuffer.length()1" << readBuffer.length();

            bufferMutex.unlock();

            Log() << "readBuffer.length()2" << readBuffer.length();

            if(readBuffer.length() < 1 || (int)readBuffer[readBuffer.length() - 1] != kETX)
            {
                timerStart();
                return;
            }

            Log();

            if(isCompletePacketReceved())
            {
                QByteArray rcvPacket = getReceivePacket();
                lastRcvPacket.remove(0, lastRcvPacket.count());
                lastRcvPacket.append(rcvPacket);
                processPacket(rcvPacket);
            }
            else
            {
                timerStart();
            }
        }
#endif
    }
    else
    {
        bufferMutex.unlock();
        return;
    }
}

#if 0
void SerialProtocol3::error(QSerialPort::SerialPortError error) {
    timerStop();
    if(error != QSerialPort::NoError) {
        emit needReopenSerialComm();
    }
}
#endif


void SerialProtocol3::parseReceivedData(QByteArray rcvPacket)
{
    Sp::ProtocolCommand cmd = getCommand(rcvPacket);
    if(cmd == Sp::ReadSerialNumber)
    {
#ifdef SERIALCOM_SMARTLOG
        m_serialnumber = QString(rcvPacket.mid(11,12));
#endif
#ifdef SERIALCOM_QC
        Log() << "rcvPacket = " << QString(rcvPacket);
        if(rcvPacket.count() >= 23)
        {
            m_serialnumber = QString(rcvPacket.mid(11,12));
            if(m_serialnumber.length() == 12)
            {
                m_serialnumber.insert(8, " ");
                m_serialnumber.insert(4, " ");
                m_serialnumber.insert(2, " ");
            }
        }
        else
        m_serialnumber = "-----";
        m_settings.insert(SFD_q_serialnumber, QVariant(m_serialnumber));
        m_settings.insert(SFD_q_new_serialnumber, QVariant(""));
        Log() << m_settings.value(SFD_q_serialnumber).toString();
        Log() << m_settings.value(SFD_q_new_serialnumber).toString();
        Settings::Instance()->setSerialNumber(m_serialnumber, "");
#endif
    }
    else if(cmd == Sp::WriteTimeInformation) {
        QDateTime syncronizedDateTime = QDateTime(QDate(rcvPacket[10] + 2000, rcvPacket[11], rcvPacket[12]), QTime(rcvPacket[13], rcvPacket[14], rcvPacket[15]));
        qint64 syncronizedSeconds = syncronizedDateTime.toMSecsSinceEpoch();
        qint64 systemSyncSeconds = m_current_time.toMSecsSinceEpoch();

        if(labs(syncronizedSeconds - systemSyncSeconds) < 10000 ) { //+- 10초 이내
            emit completeTimeSync(&syncronizedDateTime, &m_current_time);
        } else {
            emit failTimeSync();
        }
    } else if(cmd == Sp::ReadTimeInformation) {
        int dataindex = 10;
        int year = rcvPacket[dataindex];
        int mon = rcvPacket[dataindex + 1];
        int day = rcvPacket[dataindex + 2];
        int hour = rcvPacket[dataindex + 3];
        int min = rcvPacket[dataindex + 4];
        int sec = rcvPacket[dataindex + 5];

        QDate date;// = QDate::currentDate();
        date.setDate(2000 + year, mon, day);
        QTime dtime;// = QTime::currentTime();
        dtime.setHMS(hour, min, sec, 0);

        QDateTime meterDateTime = QDateTime(date, dtime);
        Settings::Instance()->setMeterTime(meterDateTime.toString("yyyy-MM-dd hh:mm:ss"));
        //emit completeReadTime(&meterDateTime);
    }
    else if(cmd == Sp::GluecoseResultDataTxExpanded)
    {
        int count = getGluecoseCount(rcvPacket);
#ifdef SERIALCOM_QC
        downloadInfo.setDownloadedCount(count);
#endif

        for(int i = 1; i <= count; i++)
        {
            QJsonObject data;
            int dataindex = (i*9) + 1;
            int year = rcvPacket[dataindex];
            int mon = rcvPacket[dataindex + 1];
            int day = rcvPacket[dataindex + 2];
            int hour = rcvPacket[dataindex + 3];
            int min = rcvPacket[dataindex + 4];
            int sec = rcvPacket[dataindex + 5];

            QDate date;// = QDate::currentDate();

            date.setDate(2000 + year, mon, day);

            QTime dtime;// = QTime::currentTime();

            dtime.setHMS(hour, min, sec,0);

            QDateTime datatime;
            datatime.setDate(date);
            datatime.setTime(dtime);

            int value = (quint8(rcvPacket[dataindex + 7]) << 8) + quint8(rcvPacket[dataindex + 8]);
            data["glucose_data"] = QString().sprintf("%d", value);
            data["dDate"] = QString::number(datatime.toMSecsSinceEpoch());
            data["manual"] = m_serialnumber;
#ifdef SERIALCOM_SMARTLOG
            data["time"] = Settings::Instance()->GetDatetimestringFromMSec(QString::number(datatime.toMSecsSinceEpoch()));
            data["update_date_time"] = QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd hh:mm:ss");
//            data["flag_cs"] = "0";
//            data["flag_meal"] = "0";
//            data["flag_hilo"] = "0";
//            data["flag_fasting"] = "0";
//            data["flag_nomark"] = "0";
//            data["flag_ketone"] = "0";
#endif
#ifdef SERIALCOM_QC
            data["index"] = QString().sprintf("%d", downloadInfo.index());
            data["time"] = "";//Settings::Instance()->GetDatetimestringFromMSec(QString::number(datatime.toMSecsSinceEpoch()));
            data["flag_cs"] = "0";
            data["flag_meal"] = "0";
            data["flag_hilo"] = "0";
            data["flag_fasting"] = "0";
            data["flag_nomark"] = "0";
            data["flag_ketone"] = "0";
#endif
            data["glucose_unit"] = "mg/dL";

            int gflag = quint8(rcvPacket[dataindex + 6]);
            //data["gflag"] = QString().sprintf("%d", gflag);

            qDebug() << "data = " << value << " gflag = "<< gflag;

            if((gflag & 1) == 1)
            {
                data["flag_cs"] = "cs";
            }

//            if((gflag & 2) == 2)
//            {
//                data["flag_meal"] = "af";
//            }

            if((gflag & 4) == 4)
            {
                data["flag_hilo"] = "Lo";
            }

            if((gflag & 8) == 8)
            {
                data["flag_hilo"] = "Hi";
            }

            if((gflag & 16) == 16)
            {
                data["flag_fasting"] = "fs";
            }

            if((gflag & 32) == 32)
            {
                data["flag_nomark"] = "nm";
            }
            else
            {
                if((gflag & 2) == 2)
                {
                    data["flag_meal"] = "af";
                }
                else
                {
                    data["flag_meal"] = "bf";
                }
            }

            if((gflag & 64) == 64)
            {
                data["flag_ketone"] = "kt";
                data["glucose_unit"] = "mmol/L";
                data["glucose_data"] = QString().sprintf("%.1f", (double)value / 10);
            }

            //brazil index C0 exception
            if(m_serialnumber != "" && value == 10 &&
              m_serialnumber[0]== 'C' && m_serialnumber[1]== '0')
            {
                data["flag_hilo"] = "Lo"; // "lo" // SERIALCOM_QC
            }

#ifdef SERIALCOM_SMARTLOG
            m_dataArray.push_front(data);
#endif
#ifdef SERIALCOM_QC
            m_dataArray.push_back(data);
#endif
        }
    }
    else if(cmd == Sp::CurrentIndexOfGluecose)
    {        
        m_dataArray = QJsonArray();
        QJsonObject sn;
        sn["sn"] = m_serialnumber;
        m_dataArray.push_front(sn);

        downloadInfo.setNumberOfGluecose(getIndexOfGluecose(rcvPacket));

        quint16 NumOfGlu = getIndexOfGluecose(rcvPacket);

        Settings::Instance()->setNumberofCurrentGlucoseData(NumOfGlu);

        Log();
    }    
#ifdef SERIALCOM_QC
    else if(cmd == Sp::ReadBLE)
    {
        int namesize = rcvPacket[5] - 9;
        //size cal = 11,namesize - (4+2+1+2)
        QString blename = INVALID_BLE;
        if(rcvPacket.length() >= 11 + namesize)
        {
            blename = QString(rcvPacket.mid(11,namesize));
            //blename = QString(rcvPacket.mid(11,12));
        }
        Settings::Instance()->setBleName(blename);
    }
    else if(cmd == Sp::CommandPacketVerifyError)
    {
        //not implement
        if(isBleCmd == true)
        {
            Settings::Instance()->setBleName(INVALID_BLE);
        }
        else if(isDBLE == true)
        {
            lastCommand = Sp::TryChangeBLEMode;
            emit finishDoCommands(true, lastCommand);
        }
    }

    else if(cmd == Sp::ReadAESKey)
    {
        QString aeskey = QString(rcvPacket.mid(10,32).toHex());
        Log() << "aeskey = " << aeskey;
        m_settings.insert(SFD_q_aeskey, QVariant(aeskey));
        m_settings.insert(SFD_q_new_aeskey, QVariant(""));
        Log() << m_settings.value(SFD_q_aeskey).toString();
        Log() << m_settings.value(SFD_q_new_aeskey).toString();
        Settings::Instance()->setAESKey(aeskey, "");
    }
    else if(cmd == Sp::WriteAESKey)
    {
        Log();
        QString oldaeskey = m_settings.value(SFD_q_aeskey).toString();
        QString aeskey = QString(rcvPacket.mid(10,32).toHex());
        m_settings.insert(SFD_q_aeskey, QVariant(aeskey));
        m_settings.insert(SFD_q_new_aeskey, QVariant(oldaeskey));
        Log() << m_settings.value(SFD_q_aeskey).toString();
        Log() << m_settings.value(SFD_q_new_aeskey).toString();
        Settings::Instance()->setAESKey(aeskey, oldaeskey);
    }
    //set APN
    else if(cmd == Sp::ReadAPN || cmd == Sp::WriteAPN)
    {
        Log() << "rcvPacket: " << rcvPacket;
        Log() << "rcvPacket: " << rcvPacket.toHex();
        int keysize = rcvPacket[5] - 8;
        QString apnkey = QString(rcvPacket.mid(11,keysize));
        m_settings.insert(SFD_q_apnkey, QVariant(apnkey));
        Log() << m_settings.value(SFD_q_apnkey).toString();
        Settings::Instance()->setAPNKey(apnkey);
    }
    else if(cmd == Sp::ReadServerAddress || cmd == Sp::WriteServerAddress)
    {
        Log() << "rcvPacket: " << rcvPacket;
        Log() << "rcvPacket: " << rcvPacket.toHex();
        int keysize = rcvPacket[5] - 8;
        QString server_address = QString(rcvPacket.mid(11,keysize));
        m_settings.insert(SFD_q_server_address, QVariant(server_address));
        Log() << m_settings.value(SFD_q_server_address).toString();
        Settings::Instance()->setServerAddress(server_address);
    }
    else if(cmd == Sp::ReadDeviceToken || cmd == Sp::WriteDeviceToken)
    {
        Log() << "rcvPacket: " << rcvPacket;
        Log() << "rcvPacket: " << rcvPacket.toHex();
        int keysize = rcvPacket[5] - 8;
        QString device_token = QString(rcvPacket.mid(11,keysize));
        m_settings.insert(SFD_q_device_token, QVariant(device_token));
        Log() << m_settings.value(SFD_q_device_token).toString();
        Settings::Instance()->setDeviceToken(device_token);
    }
    else if(cmd == Sp::ReadServiceName || cmd == Sp::WriteServiceName)
    {
        Log() << "rcvPacket: " << rcvPacket;
        Log() << "rcvPacket: " << rcvPacket.toHex();
        int keysize = rcvPacket[5] - 8;
        QString service_name = QString(rcvPacket.mid(11,keysize));
        m_settings.insert(SFD_q_service_name, QVariant(service_name));
        Log() << m_settings.value(SFD_q_service_name).toString();
        Settings::Instance()->setServiceName(service_name);
    }
    else if(cmd == Sp::ReadIMEI)
    {
        Log() << "rcvPacket: " << rcvPacket;
        Log() << "rcvPacket: " << rcvPacket.toHex();
        int keysize = rcvPacket[5] - 8;
        QString imei = QString(rcvPacket.mid(11,keysize));
        m_settings.insert(SFD_q_imei, QVariant(imei));
        Log() << m_settings.value(SFD_q_imei).toString();
        Settings::Instance()->setIMEI(imei);
    }
    else if(cmd == Sp::ReadICCID)
    {
        Log() << "rcvPacket: " << rcvPacket;
        Log() << "rcvPacket: " << rcvPacket.toHex();
        int keysize = rcvPacket[5] - 8;
        QString iccid = QString(rcvPacket.mid(11,keysize));
        m_settings.insert(SFD_q_iccid, QVariant(iccid));
        Log() << m_settings.value(SFD_q_iccid).toString();
        Settings::Instance()->setICCID(iccid);
    }
    else if(cmd == Sp::ReadModuleSoftwareVer)
    {
        Log() << "rcvPacket: " << rcvPacket;
        Log() << "rcvPacket: " << rcvPacket.toHex();
        int keysize = rcvPacket[5] - 8;
        QString module_software_ver = QString(rcvPacket.mid(11,keysize));
        m_settings.insert(SFD_q_module_software_ver, QVariant(module_software_ver));
        Log() << m_settings.value(SFD_q_module_software_ver).toString();
        Settings::Instance()->setModuleSoftwareVer(module_software_ver);
    }
    else if(cmd == Sp::ReadIMSI)
    {
        Log() << "rcvPacket: " << rcvPacket;
        Log() << "rcvPacket: " << rcvPacket.toHex();
        int keysize = rcvPacket[5] - 8;
        QString imsi = QString(rcvPacket.mid(11,keysize));
        m_settings.insert(SFD_q_imsi, QVariant(imsi));
        Log() << m_settings.value(SFD_q_imsi).toString();
        Settings::Instance()->setIMSI(imsi);
    }
    else if(cmd == Sp::ReadSktSN || cmd == Sp::WriteSktSN)
    {
        Log() << "rcvPacket: " << rcvPacket;
        Log() << "rcvPacket: " << rcvPacket.toHex();
        int keysize = rcvPacket[5] - 8;
        QString skt_sn = QString(rcvPacket.mid(11,keysize));
        m_settings.insert(SFD_q_skt_sn, QVariant(skt_sn));
        Log() << m_settings.value(SFD_q_skt_sn).toString();
        Settings::Instance()->setSktSN(skt_sn);
    }
    else if(cmd == Sp::ReadRegistrationURL || cmd == Sp::WriteRegistrationURL)
    {
        Log() << "rcvPacket: " << rcvPacket;
        Log() << "rcvPacket: " << rcvPacket.toHex();
        int keysize = rcvPacket[5] - 8;
        QString url = QString(rcvPacket.mid(11,keysize));
        m_settings.insert(SFD_q_registration_url, QVariant(url));
        Log() << m_settings.value(SFD_q_registration_url).toString();
        Settings::Instance()->setRegistrationURL(url);
    }
    else if(cmd == Sp::Unlock)
    {
        if(rcvPacket[10] == (char)0xAA && rcvPacket[11] == (char)0x55) { // unlock success
            m_settings.insert(SFD_q_flag_unlock_success, QVariant(1));
        } else if(rcvPacket[10] == (char)0x55 && rcvPacket[11] == (char)0xAA) { // unlock fail
            m_settings.insert(SFD_q_flag_unlock_success, QVariant(0));
        }
    }
    else if(cmd == Sp::ReadCodingMode)
    {
        Log() << "rcvPacket: " << rcvPacket;
        Log() << "rcvPacket: " << rcvPacket.toHex();
        int codingMode = (int)rcvPacket[10];
        m_settings.insert(SFD_q_code_mode , QVariant(codingMode));
        Log() << m_settings.value(SFD_q_code_mode).toString();
        Settings::Instance()->setCodingMode(codingMode);
    }
    else if(cmd == Sp::ReadOtgMode)
    {
        Log() << "rcvPacket: " << rcvPacket;
        Log() << "rcvPacket: " << rcvPacket.toHex();
        int otgMode = (int)rcvPacket[10];
        m_settings.insert(SFD_q_otg_mode , QVariant(otgMode));
        Log() << m_settings.value(SFD_q_otg_mode).toString();
        Settings::Instance()->setOtgMode(otgMode);
    }
#endif
}

// STM32_DEBUG
// Sp::ReadQcData 요청 후 답변이 없는 경우에 다시 처음상태로 돌아가게 함
static int ReadQcData_timerEventCalled = 0;
void SerialProtocol3::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event);
    Log() << ReadQcData_timerEventCalled;
    if(lastCommand == Sp::ReadQcData && ReadQcData_timerEventCalled < 2)
    {
        ReadQcData_timerEventCalled++;
        if(readBuffer.length() >= 36)
        {
            lastRcvPacket.remove(0, readBuffer.count());
            lastRcvPacket.append(readBuffer);
            parseQcData(readBuffer);
        }
        return;
    }
    else if(lastCommand == Sp::ReadBLE)
    {
//        lastCommand = Sp::ProtocolCommandNone;
//        Settings::Instance()->setBleMeter(0);
//        emit finishReadingQcData();

        return;
    }
    /*
    else
    {
        timerStop();
        currentState = Sp::Idle;
        //emit timeoutError(lastCommand);
        if(m_current_cmd_index < m_commands.length())
        {
            //go ahead
            requestCommand(m_commands[m_current_cmd_index]);
        }
        else
        {
            emit finishDoCommands(true, m_commands[m_current_cmd_index - 1]);
        }
    }
    */

//    if(ReadQcData_timerEventCalled >= 3) {
//        Log() << ReadQcData_timerEventCalled;   // DEBUG
//    }

    ReadQcData_timerEventCalled = 0;
    timerStop();
    currentState = Sp::Idle;
    emit timeoutError(lastCommand);
}

void SerialProtocol3::doCommands(QList<Sp::ProtocolCommand> commands)
{
    m_settings = Settings::Instance()->getSettings();
    m_current_cmd_index = 0;
    m_commands = commands;

    requestCommand(commands[m_current_cmd_index]);
}

float SerialProtocol3::bytesToFloat(uchar b0, uchar b1, uchar b2, uchar b3)
{
    float output;

    *((uchar*)(&output) + 3) = b0;
    *((uchar*)(&output) + 2) = b1;
    *((uchar*)(&output) + 1) = b2;
    *((uchar*)(&output) + 0) = b3;

    return output;
}
