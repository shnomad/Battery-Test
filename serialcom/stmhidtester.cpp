#include "stmhidtester.h"
#include "serialdefinition.h"

STMHIDTester::STMHIDTester(STMHIDPort *stmport , QObject *parent)
    : QObject(parent),
      selectstmport(stmport),
      protocol(Sp::CommProtocolUnknown)
{
    connect(selectstmport, SIGNAL(readyRead()), this, SLOT(readResponse()));
    connect(selectstmport, SIGNAL(timeoutErrorFromPort()), this, SLOT(timeoutErrorFromPort()));

    timeoutTimerID = startTimer(TESTER_TIMER_INTERVAL_STM32);

    isCheckState = false;
}

STMHIDTester::~STMHIDTester()
{
    if(selectstmport) {
        disconnect(selectstmport, SIGNAL(readyRead()), this, SLOT(checkResponse()));
        disconnect(selectstmport, SIGNAL(timeoutErrorFromPort()), this, SLOT(timeoutErrorFromPort()));

        selectstmport->close();
        delete selectstmport;
        selectstmport = Q_NULLPTR;
    }
}

void STMHIDTester::start()
{
    Log();
    isCheckState = false;
    const char c = 0x80;
    selectstmport->write(&c, 1);
}

void STMHIDTester::check()
{
    isCheckState = true;
    const char c = 0x80;
    selectstmport->write(&c, 1);

    if(timeoutTimerID)
    {
        Log() << "timeoutTimerID = " << timeoutTimerID;
        killTimer(timeoutTimerID); timeoutTimerID =0;
    }

    timeoutTimerID = startTimer(TESTER_TIMER_INTERVAL_STM32);
}

bool STMHIDTester::CheckState()
{
    Log();
    return isCheckState;
}

void STMHIDTester::unsetCheckState()
{
     isCheckState = false;
}

void STMHIDTester::readResponse()
{
    QByteArray read = selectstmport->readAll();
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
            disconnect(selectstmport, SIGNAL(readyRead()), this, SLOT(readResponse()));
            disconnect(selectstmport, SIGNAL(timeoutErrorFromPort()), this, SLOT(timeoutErrorFromPort()));
            connect(selectstmport, SIGNAL(readyRead()), this, SLOT(checkResponse()));
            connect(selectstmport, SIGNAL(timeoutErrorFromPort()), this, SLOT(timeoutErrorFromPort()));

            emit responseSuccess(this);
            return;
        }

        Log()<<"Sp::CommProtocolUnknown";

        emit responseUnknown(this);
    }
}

void STMHIDTester::checkResponse()
{
    Log();
    if(isCheckState == true)
    {
        Log() << "isCheckState true";
        QByteArray readed = selectstmport->readAll();
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

void STMHIDTester::timeoutErrorFromPort()
{
    Log();
    emit timeoutError(this);
}

void STMHIDTester::timerEvent(QTimerEvent *event) {

    Q_UNUSED(event);

    if(timeoutTimerID)
    {
        Log() << "timeoutTimerID = " << timeoutTimerID;
        killTimer(timeoutTimerID); timeoutTimerID = 0;
    }

    emit timeoutError(this);
}
