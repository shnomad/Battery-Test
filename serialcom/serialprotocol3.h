#ifndef SERIALPROTOCOL3_H
#define SERIALPROTOCOL3_H

#include "serialprotocolabstract.h"
//#include "serialcomm.h"
#include "glucosedownloadprogress.h"
#include "settings.h"
#include <QTimer>
#include <QMutex>

class SerialComm;
class QTimer;

#define kProtocol3TimeoutDuration   (3000)
//#define SERIALCOM_SMARTLOG

class SerialProtocol3 : public SerialProtocolAbstract
{
    Q_OBJECT
    QMutex bufferMutex;
    GlucoseDownloadProgress downloadInfo;
    QByteArray lastRcvPacket;

public:
//   explicit SerialProtocol3(SerialComm *serialComm = 0, QObject *parent = 0);
     explicit SerialProtocol3(QObject *parent = nullptr);
    ~SerialProtocol3();

//  void setCommObject(SerialComm *serialComm);
    void setCommObject();
    Sp::ProtocolState startDownload();
    Sp::ProtocolState syncTime();
    Sp::ProtocolState readTime();
    Sp::ProtocolState readSerialNumber();
    Sp::ProtocolState deleteData();
    void cancelDownload();
    void readBleData();
    void readQcData();
    //void doCommands(QList<Sp::ProtocolCommand> commands);
    QByteArray doCommands(QList<Sp::ProtocolCommand> commands);

//  qint64 requestCommand(const Sp::ProtocolCommand &command, QByteArray *arg1 = 0, QByteArray *arg2 = 0, QByteArray *arg3 = 0);
    QByteArray requestCommand(const Sp::ProtocolCommand &command, QByteArray *arg1 = 0, QByteArray *arg2 = 0, QByteArray *arg3 = 0);

//  void processPacket(QByteArray rcvPacket);
    QByteArray processPacket(QByteArray rcvPacket);
    bool parseReceivedData(QByteArray rcvPacket);
    const QByteArray &lastReceivePacket();    

private slots:
    void readyRead();
//  void error(QSerialPort::SerialPortError);

protected:
    void connectSignals();
    void disconnectSignals();
    void timerEvent(QTimerEvent *event);

private:
    ushort m_data_address;

    QByteArray beginCreatePacket();
    void endCreatePacket(QByteArray *array);
    QByteArray makeGluecoseResultDataTx(QByteArray indexArray);
    QByteArray makeGluecoseResultDataTxExpanded(QByteArray indexArray, QByteArray countArray);
    QByteArray makeCurrentIndexOfGluecose();
    QByteArray makeReadSerialNumber();
    QByteArray makeWriteSerialNumber(QByteArray sn);
    QByteArray createWriteSerialNumber();
    QByteArray makeReadTimeInformation();
    QByteArray makeWriteTimeInformation();
    QByteArray makeSaveData();
    QByteArray makeDeleteData();
    QByteArray makeReadBLE();
    QByteArray makeWriteBLE();
    QByteArray makeChangeBLEMODE();
    QByteArray makeChangeBLEMODE_EXT();
    QByteArray makeReadAESKey();
    QByteArray makeWriteAESkey();
    QByteArray makeReadAPNNumber();
    QByteArray makeWriteAPNNumber();
    QByteArray makeReadServerAddress();
    QByteArray makeWriteServerAddress();
    QByteArray makeReadDeviceToken();
    QByteArray makeWriteDeviceToken();
    QByteArray makeReadServiceName();
    QByteArray makeWriteServiceName();
    QByteArray makeWriteSktSN();
    QByteArray makeReadIMEI();
    QByteArray makeReadICCID();
    QByteArray makeReadModuleSoftwareVer();
    QByteArray makeReadIMSI();
    QByteArray makeReadSktSN();
    QByteArray makeLockData();
    QByteArray makeUnlockData();
    QByteArray makeReadRegistrationURL();
    QByteArray makeWriteRegistrationURL();
    QByteArray makeWriteCodingMode();
    QByteArray makeReadCodingMode();
    QByteArray makeWriteOtgMode();
    QByteArray makeReadOtgMode();

    int timeoutTimerID;
    bool mOnlyReadSN;
    bool dataDownload = false;
    void requestNextQcData();

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
//  void processPacket(QByteArray rcvPacket);
    void produceError(Sp::ProtocolCommand receivedCommand, QByteArray rcvPacket);
    void parseQcData(QByteArray rcvPacket);
    void parseQcData_default(QByteArray rcvPacket);
    void parseQcData_cs_color(QByteArray rcvPacket);

    QDateTime m_current_time;

    QHash<QString, QVariant> m_settings;
    QString tempstring;
    QByteArray next_temparr;

    QList<Sp::ProtocolCommand> m_commands;
    int m_current_cmd_index;
    bool isBleCmd;
    bool isDBLE;

    float bytesToFloat(uchar b0, uchar b1, uchar b2, uchar b3);
};


#endif // SERIALPROTOCOL3_H
