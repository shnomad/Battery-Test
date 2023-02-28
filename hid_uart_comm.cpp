#include "hid_uart_comm.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>
#include "commondefinition.h"

hid_uart_comm::hid_uart_comm(quint8 channel, QObject *parent) : QObject(parent)
{ 
    comm_port_check_start = new QTimer(this);
    comm_port_check_start->setSingleShot(true);

    connect(comm_port_check_start, SIGNAL(timeout()), this, SLOT(check_connection()));

    hidpath = uart_hid_path[channel];

    comm_port_check_start->start(500);
}

void hid_uart_comm::check_connection()
{
    Log();

    resp_bgms_comm = new sys_cmd_resp;

    fd = open(hidpath.toLocal8Bit(), O_RDWR);

    if(fd <0)
    {
        resp_bgms_comm->m_comm_resp = sys_cmd_resp::RESP_COMM_PORT_OPEN_FAIL;
        emit sig_bgms_comm_response(resp_bgms_comm);
    }
    else
    {
        resp_bgms_comm->m_comm_resp = sys_cmd_resp::RESP_COMM_PORT_OPEN_SUCCESS;
        emit sig_bgms_comm_response(resp_bgms_comm);

        port_init();

        bgms_check_start = new QTimer(this);
        bgms_check_start->setSingleShot(true);

        connect(bgms_check_start, SIGNAL(timeout()), this, SLOT(check_bgms()));

        m_notify_hid = new QSocketNotifier(fd, QSocketNotifier::Read, this);
        m_notify_error = new QSocketNotifier(fd, QSocketNotifier::Exception, this);

        connect(m_notify_hid, SIGNAL(activated(int)), this, SLOT(ReadyRead()));
//      connect(m_notify_error, SIGNAL(activated(int)), this, SLOT(Error()));

        m_notify_hid->setEnabled(true);
        m_notify_error->setEnabled(true);

        resp_timer = new QTimer(this);
        resp_timer->setSingleShot(true);

        connect(resp_timer,  &QTimer::timeout, [=]()
        {
            Log()<<QString::fromUtf8(m_tranferHost.toHex());

            resp_bgms_comm->m_comm_resp = sys_cmd_resp::RESP_COMM_BGMS_CHECK_SUCCESS;
            emit sig_bgms_comm_response(resp_bgms_comm);
        });

        bgms_check_start->start();
    }
}

void hid_uart_comm::port_init()
{
    int result = 0;

    /*Set PL23B3 Uart Configuration*/

    cmd_buf[0] = 0x81;
    cmd_buf[1] = 0x0a;

    result = ioctl(fd, HIDIOCSFEATURE(2), cmd_buf);

    memset(cmd_buf,0x00,sizeof(cmd_buf));

    cmd_buf[0] = 0x81;
    cmd_buf[1] = 0x03;

    result = ioctl(fd, HIDIOCGFEATURE(2), cmd_buf);

    memset(cmd_buf,0x00,sizeof(cmd_buf));

    cmd_buf[0] = 0x90;
    cmd_buf[1] = 0x80;     //dwBaudRate    //9600 bps
    cmd_buf[2] = 0x25;     //dwBaudRate    //9600 bps
    cmd_buf[3] = 0x00;     //dwBaudRate    //9600 bps
    cmd_buf[4] = 0x00;     //dwBaudRate    //9600 bps

    cmd_buf[5] = 0x00;     //stop bit,     UART_ONE_STOP_BIT
    cmd_buf[6] = 0x00;     //parity type,  UART_PARITY_NONE
    cmd_buf[7] = 0x08;     //data bit,     UART_DATA_BIT_8
    cmd_buf[8] = 0xff;     //flow control, UART_DISABLE_FLOW_CONTROL

    result = ioctl(fd, HIDIOCSFEATURE(9), cmd_buf);

    memset(cmd_buf,0x00,sizeof(cmd_buf));
}

void hid_uart_comm::check_bgms()
{
    int result = 0;

    cmd_buf[0] = 0x01;
    cmd_buf[1] = 0x80;     //BGMS echo command

    result = write(fd, cmd_buf, 0x2);

    resp_timer->start(response_time_msec);

    Log();
}

void hid_uart_comm::ReadyRead()
{
    read(fd, resp_buf, 64);

    m_tranferHost.append(resp_buf[1]).toHex();

    resp_timer->start(response_time_msec);

    Log()<<QString::number(resp_buf[1], 16);
}

hid_uart_comm::~hid_uart_comm()
{
    resp_bgms_comm->m_comm_resp = sys_cmd_resp::RESP_COMM_PORT_CLOSE_SUCCESS;
    emit sig_bgms_comm_response(resp_bgms_comm);
}
