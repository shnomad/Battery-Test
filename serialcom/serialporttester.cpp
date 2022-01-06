#include "serialporttester.h"
#include <QtSerialPort/QSerialPort>
#include "serialprotocolabstract.h"
#include "serialdefinition.h"


SerialPortTester::SerialPortTester(QSerialPort *port, QObject *parent) : QObject(parent),
    serialPort(port),
    protocol(Sp::CommProtocolUnknown),
    timeoutTimerID(0)
{
    Q_ASSERT(port);

    Log();

    isCheckState = false;
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(readResponse()));

    timeoutTimerID = startTimer(TESTER_TIMER_INTERVAL);
}

SerialPortTester::~SerialPortTester()
{
    Log() << serialPort;

    if(serialPort)
    {
        disconnect(serialPort, SIGNAL(readyRead()), this, SLOT(readResponse()));
        serialPort->close();
        delete serialPort; serialPort = Q_NULLPTR;
    }
}

void SerialPortTester::start()
{
    Log();
    isCheckState = false;
    const char c = 0x80;
    serialPort->write(&c, 1);
}

void SerialPortTester::check()
{
    Log();
    isCheckState = true;
    const char c = 0x80;
    serialPort->write(&c, 1);

    if(timeoutTimerID)
    {
        Log() << "timeoutTimerID = " << timeoutTimerID;
        killTimer(timeoutTimerID); timeoutTimerID =0;
    }

    timeoutTimerID = startTimer(TESTER_TIMER_INTERVAL);
}

bool SerialPortTester::CheckState()
{
    return isCheckState;
}

void SerialPortTester::unsetCheckState()
{
     isCheckState = false;
}

void SerialPortTester::readResponse()
{

#if 0
    Log() << "read response~~";
    QByteArray readed = serialPort->readAll();
    receivedData.append(readed);
    Log() << receivedData.toHex();

    if(receivedData.length() >= 3)
    {
        if(timeoutTimerID)
        {
            Log() << "timeoutTimerID = " << timeoutTimerID;
            killTimer(timeoutTimerID); timeoutTimerID =0;
        }

        protocol = SerialProtocolAbstract::checkProtocol(receivedData);

        if(protocol != Sp::CommProtocolUnknown)
        {
            disconnect(serialPort, SIGNAL(readyRead()), this, SLOT(readResponse()));
            connect(serialPort, SIGNAL(readyRead()), this, SLOT(checkResponse()));

            emit responseSuccess(this);

            return;
        }

        emit responseUnknown(this);
    }

#endif

}

void SerialPortTester::checkResponse()
{
    Log() << "check Response~`";

#if 0
    if(isCheckState == true)
    {
        QByteArray readed = serialPort->readAll();
        receivedData.append(readed);
        Log() << receivedData.toHex();

        if(receivedData.length() >= 3)
        {
            if(timeoutTimerID)
            {
                Log() << "timeoutTimerID = " << timeoutTimerID;
                killTimer(timeoutTimerID); timeoutTimerID =0;
            }

            protocol = SerialProtocolAbstract::checkProtocol(receivedData);

            if(protocol != Sp::CommProtocolUnknown)
            {
                emit responseSuccess(this);
                return;
            }
            emit responseUnknown(this);
        }    
    }

#endif

}

void SerialPortTester::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    Log() << serialPort;

    if(timeoutTimerID)
    {
        Log() << "timeoutTimerID = " << timeoutTimerID;
        killTimer(timeoutTimerID); timeoutTimerID =0;
    }

    emit timeoutError(this);
}
