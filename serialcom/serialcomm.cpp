#include "serialdefinition.h"
#include "serialcomm.h"
#include <QList>

//HID definition
#define VID_CP2110      0x10C4
#define PID_CP2110      0xEA80 // CP2110 default value
#define PID_ISENS       0x8A32 // 0x8A32 (i-SENS의 HID cable에 사용할 Product ID)
//STM32L
#define VID_STM32L      0x483
#define PID_STM32L      0xA18B

#define READ_TIMEOUT		0//100
#define WRITE_TIMEOUT		100//2000

SerialComm::SerialComm(QObject *parent) : QObject(parent), selectedProtocol(Sp::CommProtocolUnknown)
{
    Log() << "+SerialComm::SerialComm() " << "make serialTester = null";
    portType = PORT_HID_STM32L;
    selectedStmPort = Q_NULLPTR;
    selectedStmPortTester = Q_NULLPTR;

}

SerialComm::~SerialComm()
{
    Log() << "+SerialComm::~SerialComm()" << stmPortTesters.length();

    close();

    if(stmPortTesters.length()>0)
    {
        foreach (const STMHIDTester *tester, stmPortTesters)
        {
            disconnect(tester, SIGNAL(timeoutError(STMHIDTester *)), this, SLOT(timeoutError(STMHIDTester *)));
            disconnect(tester, SIGNAL(responseUnknown(STMHIDTester *)), this, SLOT(responseUnknown(STMHIDTester *)));
            disconnect(tester, SIGNAL(responseSuccess(STMHIDTester *)), this, SLOT(responseSuccess(STMHIDTester *)));
            delete tester; tester = Q_NULLPTR;
        }
        stmPortTesters.clear();
    }

    Log() << "-SerialComm::~SerialComm()";
}

void SerialComm::check()
{
    Log() << "check()";

    selectedStmPortTester->check();

}

bool SerialComm::isAvailable()
{
     if(selectedStmPortTester == Q_NULLPTR)
     {
         return false;
     }
    return true;
}

void SerialComm::unsetCheckState()
{
    selectedStmPortTester->unsetCheckState();
}


bool SerialComm::open()
{
//    Settings::Instance()->setQcStringValue("hid_pid", "");
    if(checkStmPort() == false)
    {
        // connectionError!!!
        Log() << "Failed to open port!";
        return false;
    }

    return true;
}

