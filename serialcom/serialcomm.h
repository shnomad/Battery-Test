#ifndef SERIALCOM_H
#define SERIALCOM_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include "commondefinition.h"
#include "serialprotocolabstract.h"
#include "stmhidport.h"
#include "stmhidtester.h"
#include "serialdefinition.h"
//#include "hidapi.h"

enum PortType {
    PORT_SERIAL,    // Serial (mostly FTDI)
    PORT_HID_SiLab,       // SiLab HID
    PORT_HID_STM32L       // STM32L USB HID
};

class QSerialPortInfo;
class SerialPortTester;

class SerialComm : public QObject
{
    Q_OBJECT

private:
    //FT230x
    QList<SerialPortTester *> serialPortTesters;
    SerialPortTester *selectedSerialPortTester;
    QSerialPort *selectedSerialPort;

    // STM32L HID
    QList<STMHIDTester *> stmPortTesters;
    STMHIDTester *selectedStmPortTester;
    STMHIDPort *selectedStmPort;

    Sp::CommProtocolType selectedProtocol;
    int m_baudrate;
    PortType portType;

public:
    explicit SerialComm(QObject *parent = 0);
    ~SerialComm();

    enum intercface
    {
        micro_usb,
        phone_jack
    };

    Sp::CommProtocolType protocol();
    bool open(intercface);
    void close();
    void check();
    void unsetCheckState();
    bool isAvailable();

Q_SIGNALS:
    void textMessageSignal(QString text);
    void portReady();
    void connectionError();
    void readyRead();
//    void error(QSerialPort::SerialPortError);
    void maintainConnection(bool);

public slots:
    qint64 writeData(const char *data, qint64 size);
    qint64 writeData(QByteArray data);
    QByteArray readAll();

private Q_SLOTS:

    //QSerialPort, FT230x
     void timeoutError(SerialPortTester *sender);
    void responseUnknown(SerialPortTester *sender);
    void responseSuccess(SerialPortTester *sender);

    // STM32L HID
    void timeoutError(STMHIDTester *sender);
    void responseUnknown(STMHIDTester *sender);
    void responseSuccess(STMHIDTester *sender);

    void timeoutErrorFromPort();    //
    void textMessage(QString text);

private:

    // STM32L HID
    bool checkStmPort();

    //FT230x
    bool checkSerialPort();
    QSerialPort *openSerialPort(const QSerialPortInfo &info);
};

#endif // SERIALCOM_H
