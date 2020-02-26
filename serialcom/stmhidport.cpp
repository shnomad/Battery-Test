#include "stmhidport.h"
#include "serialdefinition.h"

STMHIDPort::STMHIDPort(hid_device *hiddevice, QObject *parent) : QObject(parent), m_hid_device(hiddevice)
{
    recv_timer = new QTimer(this);
    connect(recv_timer, SIGNAL(timeout()), this, SLOT(checkReceived()));
    recv_timer->start(RECV_TIMER_INTERVAL);
    busy = false;

    //m_buffer = new QByteArray();
}

STMHIDPort::~STMHIDPort()
{
    close();
}

void STMHIDPort::checkReceived()
{
    if(busy != true)
    {
        ReceiveData();
        //recv_timer->stop();
        //Log() << "checkReceived(in) ";
    }
    //Log() << "checkReceived(out)";
}

void STMHIDPort::Append(quint8 *data, quint32 len)
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

void STMHIDPort::logchar(unsigned char ch)
{
    if ( ('a' <= ch && ch <='z') ||  ('A' <= ch && ch <='Z') )
    {
         Log() << QString().sprintf("read : %02x %c", ch, ch);
    } else
    {
         Log() << QString().sprintf("read : %02x", ch);
    }
}

#define INTERVAL_MULTIPLYER 0
#define INTERVAL_AFTER_WRITE (20 * INTERVAL_MULTIPLYER)
#define INTERVAL_AFTER_RECEIVE (00 * INTERVAL_MULTIPLYER)
#define INTERVAL_BTWN_READ (00 * INTERVAL_MULTIPLYER)

// STM32_DEBUG
// 읽다 말고 무한대기 (회색화면)
static unsigned long count_ReceiveData_ZeroByte = 0;

void STMHIDPort::ReceiveData()
{
    Log() << "STMHIDPort::ReceiveData()";
    busy = true;

    quint32 numberOfBytesRead;
    unsigned char buf[65];
    #define MAX_STR 255
    wchar_t wstr[MAX_STR];
    quint32 totalBytesRead_ActualData = 0;
    //DWORD totalBytesRead_ActualData = 0;

    // Set the hid_read() function to be non-blocking.
    hid_set_nonblocking(m_hid_device, 1);

    // Read requested state
    for(int j=0; j<MAX_READ_TRY; j++)
    {
        numberOfBytesRead = hid_read(m_hid_device, buf, 64);

        if (numberOfBytesRead <= 0)
        {
            count_ReceiveData_ZeroByte ++;

            if(count_ReceiveData_ZeroByte > ((float)TESTER_TIMER_INTERVAL_STM32*2)/(float)RECV_TIMER_INTERVAL )
            {
                // STM32_DEBUG
                // 읽다 말고 멈춤 1 (회색화면)
                close();
                Log() << "###6 read buffer empty - try to send timeoutErrorFromPort()";
                count_ReceiveData_ZeroByte = 0;
                emit timeoutErrorFromPort();
                return;
            }
            //Log() << "read buffer empty - exiting.";
            break;
        }

        count_ReceiveData_ZeroByte = 0;

        logchar(buf[0]);
        logchar(buf[1]);

        if(buf[0] == 0x02)
        {
            for(int k=0; k<buf[1]; k++ )
            {
                logchar(buf[k+2]);
                m_buffer.append(buf[k+2]);
                totalBytesRead_ActualData += 1;
            }
        }
    }

    if(m_hid_device==NULL) return;

    hid_set_nonblocking(m_hid_device, 0);

       Log() << "totalBytesRead_ActualData: " << totalBytesRead_ActualData;

    if (totalBytesRead_ActualData > 0)
    {
        buffer_len = totalBytesRead_ActualData;
        Log() << "readBuf len: " << buffer_len;
        Log() << "readBuf: " << m_buffer;
        emit readyRead();
    }
    else
    {
//        LOG(GetStatusString(m_status) + _T("\r\n"));
        Log() << "checkReceived22 ";
    }

    // HidUart_Read returns HID_UART_SUCCESS if numBytesRead == numBytesToRead
    // and returns HID_UART_READ_TIMED_OUT if numBytesRead < numBytesToRead
    // HIDPort: if (status == HID_UART_SUCCESS || status == HID_UART_READ_TIMED_OUT)

//    Log() << "busy status " << status << "len =" << numBytesRead;
    busy = false;
}

QByteArray STMHIDPort::readAll()
{
    Log();
    return m_buffer;
}

qint64 STMHIDPort::writeWithPaddingData(QByteArray data_array)
{
    Log() << "STMHIDPort::writeWithPaddingData()";
    busy = true;

    if(buffer_len > 0)
    {
        buffer_len = 0;
        m_buffer = Q_NULLPTR;
    }

    int numberOfBytesWritten;
    unsigned char buf[64];
    #define MAX_STR 255
    wchar_t wstr[MAX_STR];
    quint32 totalBytesWritten = 0;

    buf[0] = 0x01;
    buf[1] = data_array.length();

    for(int i=0; i<data_array.length(); i++)
    {
        buf[i+2] = data_array[i];
    }
    for(int i=0;i<data_array.length()+2;i++)
    {
        logchar(buf[i]);
    }

    numberOfBytesWritten = hid_write(m_hid_device, buf, 64);
    totalBytesWritten += numberOfBytesWritten;

    if (totalBytesWritten > 0)
    {
        Log() << "write bytes : " << data_array.length() << totalBytesWritten;
    }

    busy = false;
    return totalBytesWritten;
}

qint64 STMHIDPort::write(QByteArray data_array)
{
    Log();
    return writeWithPaddingData(data_array);
}

qint64 STMHIDPort::write(const char *data, qint64 len)
{
    busy = true;
    QByteArray data_array(data, len);
    Log() << "STMHIDPort::write " << data_array.toHex();
    qint64 result = writeWithPaddingData(data_array);
    busy = false;

    return result;
}

void STMHIDPort::close()
{
    if(recv_timer != Q_NULLPTR)
    {
        recv_timer->stop();
        recv_timer = Q_NULLPTR;
    }

    if(m_hid_device != NULL)
    {
        //HidUart_Close(hidUart);
        hid_close(m_hid_device);
        m_hid_device = NULL;
    }
}
