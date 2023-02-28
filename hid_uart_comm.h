#ifndef HID_UART_COMM_H
#define HID_UART_COMM_H

#include <QObject>
#include <QTimer>
#include <QSocketNotifier>
#include <QStringList>
#include "sys_cmd_resp.h"

class hid_uart_comm : public QObject
{
    Q_OBJECT
public:
    explicit hid_uart_comm(quint8, QObject *parent = nullptr);
    ~hid_uart_comm();

    void port_init();

signals:
    void sig_bgms_comm_response(sys_cmd_resp *);

public slots:

    void check_connection();
    void check_bgms();
    void ReadyRead();    

private:

    int fd;
    QTimer *comm_port_check_start, *bgms_check_start, *resp_timer;
    int response_time_msec = 200;

    QSocketNotifier *m_notify_hid, *m_notify_error;
    QByteArray m_tranferHost;

    quint8 cmd_buf[10]= {0x0,}, resp_buf[64] ={0x0,};
    const QStringList uart_hid_path ={"/dev/uart-hid-1", "/dev/uart-hid-2", "/dev/uart-hid-3"};
    QString hidpath;
    sys_cmd_resp *resp_bgms_comm;
};

#endif // HID_UART_COMM_H
