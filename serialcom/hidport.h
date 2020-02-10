#ifndef HIDPORT_H
#define HIDPORT_H

#include <QObject>
#include <QTimer>
#ifdef Q_OS_WIN
#include "HID/SiLab/Windows/SLABCP2110.h"
#include "HID/SiLab/Windows/SLABHIDtoUART.h"
#else
#include "HID/SiLab/MacOSX/SLABCP2110.h"
#endif

#define READ_SIZE   1000

class HIDPort : public QObject
{
    Q_OBJECT

    HID_UART_DEVICE hidUart;

    QByteArray m_buffer;
    int buffer_len;
    QTimer *recv_timer;
    bool busy;

public:
    explicit HIDPort(HID_UART_DEVICE hiduart, QObject *parent = 0);
    ~HIDPort();

    qint64 write(const char *data, qint64 len);
    qint64 write(QByteArray data);


    void ReceiveData();
    void Append(BYTE *data, DWORD len);
    QByteArray readAll();
    void close();
signals:
    void readyRead();

public slots:
     void checkReceived();
};

#endif // HIDPORT_H
