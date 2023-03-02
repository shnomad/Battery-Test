﻿#ifndef SERIALPROTOCOLABSTRACT_H
#define SERIALPROTOCOLABSTRACT_H

#include <QObject>
#include <qnamespace.h>
#include "serialprotocol.h"
//#include "glucosedownloadprogress.h"
#include <QTimer>
#include <QDateTime>
//#include <QtCore>

//class SerialComm;
class SerialProtocolAbstract : public QObject

{
    Q_OBJECT

//  GlucoseDownloadProgress downloadInfo;
//  explicit SerialProtocolAbstract(SerialComm *serailComm = 0, QObject *parent = 0);
    explicit SerialProtocolAbstract(QObject *parent = nullptr);
    ~SerialProtocolAbstract();

    Sp::ProtocolState state();

    Sp::ProtocolCommand getLastcommand()
    {
        return lastCommand;
    }

    Sp::MeterModelType modeltype;

//  virtual void setCommObject(SerialComm *serialComm) = 0;
    virtual Sp::ProtocolState startDownload() = 0;
    virtual Sp::ProtocolState syncTime() = 0;
    virtual Sp::ProtocolState readTime() = 0;

    virtual void cancelDownload() = 0;
    virtual qint64 requestCommand(const Sp::ProtocolCommand &command, QByteArray *arg1 = 0, QByteArray *arg2 = 0, QByteArray *arg3 = 0) = 0;
    virtual void parseReceivedData(QByteArray rcvPacket);
    virtual const QByteArray &lastReceivePacket() = 0;
    virtual void readBleData() = 0;
    virtual void readQcData() = 0;
    virtual void doCommands(QList<Sp::ProtocolCommand> commands) = 0;

    static ushort calcCrc(const QByteArray &array);
    static bool isEqualCrc(const QByteArray &array, ushort crc);
    static Sp::CommProtocolType checkProtocol(const QByteArray &array);
    inline bool isErrorResponse(Sp::ProtocolCommand command) { return (command >= Sp::CommunicationTimeout);}

    inline ushort byteswap(ushort value)
    {
        ushort v = value;
        char *vp = (char *)&v;
        char tmp = vp[0];
        vp[0] = vp[1];
        vp[1] = tmp;
        return v;
    }

    inline QByteArray argUShort(ushort value)
    {
        ushort ret = value;
        ret = byteswap(ret);
        return QByteArray((char *)&ret, 2);
    }

    inline QByteArray argByte(char value)
    {
        return QByteArray((char *)&value, 1);
    }

    bool isIdle();

    QString m_serialnumber;
//  QJsonArray m_dataArray;

signals:
    void timeoutError(Sp::ProtocolCommand command);
    void errorOccurred(Sp::ProtocolCommand command, Sp::ProtocolCommand preCommand = Sp::ProtocolCommandNone);
    void errorCrc();
    void errorUnresolvedCommand(Sp::ProtocolCommand command);
    void packetReceived();

    // UI
    void downloadProgress(float progress);                                  // 0~1 : 1 = 100%
    void downloadComplete(QJsonArray* datalist);

    // Serial
    void needReopenSerialComm();
    void completeTimeSync(QDateTime* meter_time, QDateTime* current_time);
    void completeReadTime(QDateTime* meter_time);
    void failTimeSync();
    void completeDelData();
    void completeReadSN(QString);

    void finishDoCommands(bool bSuccess, Sp::ProtocolCommand lastcommand);
    void finishReadingQcData();

public slots:

private:
    QByteArray beginCreatePacket();
    void endCreatePacket(QByteArray *array);
    QByteArray makeLockData();
    QByteArray makeUnlockData();

protected:
    Sp::ProtocolCommand lastCommand;
    Sp::ProtocolCommand pLastCommand;
//  SerialComm *comm;
    Sp::ProtocolState currentState;
    QByteArray readBuffer;

};

#endif // SERIALPROTOCOLABSTRACT_H
