#include "serialdefinition.h"
#include "serialcomm.h"
#include "serialporttester.h"
#include "settings.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QList>

//STM32L
#define VID_STM32L      0x483
#define PID_STM32L      0xa18B

#define READ_TIMEOUT		0//100
#define WRITE_TIMEOUT		100//2000

SerialComm::SerialComm(QObject *parent) : QObject(parent), selectedSerialPort(Q_NULLPTR),selectedProtocol(Sp::CommProtocolUnknown)
{
    Log() << "+SerialComm::SerialComm() " << "make serialTester = null";
    portType = PORT_HID_STM32L;
    selectedSerialPort = Q_NULLPTR;
    selectedSerialPortTester = Q_NULLPTR;
    selectedStmPort = Q_NULLPTR;
    selectedStmPortTester = Q_NULLPTR;
    m_baudrate = Settings::Instance()->getBaudrate();
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

    if(portType == PORT_HID_STM32L)
    {
        if(selectedStmPortTester != Q_NULLPTR)
        {
            selectedStmPortTester->check();
        }
    }
    else
    {
        if(selectedSerialPortTester != Q_NULLPTR)
        {
            selectedSerialPortTester->check();
            Log() << "selectedSerialPortTester check()";
        }
    }
}

bool SerialComm::isAvailable()
{
    if(portType == PORT_HID_STM32L)
    {
        if(selectedStmPortTester == Q_NULLPTR)
        {
            return false;
        }
    }
    else
    {
        if(selectedSerialPortTester == Q_NULLPTR)
        {
           return false;
        }
    }
        return true;
}

void SerialComm::unsetCheckState()
{
    if(portType == PORT_HID_STM32L)
    {
        if(selectedStmPortTester != Q_NULLPTR)
        {
            selectedStmPortTester->unsetCheckState();
        }
    }
    else
    {
        if(selectedSerialPortTester != Q_NULLPTR)
        {
            selectedSerialPortTester->unsetCheckState();
        }

    }
}

QSerialPort *SerialComm::openSerialPort(const QSerialPortInfo &info)
{
    QSerialPort *serial = new QSerialPort();
    serial->setPort(info);
    serial->setBaudRate(m_baudrate);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    if(serial->open(QIODevice::ReadWrite))
    {
        return serial;
    }

    return Q_NULLPTR;
}


bool SerialComm::open(SerialComm::intercface port)
{

    if(port == micro_usb)
    {
        if(checkStmPort() == false)
        {
            // connectionError!!!
            Log() << "Failed to open STM32 port!";

            return false;
        }
    }
    else
    {
        if(checkSerialPort() == false)
        {
            // connectionError!!!
            Log() << "Failed to open FT230x port!";

            return false;
        }
    }

    return true;
}

void SerialComm::close()
{
    if(portType == PORT_HID_STM32L)
    {
        if(selectedStmPort)
        {
            Log();
            selectedStmPort->close();
            selectedStmPort = Q_NULLPTR;
        }
    }
    else
    {
        if(selectedSerialPort)
        {
            Log();
            selectedSerialPort->close();
            selectedSerialPort = Q_NULLPTR;
        }
    }

    deleteLater();
}


qint64 SerialComm::writeData(const char *data, qint64 size)
{
    if(portType == PORT_HID_STM32L)
    {
        return selectedStmPort->write(data,size);
    }
    else
    {
        return selectedSerialPort->write(data, size);
    }
}

