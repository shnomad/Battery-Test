#include "serialprotocol2.h"
#include <QTimer>

#define timerStop() \
    {Log() << "timerStop()"; if(timeoutTimerID) { killTimer(timeoutTimerID); timeoutTimerID = 0; }}
#define timerStart() \
    {Log() << "timerStart()";\
    if(timeoutTimerID) { killTimer(timeoutTimerID); timeoutTimerID = 0; }\
    timeoutTimerID = startTimer(kProtocol2TimeoutDuration);}

SerialProtocol2::SerialProtocol2(SerialComm *serialComm, QObject *parent) : SerialProtocolAbstract(serialComm, parent)
{
    Log();
    downloadInfo.setProcotol(Sp::CommProtocol2);
    connect(&downloadInfo, SIGNAL(downloadProgress(float)), this, SIGNAL(downloadProgress(float)));
    connectSignals();
}

SerialProtocol2::~SerialProtocol2()
{
    Log();
    timerStop();
}

void SerialProtocol2::setCommObject(SerialComm *serialComm) {
    if(comm) {
        disconnectSignals();
    }
    comm = serialComm;
    connectSignals();
}
void SerialProtocol2::connectSignals() {
    if(comm) {
        connect(comm, SIGNAL(readyRead()), this, SLOT(readyRead()));
        connect(comm, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(error(QSerialPort::SerialPortError)));
    }
}
void SerialProtocol2::disconnectSignals() {
    if(comm) {
        disconnect(comm, SIGNAL(readyRead()), this, SLOT(readyRead()));
        disconnect(comm, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(error(QSerialPort::SerialPortError)));
    }
}

Sp::ProtocolState SerialProtocol2::startDownload() {
    if(currentState == Sp::Idle) {
        downloadInfo.setNumberOfGluecose(0);
        requestCommand(Sp::ReadSerialNumber);
    } else {
        return currentState;
    }
    return Sp::Idle;
}

Sp::ProtocolState SerialProtocol2::syncTime()
{
    requestCommand(Sp::WriteTimeInformation);

    return Sp::Idle;
}

Sp::ProtocolState SerialProtocol2::readTime()
{
    m_current_time = QDateTime::currentDateTime();
    requestCommand(Sp::ReadTimeInformation);

    return Sp::Idle;
}

void SerialProtocol2::cancelDownload() {

}
const QByteArray &SerialProtocol2::lastReceivePacket() {
    return lastRcvPacket;
}

qint64 SerialProtocol2::requestCommand(const Sp::ProtocolCommand &command, QByteArray *arg1, QByteArray *arg2, QByteArray *arg3) {
    Log();
    timerStop();
    if(currentState == Sp::RequestWaiting) {
        Log() << "Request Waiting......" << lastCommand;
        return 0;
    }
    currentState = Sp::RequestWaiting;
    QByteArray requestData = beginCreatePacket();
    switch(command) {
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
        default:
        {
            Q_ASSERT(0);
            return 0;
        }
    }
    Log() << "request" << requestData.toHex();
    lastCommand = command;
    timerStart();
    return comm->writeData(requestData);
}

QByteArray SerialProtocol2::beginCreatePacket() {
    QByteArray array;
    array.append(0x02);
    array.append("iSPc");
    return array;
}

