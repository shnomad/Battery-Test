#ifndef SERIALPORTTESTER_H
#define SERIALPORTTESTER_H

#include <QObject>
#include <QTimer>
#include "serialcomm.h"

class QSerialPort;

class SerialPortTester : public QObject
{
    Q_OBJECT

public:        
    QSerialPort *serialPort;
    Sp::CommProtocolType protocol;

    quint8 meter_check_retry_count=0;

public:
    explicit SerialPortTester(QSerialPort *port, QObject *parent = 0);
    ~SerialPortTester();

    void check();
    bool CheckState();
    void unsetCheckState();

Q_SIGNALS:
    void timeoutError(SerialPortTester *sender);
    void responseUnknown(SerialPortTester *sender);
    void responseSuccess(SerialPortTester *sender);

public Q_SLOTS:
    void start();

private Q_SLOTS:
    void readResponse();
    void checkResponse();

protected:
    void timerEvent(QTimerEvent *event);

private:
    int timeoutTimerID;
    QByteArray receivedData;
    bool isCheckState;
};

#endif // SERIALPORTTESTER_H
