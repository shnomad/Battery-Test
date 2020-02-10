#include "serialprotocol1.h"

QString model_code[19] = {"CSP", "ISP","GSP", "CNP", "NCP", "HDP", "ACP", "NTP", "ECP", "AQP", "ITP", "CNM",
                         "APP", "CLP", "DAP", "MDP", "CMP", "CVP", "---"};
char program_ver[26] = {'A','B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N','O',
                        'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

#define GetByte(buffer, index)\
    (quint8((buffer[1+(index*3)] & 0x0F) << 4) + quint8(buffer[2+(index*3)] & 0x0F))

#define kProtocol1TimeoutDuration   (3000)        // 3초
#define timerStop() \
    {if(timeoutTimerID) { killTimer(timeoutTimerID); timeoutTimerID = 0; }}
#define timerStart() \
    {if(timeoutTimerID) { killTimer(timeoutTimerID); timeoutTimerID = 0; }\
    timeoutTimerID = startTimer(kProtocol1TimeoutDuration);}


SerialProtocol1::SerialProtocol1(SerialComm *serialComm, QObject *parent) : SerialProtocolAbstract(serialComm, parent),
    timeoutTimerID(0), isSerialNumberType1(false)
{
    connect(&downloadInfo, SIGNAL(downloadProgress(float)), this, SIGNAL(downloadProgress(float)));
    connectSignals();

    m_sn_flag = 0;
    m_nMemory = 0;
    m_nMaxMeterCount = MAX_DATA_COUNT;
    m_nSavePoint = 0;
    m_nReqMaxCnt = 1;
}

SerialProtocol1::~SerialProtocol1()
{
    Log() << "SerialProtocol1::~SerialProtocol1()";
    timerStop();
}

void SerialProtocol1::setCommObject(SerialComm *serialComm) {
    if(comm) {
        disconnectSignals();
    }
    comm = serialComm;
    connectSignals();
}
void SerialProtocol1::connectSignals() {
    if(comm) {
        connect(comm, SIGNAL(readyRead()), this, SLOT(readyRead()));
        connect(comm, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(error(QSerialPort::SerialPortError)));
    }
}
void SerialProtocol1::disconnectSignals() {
    if(comm) {
        disconnect(comm, SIGNAL(readyRead()), this, SLOT(readyRead()));
        disconnect(comm, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(error(QSerialPort::SerialPortError)));
    }
}

Sp::ProtocolState SerialProtocol1::startDownload() {
    if(currentState == Sp::Idle) {
        downloadInfo.setNumberOfGluecose(0);
        requestCommand(Sp::ReadSerialNumber);
    } else {
        return currentState;
    }
    return Sp::Idle;
}
Sp::ProtocolState SerialProtocol1::syncTime()
{
    timerStart();
    requestCommand(Sp::WriteTimeInformation);
    return Sp::Idle;
}

Sp::ProtocolState SerialProtocol1::readTime()
{
    return Sp::AnError;
}
void SerialProtocol1::cancelDownload() {

}
const QByteArray &SerialProtocol1::lastReceivePacket() {
    return lastRcvPacket;
}

qint64 SerialProtocol1::requestCommand(const Sp::ProtocolCommand &command, QByteArray *arg1, QByteArray *arg2, QByteArray *arg3) {
    if(currentState == Sp::RequestWaiting) {
        Log() << "Request Waiting......" << lastCommand;
        return 0;
    }
    currentState = Sp::RequestWaiting;
    QByteArray requestData = beginCreatePacket();
    switch(command)
    {
        case Sp::ReadSerialNumber:
        {
            Q_UNUSED(arg1); Q_UNUSED(arg2); Q_UNUSED(arg3);
            requestData.append(makeReadSerialNumber());           
            break;
        }
        case Sp::WriteTimeInformation:
        {
            Q_UNUSED(arg1); Q_UNUSED(arg2); Q_UNUSED(arg3);
            requestData.clear();
            requestData.append(makeWriteTimeInformation());
            break;
        }
        case Sp::CurrentIndexOfGluecose:
        {
            Q_UNUSED(arg1); Q_UNUSED(arg2); Q_UNUSED(arg3);
            requestData.append(makeCurrentIndexOfGluecose());
            break;
        }
        case Sp::ReadSavePoint:
        {
            Q_UNUSED(arg1); Q_UNUSED(arg2); Q_UNUSED(arg3);
            requestData.append(makeReadSavePoint());
            break;
        }
        case Sp::GluecoseResultDataTx:
        {
            Q_UNUSED(arg1); Q_UNUSED(arg2); Q_UNUSED(arg3);
            //Log() << "request GluecoseResultDataTx index =" << *(ushort *)arg1->data();
            requestData.append(makeGluecoseResultDataTx());
            break;
        }
        default:
        {
            Q_ASSERT(0);
            return 0;
        }
    }
    //Log() << "request" << requestData.toHex();
    lastCommand = command;
    timerStart();


    return comm->writeData(requestData);
}

QByteArray SerialProtocol1::beginCreatePacket() {
    QByteArray array;   
    array.append(0x8B);//data read cmd @rev.1
    return array;
}

void SerialProtocol1::endCreatePacket(QByteArray *array) {
    QByteArray crc = argUShort(calcCrc(*array));
    int count = array->count();
    array->insert(count-1, crc);
}
QByteArray SerialProtocol1::makeCurrentIndexOfGluecose()
{
    QByteArray array;

    array.append(0x11);
    array.append(0x20);
    array.append(0x18);
    array.append(0x26);
    array.append(0x10);
    array.append(0x22);

    return array;

}

QByteArray SerialProtocol1::makeReadSavePoint()
{
    QByteArray array;

    array.append(0x11);
    array.append(0x20);
    array.append(0x18);
    array.append(0x28);
    array.append(0x10);
    array.append(0x22);

    return array;

}



QByteArray SerialProtocol1::makeGluecoseResultDataTx()
{
    QByteArray array;

    if(m_sn_flag == 1)
    {
        if((m_serialnumber.contains("CSP") == true) || (m_serialnumber.contains("ISP") == true ))
        {
            m_data_address = 0xc200;
        }
        else if(m_serialnumber.contains("GSP") == true )
        {
            m_data_address = 0xd200;
        }
        else
        {
            m_data_address = 0xe200;
        }
    }
    else
    {
        if(m_nMemory == 4)
        {
            m_data_address = 0xcc00;
        }
        else
        {
            if(m_serialnumber.contains("B3A") == true )
            {
                m_data_address = 0xc200;
            }
            else
            {
                 m_data_address = 0xe200;
            }
        }
    }
    m_data_address = m_data_address + (8 * downloadInfo.index());
    int savepoint = m_nMaxMeterCount - m_nSavePoint;
    if(m_serialnumber.contains("CVP") == true)
    {
        m_nReqMaxCnt = 1;
    }
    else if((downloadInfo.getNunberOfGluecose() == m_nMaxMeterCount) && (savepoint > 0))
    {
        // save point 처리 하는 코드가 들어가야 함(Full로 한번이라도 차있을 경우만 처리함)
        if(savepoint > downloadInfo.index())
            m_nReqMaxCnt = savepoint - downloadInfo.index() >= 10? 10 : savepoint - downloadInfo.index();
        else
            m_nReqMaxCnt = downloadInfo.getNunberOfGluecose() - downloadInfo.index() >= 10 ? 10: downloadInfo.getNunberOfGluecose() - downloadInfo.index();
    }
    else
        m_nReqMaxCnt = downloadInfo.getNunberOfGluecose() - downloadInfo.index() >= 10 ? 10 : downloadInfo.getNunberOfGluecose() - downloadInfo.index();

    qDebug() << "m_nReqMaxCnt = " << m_nReqMaxCnt;

    array.append((1 << 4) | ((m_data_address & 0xF000) >> 12));
    array.append((2 << 4) | ((m_data_address & 0x0F00) >> 8));
    array.append((1 << 4) | ((m_data_address & 0x00F0) >> 4));
    array.append((2 << 4) | (m_data_address & 0x000F));
    array.append(((m_nReqMaxCnt* 8) >> 4)	+ 0x10);//array.append(0x10);
    array.append(((m_nReqMaxCnt* 8) & 0x0F) + 0x20);//array.append(0x28);


    return array;
}


QByteArray SerialProtocol1::makeReadSerialNumber() {
    QByteArray array;

    array.append(0x11);
    array.append(0x20);
    array.append(0x13);
    array.append(0x24);
    array.append(0x10);
    array.append(0x2a);

    return array;
}


QByteArray SerialProtocol1::makeWriteTimeInformation() {
    QByteArray array;

    char year, month, day, hour, min, sec, checksum;

    //QDateTime current = QDateTime::currentDateTime();

    QDate date = QDate::currentDate();
    year = (date.year() - 2000) & 0xff;
    month = date.month() & 0xff;
    day = date.day() & 0xff;//

    QTime currenttime = QTime::currentTime();
    hour = currenttime.hour() & 0xff;
    min = currenttime.minute() & 0xff;//
    sec = currenttime.second() & 0xff;

    checksum = year^month^day^hour^min^sec;

    array.append(0xb0);

    array.append((year >> 4) | 0x10);
    array.append((year & 0x0f) | 0x20);
    array.append((month >> 4) | 0x10);
    array.append((month & 0x0f) | 0x20);
    array.append((day >> 4) | 0x10);
    array.append((day & 0x0f) | 0x20);

    array.append((hour >> 4) | 0x10);
    array.append((hour & 0x0f) | 0x20);
    array.append((min >> 4) | 0x10);
    array.append((min & 0x0f) | 0x20);
    array.append((sec >> 4) | 0x10);
    array.append((sec & 0x0f) | 0x20);

    array.append((checksum >> 4) | 0x10);
    array.append((checksum & 0x0f) | 0x20);

    return array;
}



bool SerialProtocol1::isCompletePacketReceved() {
    bufferMutex.lock();
    if(lastCommand == Sp::ReadSerialNumber) {
        if(packetSize(readBuffer.count())) {
            unsigned char flag = GetByte(readBuffer, 7);
            if((flag == 0x00 && readBuffer.count() >= 24) ) {
                isSerialNumberType1 = true;
                bufferMutex.unlock();
                return true;
            } else if((flag == 0x01 && readBuffer.count() >= 30)) {
                isSerialNumberType1 = false;
                bufferMutex.unlock();
                return true;
            }
        }
        bufferMutex.unlock();
        return false;
    }
    if(packetSize(readBuffer.count()))
    {
        bufferMutex.unlock();
        return true;
    }
    bufferMutex.unlock();
    return false;
}
const QByteArray SerialProtocol1::getReceivePacket() {

    bufferMutex.lock();
    if(readBuffer.count() > 0)
    {   // stx 부터 크기까지의 고정 바이트수 == headerSize()+1
        if(lastCommand == Sp::ReadSerialNumber) {
            if(isSerialNumberType1) {
                QByteArray rcvPacket;
                rcvPacket.append(readBuffer.left(24));
                readBuffer.remove(0, 24);
                bufferMutex.unlock();
                return rcvPacket;
            } else {
                QByteArray rcvPacket;
                rcvPacket.append(readBuffer.left(30));
                readBuffer.remove(0, 30);
                bufferMutex.unlock();
                return rcvPacket;
            }
        } else {
            QByteArray rcvPacket;
            rcvPacket.append(readBuffer);
            readBuffer.remove(0, readBuffer.count());
            bufferMutex.unlock();
            return rcvPacket;
        }
    }
    bufferMutex.unlock();
    return QByteArray();
}

ushort SerialProtocol1::getIndexOfGluecose(QByteArray rcvPacket) {
    ushort index = *(ushort *)rcvPacket.mid(10, 2).data();
    index = byteswap(index);
    return index;
}
ushort SerialProtocol1::getGluecoseCount(QByteArray rcvPacket) {
    ushort cnt = rcvPacket.count()-13;
    if(cnt%9) {
        Q_ASSERT(0);
    }
    return ushort(cnt/9);
}

void SerialProtocol1::produceError(Sp::ProtocolCommand receivedCommand, QByteArray rcvPacket) {
    Log() << "Error Command received" << rcvPacket.mid(6, 4);
    timerStop();
    emit errorOccurred(receivedCommand, Sp::ProtocolCommandNone);

}
void SerialProtocol1::serialNumberType1Timeout() {
    if(lastCommand == Sp::ReadSerialNumber) {
        bufferMutex.lock();

        Log() << "readBuffer = " << readBuffer.toHex().at(0);
        Log() << "readBuffer = " << readBuffer.toHex().at(1);
        if(readBuffer.count()) {
            if(readBuffer.count() >= 6) {
                if(readBuffer.toHex().at(0) == '8' && readBuffer.toHex().at(1) == 'b' &&
                   readBuffer.toHex().at(2) == '1' && readBuffer.toHex().at(3) == 'f' &&
                   readBuffer.toHex().at(4) == '2' && readBuffer.toHex().at(5) == 'f' )
                        //readBuffer.at(3) == 0x8b && readBuffer.at(4) == 0x1f && readBuffer.at(5) == 0x2f)
                {
                    readBuffer.remove(0, 6);
                } else {
                    // other exception
                    // normaly going
                }
            } else {
                waitingCount --;
                if(waitingCount) {
                    serialNumberType1ExceptionTimer.start(200);
                    bufferMutex.unlock();
                    return;
                }
            }

            Log() << "readBuffer = " << readBuffer.toHex();

            currentState = Sp::GluecoseDownloading;
            // 다운로드할 글루코스 데이터 개수 설정: 다운로드가 시작됨

            emit downloadProgress(downloadInfo.progress());
            if(m_serialnumber != "" && m_serialnumber.contains("CVP") == true)
            {
                requestCommand(Sp::GluecoseResultDataTx);
            }
            else
            {
                downloadInfo.setNumberOfGluecose(0);
                requestCommand(Sp::CurrentIndexOfGluecose);
            }
        }
        bufferMutex.unlock();
    }
}
void SerialProtocol1::processPacket(QByteArray rcvPacket) {
    //Log() << "processPacket" << rcvPacket.toHex();
    currentState = Sp::GluecoseDownloading;
    parseReceivedData(rcvPacket);

    // 예외 처리 xxxx
    if(lastCommand == Sp::ReadSerialNumber)
    {
        if(isSerialNumberType1) {
            timerStop();
            waitingCount = 3;
            connect(&serialNumberType1ExceptionTimer,  SIGNAL(timeout()), this, SLOT(serialNumberType1Timeout()));
            //serialNumberType1ExceptionTimer.setSingleShot(true);
            serialNumberType1ExceptionTimer.start(200);
            return;
        }
    }

    Sp::ProtocolCommand cmd = lastCommand;
    lastCommand = Sp::ProtocolCommandNone;

    if(cmd == Sp::ReadSerialNumber)
    {
        currentState = Sp::GluecoseDownloading;
        // 다운로드할 글루코스 데이터 개수 설정: 다운로드가 시작됨

        emit downloadProgress(downloadInfo.progress());
        if(m_serialnumber != "" && m_serialnumber.contains("CVP") == true)
        {
            requestCommand(Sp::GluecoseResultDataTx);
        }
        else
        {
            downloadInfo.setNumberOfGluecose(0);
            requestCommand(Sp::CurrentIndexOfGluecose);
        }
    }
    else if(cmd == Sp::WriteTimeInformation)
    {
        emit completeTimeSync(Q_NULLPTR, Q_NULLPTR);
        timerStop();
    }
    else if(cmd == Sp::CurrentIndexOfGluecose)
    {
         requestCommand(Sp::ReadSavePoint);
    }
    else if(cmd == Sp::ReadSavePoint)
    {
         if(downloadInfo.downloadableCount() == 0)
         {
             QJsonObject sn;
             sn["sn"] = m_serialnumber;
             m_dataArray.push_front(sn);
             emit downloadComplete(&m_dataArray);
             timerStop();
         }
         else
         {
            requestCommand(Sp::GluecoseResultDataTx);
         }
    }

    else if(cmd == Sp::GluecoseResultDataTx)
    {
        // TODO: 다운로드 된 데이터 처리 한다.
        emit packetReceived();

        if(downloadInfo.downloadableCount())
        {
            currentState = Sp::GluecoseDownloading;
            requestCommand(Sp::GluecoseResultDataTx);
        }
        else
        {
            //Log() << "다운로드 완료" << downloadInfo.getNunberOfGluecose();
            QJsonObject sn;
            sn["sn"] = m_serialnumber;
            m_dataArray.push_front(sn);
            emit downloadComplete(&m_dataArray);
            timerStop();
        }

    } else {
        // 다운로드 과정에서 처리하지 않는 커멘드의 수신
        emit errorUnresolvedCommand(lastCommand);
        timerStop();
    }

}

void SerialProtocol1::readyRead() {
    timerStop();

    bufferMutex.lock();
    QByteArray readData = comm->readAll();
    if(lastCommand != Sp::ProtocolCommandNone) {
        if(readData.count()) {
            readBuffer.append(readData);
        }
        bufferMutex.unlock();
        Log() << "readBuffer " << readBuffer.toHex();

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
void SerialProtocol1::error(QSerialPort::SerialPortError error) {
    timerStop();

    if(error != QSerialPort::NoError)
    {
            emit needReopenSerialComm();
    }
}

void SerialProtocol1::parseReceivedData(QByteArray rcvPacket)
{
    if(lastCommand == Sp::ReadSerialNumber)
    {
        if((((rcvPacket[22] & 0x0F) << 4) + (rcvPacket[23] & 0x0F)) == 0x00)
        {
            m_sn_flag = 1;
            int temp_number[4];
            for(int loop = 0; loop < 4; loop++)
                temp_number[loop] =(((rcvPacket[1 + loop*6] & 0x0F)*16 + (rcvPacket[2 + loop*6] & 0x0F)) + ((rcvPacket[4 + loop*6] & 0x0F)*16 + (rcvPacket[5 + loop*6] & 0x0F))*256);

            if(temp_number[2] > 18)
            {
                m_serialnumber = QString().sprintf("%c%c%c%02d%c%05d",
                                                   ((rcvPacket[13] & 0x0f) << 4) + (rcvPacket[14] & 0x0f),
                        ((rcvPacket[16] & 0x0f) << 4) + (rcvPacket[17] & 0x0f),
                        ((rcvPacket[10] & 0x0f) << 4) + (rcvPacket[11] & 0x0f),
                        ((rcvPacket[7] & 0x0f) << 4) + (rcvPacket[8] & 0x0f),
                        program_ver[temp_number[3]%10], temp_number[0]);

            }
            else
            {
                m_serialnumber = model_code[temp_number[2]] + QString().sprintf("%02d%c%05d" , temp_number[1], program_ver[temp_number[3]%26], temp_number[0]);
                if(temp_number[2] == 17)//CVP
                {
                    m_serialnumber = model_code[temp_number[2]] + QString("%1").arg(temp_number[1], 2, 16, QLatin1Char( '0' )).toUpper() + QString().sprintf("%c%05d" , program_ver[temp_number[3]%26], temp_number[0]);
                }
            }
            m_nMaxMeterCount = 250;
            if(temp_number[2] == 12 || temp_number[2] == 13 || temp_number[2] == 16 || temp_number[2] == 17)
                m_nMaxMeterCount = 500;
            if(m_serialnumber != "" && m_serialnumber.contains("CVP") == true)
            {
                //no data about store glucose data
                downloadInfo.setNumberOfGluecose(m_nMaxMeterCount);
            }
        }
        else
        {
            // new s/n
            m_sn_flag = 2;
            m_nMemory = ((rcvPacket[10] & 0x0f) << 4) + (rcvPacket[11] & 0x0f);
            m_nMaxMeterCount = m_nMemory * 250; //memory = max count

            m_serialnumber = QString().sprintf("%c%c%c%03d%c%05d",
                                               ((rcvPacket[1] & 0x0f) << 4) + (rcvPacket[2] & 0x0f), // model code
                    ((rcvPacket[4] & 0x0f) << 4) + (rcvPacket[5] & 0x0f), // model code
                    ((rcvPacket[7] & 0x0f) << 4) + (rcvPacket[8] & 0x0f), // factory code
                    ((rcvPacket[13] & 0x0f) << 4) + (rcvPacket[14] & 0x0f) + ((rcvPacket[16] & 0x0f) << 12) + ((rcvPacket[17] & 0x0f) << 8), //Production date
                    ((rcvPacket[19] & 0x0f) << 4) + (rcvPacket[20] & 0x0f), //production year
                    ((rcvPacket[25] & 0x0f) << 4) + (rcvPacket[26] & 0x0f) + ((rcvPacket[28] & 0x0f) << 12) + ((rcvPacket[29] & 0x0f) << 8) //product count
                    );

        }
    }
    else if(lastCommand == Sp::GluecoseResultDataTx)
    {
        if(rcvPacket[1] == char(0x1f) && rcvPacket[2] == char(47))
        {
            //m_data_list[m_data_count].year = m_data_list[m_data_count].month = m_data_list[m_data_count].day = m_data_list[m_data_count].hour = m_data_list[m_data_count].minute = m_data_list[m_data_count].second = '-';
            //m_data_list[m_data_count].value = m_data_list[m_data_count].ext = -1;
            //just counting for CVP
            downloadInfo.setDownloadedCount(1);
        }
        else
        {
            int year, month, day, hour, min, sec, value,nAddress ;
            for(int i = 0; i < m_nReqMaxCnt; i++)
            {
                nAddress = REV1_GLUCOSE_SIZE * i;
                year = ((rcvPacket[1+nAddress] & 0x0F) << 4) + (rcvPacket[2+nAddress] & 0x0F);
                month = ((rcvPacket[4+nAddress] & 0x0F) << 4) + (rcvPacket[5+nAddress] & 0x0F);
                day = ((rcvPacket[7+nAddress] & 0x0F) << 4) + (rcvPacket[8+nAddress] & 0x0F);
                hour = ((rcvPacket[10+nAddress] & 0x0F) << 4) + (rcvPacket[11+nAddress] & 0x0F);
                min = ((rcvPacket[13+nAddress] & 0x0F) << 4) + (rcvPacket[14+nAddress] & 0x0F);
                sec = ((rcvPacket[16+nAddress] & 0x0F) << 4) + (rcvPacket[17+nAddress] & 0x0F);
                value = ((rcvPacket[19+nAddress] & 0x0F) << 4) + (rcvPacket[20+nAddress] & 0x0F)
                        + ((rcvPacket[22+nAddress] & 0x0F) << 12) + ((rcvPacket[23+nAddress] & 0x0F) << 8);
                QDate date;// = QDate::currentDate();
                date.setDate(2000 + year, month, day);
                QTime dtime;// = QTime::currentTime();
                dtime.setHMS(hour, min, sec,0);
                QDateTime datatime;
                datatime.setDate(date);
                datatime.setTime(dtime);
                Log() << "total= "<< m_dataArray.count() << "index = "<< downloadInfo.index() << " glucose = " << value;
                if(value != 65535)
                {
                    m_dataArray.push_front(makeGlucoseObject(value, datatime));
                }
                downloadInfo.setDownloadedCount(1);
            }

        }

    }
    else if(lastCommand == Sp::CurrentIndexOfGluecose)
    {
        int itemcount = (qint8(rcvPacket[1] & 0x0f) << 4) + qint8(rcvPacket[2] & 0x0f)
                + (qint8(rcvPacket[4] & 0x0f) << 12) + (qint8(rcvPacket[5] & 0x0f) << 8);
        Log() << "itemcount = " << itemcount;
        if(itemcount >= m_nMaxMeterCount)
            itemcount = m_nMaxMeterCount;
        downloadInfo.setNumberOfGluecose(itemcount);


    }
    else if(lastCommand == Sp::ReadSavePoint)
    {
        m_nSavePoint = (qint8(rcvPacket[1] & 0x0f) << 4) + qint8(rcvPacket[2] & 0x0f) //1088
        + (qint8(rcvPacket[4] & 0x0f) << 12) + (qint8(rcvPacket[5] & 0x0f) << 8);//1089
    }
    else if(lastCommand == Sp::WriteTimeInformation)
    {
        //Log() << rcvPacket << ": " << rcvPacket.length();
    }
}

void SerialProtocol1::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event);
    Log();
#ifdef SERIALCOM_SMARTLOG
    if(lastCommand == Sp::ReadSerialNumber)
    {
        if(readBuffer.count() == 24)
        {
            unsigned char flag = GetByte(readBuffer, 7);
            if(flag == 0x00)
            {
                lastRcvPacket.remove(0, lastRcvPacket.count());
                lastRcvPacket.append(readBuffer);
                processPacket(readBuffer);
                bufferMutex.lock();
                readBuffer.remove(0, 24);
                bufferMutex.unlock();
                return;
            }
        }
    }
#endif
    timerStop();
    currentState = Sp::Idle;
    emit timeoutError(lastCommand);
}