void SerialProtocol2::endCreatePacket(QByteArray *array) {
    QByteArray crc = argUShort(calcCrc(*array));
    int count = array->count();
    array->insert(count-1, crc);
}
QByteArray SerialProtocol2::makeGluecoseResultDataTx(QByteArray indexArray) {
    QByteArray array;
    array.append("GLUC");
    array.append(indexArray);
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}
QByteArray SerialProtocol2::makeGluecoseResultDataTxExpanded(QByteArray indexArray, QByteArray countArray) {
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
QByteArray SerialProtocol2::makeCurrentIndexOfGluecose() {
    QByteArray array;
    array.append("NCOT");
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}

QByteArray SerialProtocol2::makeReadSerialNumber() {
    QByteArray array;
    array.append("RSNB");
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}
QByteArray SerialProtocol2::makeWriteSerialNumber(QByteArray sn) {
    QByteArray array;
    array.append("WSNB");
    array.append(sn);
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}
QByteArray SerialProtocol2::makeReadTimeInformation() {
    QByteArray array;
    array.append("RTIM");
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}
QByteArray SerialProtocol2::makeWriteTimeInformation() {
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
QByteArray SerialProtocol2::makeSaveData() {
    QByteArray array;
    array.append("SVDT");
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}
QByteArray SerialProtocol2::makeDeleteData() {
    QByteArray array;
    array.append("DELD");
    array.append(0x03);
    // 패킷 사이즈
    short cnt = array.count()+2;    // crc 포함
    array.insert(0, (const char)cnt);
    return array;
}


bool SerialProtocol2::isCompletePacketReceved() {
    bufferMutex.lock();
    if(readBuffer.count() > 5) {
        uchar size = readBuffer.at(headerSize());
//        Log() << "complete size = " << size << " headersize =" << headerSize() << " buffer = " << readBuffer.count();
//        Log() << "complete buffer = " << QString(readBuffer);
//        Log() << "complete bufferhex = " << readBuffer.toHex();
        if(readBuffer.count() >= size+headerSize()+1) {   // stx 부터 크기까지의 고정 바이트수 == headerSize()+1
            bufferMutex.unlock();
            return true;
        }
    }
    bufferMutex.unlock();
    return false;
}
const QByteArray SerialProtocol2::getReceivePacket() {

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
Sp::ProtocolCommand SerialProtocol2::getCommand(QByteArray rcvPacket) {
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
    return Sp::ProtocolCommandNone;
}
ushort SerialProtocol2::getIndexOfGluecose(QByteArray rcvPacket) {
    ushort index = *(ushort *)rcvPacket.mid(10, 2).data();
    index = byteswap(index);
    return index;
}
ushort SerialProtocol2::getGluecoseCount(QByteArray rcvPacket) {
    ushort cnt = rcvPacket.count()-13;
    if(cnt%9) {
        Q_ASSERT(0);
    }
    return ushort(cnt/9);
}

void SerialProtocol2::produceError(Sp::ProtocolCommand receivedCommand, QByteArray rcvPacket) {
    Log() << "Error Command received" << rcvPacket.mid(6, 4);
    timerStop();
    emit errorOccurred(receivedCommand, Sp::ProtocolCommandNone);

}
void SerialProtocol2::processPacket(QByteArray rcvPacket) {
    timerStop();
    currentState = Sp::GluecoseDownloading;

    // crc check
    QByteArray checkPacket = QByteArray(rcvPacket.left(rcvPacket.count()-3));
    checkPacket.append(rcvPacket.at(rcvPacket.count()-1));
    QByteArray crcArray = rcvPacket.mid(rcvPacket.count()-3, 2);
    ushort rcvCrc = *(ushort *)(crcArray.data());
    rcvCrc = byteswap(rcvCrc);
    if(isEqualCrc(checkPacket, rcvCrc) || lastCommand == Sp::GluecoseResultDataTx) {
        lastCommand = Sp::ProtocolCommandNone;
        Sp::ProtocolCommand cmd = getCommand(rcvPacket);
        if(isErrorResponse(cmd)) {
            produceError(cmd, rcvPacket);
            currentState = Sp::Idle;
            return;
        }

        parseReceivedData(rcvPacket);

        if(cmd == Sp::ReadSerialNumber)
        {
            currentState = Sp::GluecoseDownloading;

            downloadInfo.setNumberOfGluecose(0);

            emit downloadProgress(downloadInfo.progress());

            requestCommand(Sp::CurrentIndexOfGluecose);
        }        
        else if(cmd == Sp::CurrentIndexOfGluecose)
        {
            // 다운로드할 글루코스 데이터 개수 설정: 다운로드가 시작됨
            downloadInfo.setNumberOfGluecose(getIndexOfGluecose(rcvPacket));

            emit downloadProgress(downloadInfo.progress());

            QByteArray arg1 = argUShort(ushort(downloadInfo.index()));
            QByteArray arg2 = argByte(char(downloadInfo.downloadableCount()));
            requestCommand(Sp::GluecoseResultDataTx, &arg1, &arg2);
        } else if(cmd == Sp::GluecoseResultDataTx) {
            // TODO: 다운로드 된 데이터 처리 한다.
            emit packetReceived();

            downloadInfo.setDownloadedCount(1);

            if(downloadInfo.downloadableCount()) {
                currentState = Sp::GluecoseDownloading;
                QByteArray arg1 = argUShort(ushort(downloadInfo.index()));
                QByteArray arg2 = argByte(char(downloadInfo.downloadableCount()));
                requestCommand(Sp::GluecoseResultDataTx, &arg1, &arg2);
            } else {
                QJsonObject sn;
                sn["sn"] = m_serialnumber;
                m_dataArray.push_front(sn);
                Log() << "DataDownload Complete total = " << downloadInfo.getNunberOfGluecose();
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
        else {
            // 다운로드 과정에서 처리하지 않는 커멘드의 수신
             emit errorUnresolvedCommand(cmd);
        }
    } else {
        Log() << "CRC error";
        currentState = Sp::Idle;
        emit errorCrc();
    }
}

void SerialProtocol2::readyRead() {
    timerStop();

    bufferMutex.lock();
    QByteArray readData = comm->readAll();
    if(lastCommand != Sp::ProtocolCommandNone) {
        if(readData.count()) {
            readBuffer.append(readData);
        }
        int flushIndex = 0;
        while(readBuffer.at(flushIndex) != kSTX) {
            flushIndex ++;
            if(flushIndex > readBuffer.count()) {
                flushIndex  = readBuffer.count();
                break;
            }
        }
        readBuffer.remove(0, flushIndex);
        bufferMutex.unlock();
        if(isCompletePacketReceved()) {
            QByteArray rcvPacket = getReceivePacket();
            lastRcvPacket.remove(0, lastRcvPacket.count());
            lastRcvPacket.append(rcvPacket);
            processPacket(rcvPacket);
        } else {
            timerStart();
        }
    } else {
        bufferMutex.unlock();
        return;
    }
}
void SerialProtocol2::error(QSerialPort::SerialPortError error) {
    timerStop();
    if(error != QSerialPort::NoError) {
        emit needReopenSerialComm();
    }
}

void SerialProtocol2::parseReceivedData(QByteArray rcvPacket)
{
    Sp::ProtocolCommand cmd = getCommand(rcvPacket);
    if(cmd == Sp::ReadSerialNumber)
    {
        m_serialnumber = QString(rcvPacket.mid(11,12));

    } else if(cmd == Sp::WriteTimeInformation) {
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

        QDateTime meterTime = QDateTime(date, dtime);
        emit completeReadTime(&meterTime);
    }
    else if(cmd == Sp::GluecoseResultDataTx)
    {
        QJsonObject data;
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

        dtime.setHMS(hour, min, sec,0);

        QDateTime datatime;
        datatime.setDate(date);
        datatime.setTime(dtime);

        int value = (quint8(rcvPacket[dataindex + 7]) << 8) + quint8(rcvPacket[dataindex + 8]);
        data["index"] = "";
        data["glucose_data"] = QString().sprintf("%d", value);
        data["dDate"] = QString::number(datatime.toMSecsSinceEpoch());
        data["time"] = "";//Settings::Instance()->GetDatetimestringFromMSec(QString::number(datatime.toMSecsSinceEpoch()));
        data["manual"] = m_serialnumber;
        data["flag_cs"] = "0";
        data["flag_meal"] = "0";
        data["flag_hilo"] = "0";
        data["flag_fasting"] = "0";
        data["flag_nomark"] = "0";
        data["flag_ketone"] = "0";
        data["glucose_unit"] = "mg/dL";

        int gflag = quint8(rcvPacket[dataindex + 6]);

        qDebug() << "data = " << value << " gflag = "<< gflag;

        if((gflag & 1) == 1)
        {
            data["flag_cs"] = "cs";
        }

        if((gflag & 2) == 2)
        {
            data["flag_meal"] = "af";
        }

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

        if((gflag & 64) == 64)
        {
            data["flag_ketone"] = "kt";
            data["glucose_unit"] = "mmol/L";
        }

        m_dataArray.push_front(data);
    }
    else if(cmd == Sp::GluecoseResultDataTxExpanded)
    {
        int count = getGluecoseCount(rcvPacket);

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
            data["index"] = "";
            data["glucose_data"] = QString().sprintf("%d", value);
            data["dDate"] = QString::number(datatime.toMSecsSinceEpoch());
            data["time"] = "";//Settings::Instance()->GetDatetimestringFromMSec(QString::number(datatime.toMSecsSinceEpoch()));
            data["manual"] = m_serialnumber;
            data["flag_cs"] = "0";
            data["flag_meal"] = "0";
            data["flag_hilo"] = "0";
            data["flag_fasting"] = "0";
            data["flag_nomark"] = "0";
            data["flag_ketone"] = "0";
            data["glucose_unit"] = "mg/dL";


            int gflag = quint8(rcvPacket[dataindex + 6]);
            //data["gflag"] = QString().sprintf("%d", gflag);

            qDebug() << "data = " << value << " gflag = "<< gflag;

            if((gflag & 1) == 1)
            {
                data["flag_cs"] = "cs";
            }

            if((gflag & 2) == 2)
            {
                data["flag_meal"] = "af";
            }

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

            if((gflag & 64) == 64)
            {
                data["flag_ketone"] = "kt";
                data["glucose_unit"] = "mmol/L";
            }

            //brazil index C0 exception
            if(m_serialnumber != "" && value == 10 &&
              m_serialnumber[0]== 'C' && m_serialnumber[1]== '0')
            {
                data["flag_hilo"] = "lo";
            }


            m_dataArray.push_front(data);
        }
    }
}
void SerialProtocol2::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event);
    Log();
    timerStop();
    currentState = Sp::Idle;
    emit timeoutError(lastCommand);
}
