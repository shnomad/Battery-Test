#include "hid_uart_comm.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>
#include "commondefinition.h"

hid_uart_comm::hid_uart_comm(QObject *parent) : QObject(parent)
{
    QString hidpath = "/dev/uart-hid-1";

    fd = open(hidpath.toLocal8Bit(), O_RDWR);

    if(fd <0)
    {
        exit(0);
    }

    uart_init();

    comm_start = new QTimer(this);
    comm_start->setSingleShot(true);

    connect(comm_start, SIGNAL(timeout()), this, SLOT(Send_Command()));

    m_notify_hid = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    m_notify_error = new QSocketNotifier(fd, QSocketNotifier::Exception, this);

    connect(m_notify_hid, SIGNAL(activated(int)), this, SLOT(ReadyRead()));
    connect(m_notify_error, SIGNAL(activated(int)), this, SLOT(Error()));

    m_notify_hid->setEnabled(true);
    m_notify_error->setEnabled(true);


    resp_timer = new QTimer(this);
    resp_timer->setSingleShot(true);

    connect(resp_timer,  &QTimer::timeout, [=]()
    {
        Log()<<QString::fromUtf8(m_tranferHost.toHex());
    });

    comm_start->start(200);
}

void hid_uart_comm::uart_init()
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

void hid_uart_comm::Send_Command()
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

}