bool SerialComm::checkStmPort()
{
    Log() << "checkStmPort!!";
    // hidapi
    // http://www.signal11.us/oss/hidapi/

    int res = 0;
    #define MAX_STR 255
    wchar_t wstr[MAX_STR];
    hid_device *handle;

    // Enumerate and print the HID devices on the system
    struct hid_device_info *devs, *cur_dev;

    devs = hid_enumerate(VID_STM32L, PID_STM32L);
    cur_dev = devs;
    while (cur_dev) {

        Log() << "Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls" <<
                                        cur_dev->vendor_id << cur_dev->product_id << cur_dev->path <<cur_dev->serial_number;
        Log() << QString().sprintf("Device Found\n  type: %04x %04x\n  path: %s\n  serial_number: %ls",
                                        cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
        Log() << "  Manufacturer: " <<  cur_dev->manufacturer_string;
        Log() << "  Product:      " << cur_dev->product_string;

        cur_dev = cur_dev->next;
    }

    hid_free_enumeration(devs);

    // Open the device using the VID, PID,
    // and optionally the Serial number.  // vid_0483&pid_a18b
      handle = hid_open(VID_STM32L, PID_STM32L, NULL);
    //handle = hid_open(VID_CP2110, PID_ISENS, NULL);
    //selectedStmPort->open(VID_STM32L, PID_STM32L,handle);

    if(handle) {
        // Read the Manufacturer String
        res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
        Log() << "Manufacturer String: " <<  QString::fromWCharArray(wstr);

        // Read the Product String
        res = hid_get_product_string(handle, wstr, MAX_STR);
        Log() << "Product String: " <<  QString::fromWCharArray(wstr);

        // Read the Serial Number String
        res = hid_get_serial_number_string(handle, wstr, MAX_STR);
        Log() << "Serial Number String: " << QString::fromWCharArray(wstr);

        STMHIDPort *tempstmport = new STMHIDPort(handle);
        connect(tempstmport, SIGNAL(timeoutErrorFromPort()), this, SLOT(timeoutErrorFromPort()));

        Log() << "STM32L Hid device open success";
        portType = PORT_HID_STM32L;

        STMHIDTester *stmtester = new STMHIDTester(tempstmport);
        stmPortTesters.append(stmtester);
        Log() << "###5 " << stmtester;

        connect(stmtester, SIGNAL(timeoutError(STMHIDTester *)), this, SLOT(timeoutError(STMHIDTester *)));
        connect(stmtester, SIGNAL(responseUnknown(STMHIDTester *)), this, SLOT(responseUnknown(STMHIDTester *)));
        connect(stmtester, SIGNAL(responseSuccess(STMHIDTester *)), this, SLOT(responseSuccess(STMHIDTester *)));

        stmtester->start();
    }

    if(stmPortTesters.length() > 0)
        return true;

    return false;
}

void SerialComm::textMessage(QString text) {
    emit textMessageSignal(text);
}


Sp::CommProtocolType SerialComm::protocol() {
    return selectedProtocol;
}

void SerialComm::close()
{

    if(selectedStmPort)
    {
        selectedStmPort->close();
        //delete selectedStmPort;
        selectedStmPort = Q_NULLPTR;
    }

    deleteLater();
}


// STM32L HID
void SerialComm::timeoutError(STMHIDTester *sender) {
    Log();

    if(selectedStmPortTester != Q_NULLPTR)
    {
        Log() << "timeoutError, stmTester != null, make stmTester = null";
        selectedStmPortTester = Q_NULLPTR;
        //연결해제 시그널 발생
        emit maintainConnection(false);
    }
    else
    {
        Log() << "timeoutError, stmTester == null, connection Error, make serialComm = null";
        stmPortTesters.removeOne(sender);
        sender->deleteLater();
        if(stmPortTesters.count() <= 0) {
            if(selectedStmPort  == Q_NULLPTR)
            {
                emit connectionError();
                //checkHidPort();
            }
        }
    }
}

void SerialComm::timeoutErrorFromPort() {
    Log();
    emit connectionError();
}

void SerialComm::responseUnknown(STMHIDTester *sender)
{
    stmPortTesters.removeOne(sender);
    sender->deleteLater();
    if(stmPortTesters.count() <= 0)
    {
        if(selectedStmPort == Q_NULLPTR)
        {
            //emit connectionError();
//            checkHidPort();
              emit connectionError();
        }
    }
}

void SerialComm::responseSuccess(STMHIDTester *sender)
{
    if(selectedStmPortTester != Q_NULLPTR || sender->CheckState() == true)
    {
        Log();
        emit maintainConnection(true);
        return;
    }

    foreach (STMHIDTester *tester, stmPortTesters)
    {
        if(tester == sender)
        {
            // 포트가 결정되었으니 필요한 시그널을 연결한다
            selectedStmPort = tester->selectstmport;
            connect(selectedStmPort, SIGNAL(readyRead()), this, SIGNAL(readyRead()));
            //connect(selectedSerialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SIGNAL(error(QSerialPort::SerialPortError)));

            selectedProtocol = tester->protocol;
            Log() << "###4 " << tester;
            selectedStmPortTester = tester;
            //tester->selectstmport = Q_NULLPTR;
            //tester->deleteLater();
            //stmPortTesters.removeAt(stmPortTesters.indexOf(tester));

            break;
        }
        else
        {
            Log() << "###3 " << tester;
            disconnect(tester, SIGNAL(timeoutError(STMHIDTester *)), this, SLOT(timeoutError(STMHIDTester *)));
            disconnect(tester, SIGNAL(responseUnknown(STMHIDTester *)), this, SLOT(responseUnknown(STMHIDTester *)));
            disconnect(tester, SIGNAL(responseSuccess(STMHIDTester *)), this, SLOT(responseSuccess(STMHIDTester *)));
            delete tester;
        }
    }

    while(stmPortTesters.count()) {
        stmPortTesters.removeLast();
    }

    emit portReady();
}

qint64 SerialComm::writeData(const char *data, qint64 size)
{
    return selectedStmPort->write(data,size);
}

qint64 SerialComm::writeData(QByteArray data)
{
    qint64 serialComm;

     serialComm = selectedStmPort->write(data);

    if(data.contains("RTIM"))
    {
//        Settings::Instance()->setSystemTime(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }

    return serialComm;
}

QByteArray SerialComm::readAll()
{
    return selectedStmPort->readAll();
}
