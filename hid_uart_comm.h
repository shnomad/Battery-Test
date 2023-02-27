#ifndef HID_UART_COMM_H
#define HID_UART_COMM_H

#include <QObject>
#include <QTimer>
#include <QSocketNotifier>

class hid_uart_comm : public QObject
{
    Q_OBJECT
public:
    explicit hid_uart_comm(QObject *parent = nullptr);
    ~hid_uart_comm();

    void uart_init();

signals:

public slots:

    void Send_Command();
    void ReadyRead();    

private:

    int fd;
    QTimer *comm_start, *resp_timer;
    int response_time_msec = 200;

    QSocketNotifier *m_notify_hid, *m_notify_error;
    QByteArray m_tranferHost;

    quint8 cmd_buf[10]= {0x0,}, resp_buf[64] ={0x0,};

};

#endif // HID_UART_COMM_H
