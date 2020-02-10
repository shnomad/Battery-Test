#include "hidport.h"
#include "serialdefinition.h"

HIDPort::HIDPort(HID_UART_DEVICE uart, QObject *parent) : QObject(parent),
    hidUart(uart)
{
    recv_timer = new QTimer(this);
    connect(recv_timer, SIGNAL(timeout()), this, SLOT(checkReceived()));
    recv_timer->start(RECV_TIMER_INTERVAL);
    busy = false;

    //m_buffer = new QByteArray();
}

HIDPort::~HIDPort()
{
    if(recv_timer != Q_NULLPTR)
    {
        recv_timer->stop();
        recv_timer = Q_NULLPTR;
    }
    if(hidUart != NULL)
    {
        HidUart_Close(hidUart);
        hidUart = NULL;
    }

}
void HIDPort::checkReceived()
{
    if(busy != true)
    {
        ReceiveData();
        //recv_timer->stop();
        // Log() << "checkReceived(in) ";
    }
    //Log() << "checkReceived(out)";
}


void HIDPort::ReceiveData()
{
    busy = true;

    HID_UART_STATUS status = HID_UART_DEVICE_NOT_FOUND;

    DWORD numBytesRead = 0;

    BYTE temp[100];

    // Receive UART data from the device (up to READ_SIZE bytes)
    status = HidUart_Read(hidUart, temp, 100, &numBytesRead);

    // HidUart_Read returns HID_UART_SUCCESS if numBytesRead == numBytesToRead
    // and returns HID_UART_READ_TIMED_OUT if numBytesRead < numBytesToRead
    if (status == HID_UART_SUCCESS || status == HID_UART_READ_TIMED_OUT)
    {
        if (numBytesRead > 0)
        {
            //QByteArray qtemp;// = QByteArray::fromRawData(temp, numBytesRead);

            //qtemp = QByteArray((const char*)temp, numBytesRead);

            //Append(temp, numBytesRead);
            m_buffer = QByteArray::fromRawData((const char*)temp, numBytesRead);
            buffer_len = numBytesRead;

            emit readyRead();

             Log() << "len = " << buffer_len << "m_buffer = " << m_buffer.toHex();

            //if not called this func, occur ssegmentation fault
            //status = HidUart_FlushBuffers(hidUart, true, true);
            //status = HidUart_CancelIo(hidUart);
        }
    }
    else
    {
        //LOG(GetStatusString(m_status) + _T("\r\n"));
        //Log() << "checkReceived22 ";
    }

    //Log() << "busy status " << status << "len =" << numBytesRead;
    busy = false;
}

void HIDPort::Append(BYTE *data, DWORD len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);
    //QByteArray qtemp = QByteArray((const char*)data, len);
    //QByteArray qtemp = QByteArray((const char*)data, len);

    /*
    if(m_buffer.length() > 0)
    {
        m_buffer += qtemp;
    }
    else
    */
     //   m_buffer = qtemp;

     //Log() << "Append(-)" << m_buffer << "temp = " << qtemp;

}

QByteArray HIDPort::readAll()
{
    return m_buffer;
}

qint64 HIDPort::write(QByteArray data)
{
    //if(m_buffer.length())
    //    m_buffer.clear();
    if(buffer_len > 0)
    {
        buffer_len = 0;
        m_buffer = Q_NULLPTR;
    }

    qint64 result = 0;
    DWORD numBytesWritten = 0;

    //HID_UART_STATUS status = HID_UART_DEVICE_NOT_FOUND;

    //status = HidUart_Write(hidUart, (BYTE *)data.data(), DWORD(data.length()), &numBytesWritten);
    HidUart_Write(hidUart, (BYTE *)data.data(), DWORD(data.length()), &numBytesWritten);
    //const char c = 0x80;

    //status = HidUart_Write(hidUart, (BYTE *)&c, DWORD(1), &numBytesWritten);

    //int datalen = data.length();
    //Log() << "write qbyte " << datalen;
    //Log() << "write qbyte " << datalen;
    return result;
}

qint64 HIDPort::write(const char *data, qint64 len)
{
    //if(m_buffer.length())
    //    m_buffer.clear();
    if(buffer_len > 0)
    {
        buffer_len = 0;
        m_buffer = Q_NULLPTR;
    }

    qint64 result = 0;

    DWORD numBytesWritten = 0;

    //HID_UART_STATUS status = HID_UART_DEVICE_NOT_FOUND;

    //test
    //const char c = 0x80;

    //status = HidUart_Write(hidUart, (BYTE *)&c, DWORD(len), &numBytesWritten);aaaaa


    //status = HidUart_Write(hidUart, (BYTE *)data, DWORD(len), &numBytesWritten);
    HidUart_Write(hidUart, (BYTE *)data, DWORD(len), &numBytesWritten);

    //Log() << "write echo ";

    return result;
}

void HIDPort::close()
{
    if(recv_timer != Q_NULLPTR)
    {
        recv_timer->stop();
        recv_timer = Q_NULLPTR;
    }
    if(hidUart != NULL)
    {
        HID_UART_STATUS status = HidUart_Close(hidUart);
        Log() << "close status = " << status;
        hidUart = NULL;
    }
}
