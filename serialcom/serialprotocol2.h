#ifndef SERIALPROTOCOL2_H
#define SERIALPROTOCOL2_H

#include "serialprotocolabstract.h"
#include "serialcomm.h"
#include <QTimer>
#include <QMutex>

class SerialComm;
class QTimer;

#define kProtocol2TimeoutDuration   (5000)

class SerialProtocol2 : public SerialProtocolAbstract
{
    Q_OBJECT
    QMutex bufferMutex;
    GlucoseDownloadProgress downloadInfo;
    QByteArray lastRcvPacket;

public:
    explicit SerialProtocol2(SerialComm *serialComm = 0, QObject *parent = 0);
    ~SerialProtocol2();

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

private slots:
    void readyRead();
    void error(QSerialPort::SerialPortError);


protected:
    void connectSignals();
    void disconnectSignals();
    void timerEvent(QTimerEvent *event);

private:
    QByteArray beginCreatePacket();
    void endCreatePacket(QByteArray *array);
    QByteArray makeGluecoseResultDataTx(QByteArray indexArray);
    QByteArray makeGluecoseResultDataTxExpanded(QByteArray indexArray, QByteArray countArray);
    QByteArray makeCurrentIndexOfGluecose();
    QByteArray makeReadSerialNumber();
    QByteArray makeWriteSerialNumber(QByteArray sn);
    QByteArray makeReadTimeInformation();
    QByteArray makeWriteTimeInformation();
    QByteArray makeSaveData();
    QByteArray makeDeleteData();
    int timeoutTimerID;

    inline int headerSize() { return 5; }           // STX+iSPC
    inline int packetHeaderSize() { return 10; }    // STX+iSPC+size+Command
    inline int dataSize(QByteArray packetData) {
        uchar s = (uchar)packetData.at(headerSize());
        return (int)s;
    }

    // receive packet processing
    bool isCompletePacketReceved();
    const QByteArray getReceivePacket();
    Sp::ProtocolCommand getCommand(QByteArray rcvPacket);
    ushort getIndexOfGluecose(QByteArray rcvPacket);
    ushort getGluecoseCount(QByteArray rcvPacket);
    void processPacket(QByteArray rcvPacket);
    void produceError(Sp::ProtocolCommand receivedCommand, QByteArray rcvPacket);

    QDateTime m_current_time;


};

#endif // SERIALPROTOCOL2_H
