#ifndef SERIALPROTOCOL1_H
#define SERIALPROTOCOL1_H

#include "serialprotocolabstract.h"
#include "serialcomm.h"
#include <QTimer>
#include <QMutex>

#define MAX_DATA_COUNT					1000

#define GLUCOSE_MEAL_CHECK              30000
#define GLUCOSE_CHECK_DATA              20000
#define GLUCOSE_AFTER_MEAL              10000
#define REV1_GLUCOSE_SIZE               24

class SerialComm;
class QTimer;

class SerialProtocol1 : public SerialProtocolAbstract
{
    Q_OBJECT
    int timeoutTimerID;
    QMutex bufferMutex;
    GlucoseDownloadProgress downloadInfo;
    QByteArray lastRcvPacket;

public:
    explicit SerialProtocol1(SerialComm *serialComm = 0, QObject *parent = 0);
    ~SerialProtocol1();

    void setCommObject(SerialComm *serialComm);

    Sp::ProtocolState startDownload();
    Sp::ProtocolState syncTime();
    Sp::ProtocolState readTime();
    void cancelDownload();
    void readBleData() { return;}
    void readQcData() { return;}
    void doCommands(QList<Sp::ProtocolCommand> commands)
    {
        Q_UNUSED(commands);
        return;
    }

    qint64 requestCommand(const Sp::ProtocolCommand &command, QByteArray *arg1 = 0, QByteArray *arg2 = 0, QByteArray *arg3 = 0);

    void parseReceivedData(QByteArray rcvPacket);

    const QByteArray &lastReceivePacket();

    ushort m_data_address;
    int m_sn_flag;
    int m_nMemory;
    int m_nMaxMeterCount;
    int m_nSavePoint;
    int m_nReqMaxCnt;

private slots:
    void readyRead();
    void error(QSerialPort::SerialPortError);
    void serialNumberType1Timeout();

protected:
    void connectSignals();
    void disconnectSignals();
    void timerEvent(QTimerEvent *event);

private:
    QByteArray beginCreatePacket();
    void endCreatePacket(QByteArray *array);

    QByteArray makeReadSerialNumber();
    QByteArray makeWriteTimeInformation();
    QByteArray makeCurrentIndexOfGluecose();
    QByteArray makeReadSavePoint();
    QByteArray makeGluecoseResultDataTx();    

    // Serial number type1 exception
    QTimer serialNumberType1ExceptionTimer;
    bool isSerialNumberType1;
    int waitingCount;


    inline bool packetSize(int value)
    {
        if(lastCommand == Sp::ReadSerialNumber)
        {
            if( (isSerialNumberType1 && value >= 24) || (isSerialNumberType1 == false && value >= 30 ))
                return true;
            else
                return false;
        }
        else if(lastCommand == Sp::GluecoseResultDataTx)
        {            
            return (REV1_GLUCOSE_SIZE * m_nReqMaxCnt <= value);
        }
        else if(lastCommand == Sp::CurrentIndexOfGluecose)
        {
            //qDebug() << "6";
            return (value >= 6);
        }
        else if(lastCommand == Sp::WriteTimeInformation)
        {
            //qDebug() << "17";
            return (value >= 17);
        }
        else if(lastCommand == Sp::ReadSavePoint)
        {
            //qDebug() << "17";
            return (value >= 6);
        }
        else
        {
            return (value >= 3);
        }
    }

    // receive packet processing
    bool isCompletePacketReceved();
    const QByteArray getReceivePacket();

    ushort getIndexOfGluecose(QByteArray rcvPacket);
    ushort getGluecoseCount(QByteArray rcvPacket);
    void processPacket(QByteArray rcvPacket);
    void produceError(Sp::ProtocolCommand receivedCommand, QByteArray rcvPacket);
    QJsonObject makeGlucoseObject(int value, QDateTime time)
    {
        QJsonObject data;
        data["index"] = "";
        data["dDate"] = QString::number(time.toMSecsSinceEpoch());
        data["time"] = "";//Settings::Instance()->GetDatetimestringFromMSec(QString::number(time.toMSecsSinceEpoch()));
        data["manual"] = m_serialnumber;
        data["flag_cs"] = "0";
        data["flag_meal"] = "0";
        data["flag_hilo"] = "0";
        data["glucose_unit"] = "mg/dL";

        int glucosevalue = value;
        if(value > GLUCOSE_MEAL_CHECK)
        {
            glucosevalue = value - GLUCOSE_MEAL_CHECK;
            data["flag_cs"] = "cs";
            data["flag_meal"] = "af";
        }
        else if(value > GLUCOSE_CHECK_DATA)
        {
            glucosevalue = value - GLUCOSE_CHECK_DATA;
            data["flag_cs"] = "cs";
        }
        else if(value > GLUCOSE_AFTER_MEAL)
        {
            glucosevalue = value - GLUCOSE_AFTER_MEAL;
            data["flag_meal"] = "af";
        }

        data["glucose_data"] = QString().sprintf("%d", glucosevalue);
        if(glucosevalue == 10)
        {
            data["flag_hilo"] = "Lo";
        }
        else if(glucosevalue == 700)
        {
            data["flag_hilo"] = "Hi";
        }

        Log() << "data : " << QJsonDocument(data).toJson();
        return data;
    }


};

#endif // SERIALPROTOCOL3_H