qint64 SerialComm::writeData(QByteArray data)
{
    qint64 serialComm;

    if(portType == PORT_HID_STM32L)
    {
        serialComm = selectedStmPort->write(data);
    }
    else
    {
        serialComm = selectedSerialPort->write(data);
    }

    if(data.contains("RTIM"))
    {
 //       Settings::Instance()->setSystemTime(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    }

    return serialComm;
}

QByteArray SerialComm::readAll()
{
    if(portType == PORT_HID_STM32L)
    {
        return selectedStmPort->readAll();
    }
    else
    {
        return selectedSerialPort->readAll();
    }
}

//FT230x
bool SerialComm::checkSerialPort()
{
     Log() << "checkSerialPort!!";
    QList<QSerialPortInfo> portInfos = QSerialPortInfo::availablePorts();

    int valid_portnum = portInfos.count();

    foreach (const QSerialPortInfo &info, portInfos)
    {

         Log() << info.portName();

         if(info.portName() == "ttyUSB0")
         {

            QSerialPort *port = openSerialPort(info);

            Log() << "Serial Port open success" << port->portName();

            portType = PORT_SERIAL;

            Log() << QString().sprintf("baudrate: %d", port->baudRate());
            Log() << QString().sprintf("data: %d", port->dataBits());
            Log() << QString().sprintf("parity: %d", port->parity());
            Log() << QString().sprintf("stop bits: %d", port->stopBits());
            Log() << QString().sprintf("flow control: %d", port->flowControl());

            SerialPortTester *serialtester = new SerialPortTester(port);    // PortTester => MeterTester
            serialPortTesters.append(serialtester);
            connect(serialtester, SIGNAL(timeoutError(SerialPortTester *)), this, SLOT(timeoutError(SerialPortTester *)));
            connect(serialtester, SIGNAL(responseUnknown(SerialPortTester *)), this, SLOT(responseUnknown(SerialPortTester *)));
            connect(serialtester, SIGNAL(responseSuccess(SerialPortTester *)), this, SLOT(responseSuccess(SerialPortTester *)));
            serialtester->start();
            Log() << "serialTester start()~~";
        }
        else
        {
            valid_portnum--;
        }
    }

    if(valid_portnum == 0)
    {
        emit connectionError();
        return false;
    }

    if(serialPortTesters.length() > 0)
        return true;
    return false;
}

void SerialComm::timeoutError(SerialPortTester *sender)
{
    if(selectedSerialPortTester != Q_NULLPTR)
    {
        Log() << "timeoutError, serialTester != null, make serialTester = null";
        selectedSerialPortTester = Q_NULLPTR;
        //연결해제
        emit maintainConnection(false);
    }
    else
    {
        Log() << "SerialComm::timeoutError serialTester == null, connection Error, make serialComm = null";
        serialPortTesters.removeOne(sender);
        sender->deleteLater();

        if(serialPortTesters.count() <= 0)
        {
            if(selectedSerialPort == Q_NULLPTR)
            {
                emit connectionError();
            }
        }
    }
}


void SerialComm::responseUnknown(SerialPortTester *sender)
{
    Log() << "SerialComm::responseUnknown";
    serialPortTesters.removeOne(sender);
    sender->deleteLater();
    if(serialPortTesters.count() <= 0) {
        if(selectedSerialPort == Q_NULLPTR)
        {
            emit connectionError();
        }
    }
}

void SerialComm::responseSuccess(SerialPortTester *sender)
{
    Log() << "responseSuccess, make selectedSerialPortTester ";
    if(selectedSerialPortTester != Q_NULLPTR || sender->CheckState() == true)
    {
        Log();
        emit maintainConnection(true);
        return;
    }

    foreach (SerialPortTester *tester, serialPortTesters)
    {
        if(tester == sender)
        {
            // 포트가 결정되었으니 필요한 시그널을 연결한다
            selectedSerialPort = tester->serialPort;
            connect(selectedSerialPort, SIGNAL(readyRead()), this, SIGNAL(readyRead()));
            connect(selectedSerialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SIGNAL(error(QSerialPort::SerialPortError)));

            selectedProtocol = tester->protocol;
            selectedSerialPortTester = tester;
            //tester->serialPort = Q_NULLPTR;
            //serialPortTesters.removeOne(tester);
            //delete tester;
        } else
        {
            disconnect(tester, SIGNAL(timeoutError(SerialPortTester *)), this, SLOT(timeoutError(SerialPortTester *)));
            disconnect(tester, SIGNAL(responseUnknown(SerialPortTester *)), this, SLOT(responseUnknown(SerialPortTester *)));
            disconnect(tester, SIGNAL(responseSuccess(SerialPortTester *)), this, SLOT(responseSuccess(SerialPortTester *)));
            delete tester;
        }
    }
    emit portReady();
}

//STM32
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
 //     handle = hid_open(VID_CP2110, PID_CP2110, NULL);
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

void SerialComm::textMessage(QString text)
{
    emit textMessageSignal(text);
}


Sp::CommProtocolType SerialComm::protocol()
{
    return selectedProtocol;
}

void SerialComm::timeoutError(STMHIDTester *sender)
{
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



void SerialComm::timeoutErrorFromPort()
{
    Log();
    emit connectionError();
}

// STM32 Port control
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
