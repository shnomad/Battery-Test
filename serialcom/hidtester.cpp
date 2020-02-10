#include "hidtester.h"
#include "serialdefinition.h"

HIDTester::HIDTester(HIDPort *hidport , QObject *parent)
    : QObject(parent),
      selecthidport(hidport),
      protocol(Sp::CommProtocolUnknown)
{    
    isCheckState = false;
    connect(selecthidport, SIGNAL(readyRead()), this, SLOT(readResponse()));
    timeoutTimerID = startTimer(TESTER_TIMER_INTERVAL);
}

HIDTester::~HIDTester()
{
    if(selecthidport) {
        disconnect(selecthidport, SIGNAL(readyRead()), this, SLOT(readResponse()));
        selecthidport->close();
        delete selecthidport;
        selecthidport = Q_NULLPTR;
    }

}

void HIDTester::start()
{
    isCheckState = false;
    const char c = 0x80;
    selecthidport->write(&c, 1);
}

void HIDTester::check()
{
    isCheckState = true;
    const char c = 0x80;
    selecthidport->write(&c, 1);
    if(timeoutTimerID) {
        Log() << "timeoutTimerID = " << timeoutTimerID;
        killTimer(timeoutTimerID); timeoutTimerID =0;
    }
    timeoutTimerID = startTimer(TESTER_TIMER_INTERVAL);
}

bool HIDTester::CheckState()
{
    return isCheckState;
}

void HIDTester::unsetCheckState()
{
     isCheckState = false;
}

void HIDTester::readResponse()
{
    QByteArray read = selecthidport->readAll();
    //Log() << "readResponse" << read.toHex();
    if(read.length() >= 3)
    {
        if(timeoutTimerID) {
            Log() << "timeoutTimerID = " << timeoutTimerID;
            killTimer(timeoutTimerID); timeoutTimerID =0;
        }
        protocol = SerialProtocolAbstract::checkProtocol(read);
        if(protocol != Sp::CommProtocolUnknown)
        {
            disconnect(selecthidport, SIGNAL(readyRead()), this, SLOT(readResponse()));
            connect(selecthidport, SIGNAL(readyRead()), this, SLOT(checkResponse()));
            emit responseSuccess(this);
            return;
        }
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

void HIDTester::timerEvent(QTimerEvent *event) {
    Q_UNUSED(event);
    if(timeoutTimerID) {
        Log() << "timeoutTimerID = " << timeoutTimerID;
        killTimer(timeoutTimerID); timeoutTimerID =0;
    }
    emit timeoutError(this);
}
