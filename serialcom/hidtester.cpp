#include "hidtester.h"
#include "serialdefinition.h"

HIDTester::HIDTester(HIDPort *port , QObject *parent)
    : QObject(parent),
      selecthidport(port),
      protocol(Sp::CommProtocolUnknown)
{
    connect(selecthidport, SIGNAL(readyRead()), this, SLOT(readResponse()));
    connect(selecthidport, SIGNAL(timeoutErrorFromPort()), this, SLOT(timeoutErrorFromPort()));

    timeoutTimerID = startTimer(TESTER_TIMER_INTERVAL_STM32);

    isCheckState = false;
}

HIDTester::~HIDTester()
{
    if(selecthidport) {
        disconnect(selecthidport, SIGNAL(readyRead()), this, SLOT(checkResponse()));
        disconnect(selecthidport, SIGNAL(timeoutErrorFromPort()), this, SLOT(timeoutErrorFromPort()));

        selecthidport->close();
        delete selecthidport;
        selecthidport = Q_NULLPTR;
    }
}

void HIDTester::start()
{
    Log();
    isCheckState = false;
    const char c = 0x80;
    selecthidport->write(&c, 1);
}

void HIDTester::check()
{
    isCheckState = true;
    const char c = 0x80;
    selecthidport->write(&c, 1);

    if(timeoutTimerID)
    {
        Log() << "timeoutTimerID = " << timeoutTimerID;
        killTimer(timeoutTimerID); timeoutTimerID =0;
    }

    timeoutTimerID = startTimer(TESTER_TIMER_INTERVAL_STM32);
}

bool HIDTester::CheckState()
{
    Log();
    return isCheckState;
}

void HIDTester::unsetCheckState()
{
     isCheckState = false;
}

void HIDTester::readResponse()
{
    QByteArray read = selecthidport->readAll();
     Log() << "readResponse" << read.toHex();
    if(read.length() >= 3)
    {
        if(timeoutTimerID)
        {
            Log() << "killTimer = " << timeoutTimerID;
            killTimer(timeoutTimerID); timeoutTimerID = 0;
        }

        protocol = SerialProtocolAbstract::checkProtocol(read);

        Log()<<"protocol"<<protocol;

        if(protocol != Sp::CommProtocolUnknown)
        {
            disconnect(selecthidport, SIGNAL(readyRead()), this, SLOT(readResponse()));
            disconnect(selecthidport, SIGNAL(timeoutErrorFromPort()), this, SLOT(timeoutErrorFromPort()));
            connect(selecthidport, SIGNAL(readyRead()), this, SLOT(checkResponse()));
            connect(selecthidport, SIGNAL(timeoutErrorFromPort()), this, SLOT(timeoutErrorFromPort()));

            emit responseSuccess(this);
            return;
        }

        Log()<<"Sp::CommProtocolUnknown";

        emit responseUnknown(this);
    }
}

void HIDTester::checkResponse()
{
    Log();
    if(isCheckState == true)
    {
        Log() << "isCheckState true";
        QByteArray readed = selecthidport->readAll();
        receivedData.append(readed);
        if(receivedData.length() >= 3)
        {
            if(timeoutTimerID) {
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
}

void HIDTester::timeoutErrorFromPort()
{
    Log();
    emit timeoutError(this);
}

void HIDTester::timerEvent(QTimerEvent *event) {

    Q_UNUSED(event);

    if(timeoutTimerID)
    {
        Log() << "timeoutTimerID = " << timeoutTimerID;
        killTimer(timeoutTimerID); timeoutTimerID = 0;
    }

    emit timeoutError(this);
}
