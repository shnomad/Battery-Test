#ifndef STM32HID_H
#define STM32HID_H

#include <QObject>
#include <QTimer>
#include <libusb-1.0/libusb.h>

#define MAX_READ_TRY   65536

class STMHIDPort : public QObject
{
    Q_OBJECT

//  hid_device *m_hid_device;
    libusb_device_handle *m_hid_device;
    QByteArray m_buffer;
    int buffer_len;
    QTimer *recv_timer;
    bool busy;

public:
//    explicit STMHIDPort(hid_device *hiddevice, QObject *parent = 0);
     explicit STMHIDPort(QObject *parent = 0);
    ~STMHIDPort();

    qint64 write(const char *data, qint64 len);
    qint64 write(QByteArray data_array);
    qint64 writeWithPaddingData(QByteArray data_array); // 0x01 [byte] ...

    void ReceiveData();
//    void Append(BYTE *data, DWORD len);
    void Append(quint8 *data, quint32 len);
    QByteArray readAll();
    void close();

    void logchar(unsigned char ch);

signals:
    void readyRead();
    void timeoutErrorFromPort();

public slots:
     void checkReceived();
};

#endif // STM32HID_H
