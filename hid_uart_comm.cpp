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

    comm_protocol = nullptr;

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

        connect(bgms_check_start, SIGNAL(timeout()), this, SLOT(check_bgms_protocol_type()));

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

               if(resp_bgms_comm->m_comm_status == sys_cmd_resp::CMD_COMM_BGMS_CHECK)
               {
                      if(m_tranferHost.count() >= 3)
                      {
                          if(m_tranferHost.at(1) == 0x10 && m_tranferHost.at(2) == 0x20)
                          {
                             resp_bgms_comm->m_comm_resp = sys_cmd_resp::RESP_COMM_BGMS_CHECK_SUCCESS;
                             resp_bgms_comm->m_protocol_type = sys_cmd_resp::VERSION_01;
                          }
                           else if(m_tranferHost.at(1) == 0x1e && m_tranferHost.at(2) == 0x2e)
                          {
                              Log();
                              resp_bgms_comm->m_comm_resp = sys_cmd_resp::RESP_COMM_BGMS_CHECK_SUCCESS;
                              resp_bgms_comm->m_protocol_type = sys_cmd_resp::VERSION_03;
                              comm_protocol = new SerialProtocol3;

                          }
                          else if(m_tranferHost.at(1) == 0x1f && m_tranferHost.at(2) == 0x2f)
                          {
                              resp_bgms_comm->m_comm_resp = sys_cmd_resp::RESP_COMM_BGMS_CHECK_SUCCESS;
                              resp_bgms_comm->m_protocol_type = sys_cmd_resp::VERSION_02;
                          }
                          else
                          {
                              resp_bgms_comm->m_comm_resp = sys_cmd_resp::RESP_COMM_BGMS_CHECK_FAIL;
                              resp_bgms_comm->m_protocol_type = sys_cmd_resp::VERSION_UNKNOWN;
                          }
                      }
                      else
                      {
                          resp_bgms_comm->m_comm_resp = sys_cmd_resp::RESP_COMM_BGMS_CHECK_FAIL;
                          resp_bgms_comm->m_protocol_type = sys_cmd_resp::VERSION_UNKNOWN;
                      }
              }
              else
              {
                   //parsing the response packet and post processing
                  if(resp_bgms_comm->m_comm_status == sys_cmd_resp::CMD_COMM_GET_TIME)
                  {
                        comm_protocol->processPacket(m_tranferHost);
                        resp_bgms_comm->current_bgms_time = comm_protocol->m_CurrentbgmsTimeDate;
                        resp_bgms_comm->m_comm_resp = sys_cmd_resp::RESP_COMM_GET_TIME_SUCCESS;
                  }
                  else if(resp_bgms_comm->m_comm_status == sys_cmd_resp::CMD_COMM_READ_SERIAL)
                  {
                        comm_protocol->processPacket(m_tranferHost);
                        resp_bgms_comm->serial_number = comm_protocol->m_serialnumber;
                        resp_bgms_comm->m_comm_resp = sys_cmd_resp::RESP_COMM_READ_SERIAL_SUCCESS;
                  }
                  else if(resp_bgms_comm->m_comm_status == sys_cmd_resp::CMD_COMM_GET_STORED_VALUE_COUNT)
                  {
                       comm_protocol->processPacket(m_tranferHost);
                       resp_bgms_comm->measured_result = comm_protocol->m_bgms_stored_result;
                       resp_bgms_comm->m_comm_resp = sys_cmd_resp::RESP_COMM_GET_STORED_VALUE_COUNT_SUCCESS;
                  }
                  else if(resp_bgms_comm->m_comm_status == sys_cmd_resp::CMD_COMM_MEM_DELETE)
                  {
                        resp_bgms_comm->m_comm_resp = sys_cmd_resp::RESP_COMM_MEM_DELETE_SUCCESS;
                  }
                  else if(resp_bgms_comm->m_comm_status == sys_cmd_resp::CMD_COMM_DOWNLOAD)                     //BGMS Data Download function
                  {
                      Log()<<QVariant::fromValue(resp_bgms_comm->m_bgms_data_download_cmd).toString();

                      if(resp_bgms_comm->m_bgms_data_download_cmd == sys_cmd_resp::DOWNLOAD_READ_SERIAL)
                      {
                            comm_protocol->processPacket(m_tranferHost);

                            /*first check the BGMS serial number, next check the downloadable count*/
                            resp_bgms_comm->m_comm_status = sys_cmd_resp::CMD_COMM_DOWNLOAD;
                            resp_bgms_comm->m_bgms_data_download_cmd = sys_cmd_resp::DOWNLOAD_READ_COUNT;
                            cmd_buf_tmp = comm_protocol->requestCommand(Sp::CurrentIndexOfGluecose);
                            Log();
                      }
                      else if(resp_bgms_comm->m_bgms_data_download_cmd == sys_cmd_resp::DOWNLOAD_READ_COUNT)
                      {
                            resp_bgms_comm->m_comm_status = sys_cmd_resp::CMD_COMM_DOWNLOAD;
                            resp_bgms_comm->m_bgms_data_download_cmd = sys_cmd_resp::DOWLOAD_CONTINUE;
                            cmd_buf_tmp = comm_protocol->processPacket(m_tranferHost);
                            Log();

                      }
                      else if(resp_bgms_comm->m_bgms_data_download_cmd == sys_cmd_resp::DOWLOAD_CONTINUE)
                      {
                          if(!comm_protocol->m_download_complete)
                          {
                              resp_bgms_comm->m_comm_status = sys_cmd_resp::CMD_COMM_DOWNLOAD;
                              resp_bgms_comm->m_bgms_data_download_cmd = sys_cmd_resp::DOWLOAD_CONTINUE;
                              cmd_buf_tmp = comm_protocol->processPacket(m_tranferHost);
                              Log();
                          }
                      }
                      else if(resp_bgms_comm->m_bgms_data_download_cmd == sys_cmd_resp::DOWNLOAD_DONE)
                      {

                      }

                       resp_bgms_comm->m_comm_resp = sys_cmd_resp::RESP_COMM_DOWNLOAD_SUCCESS;

                       m_tranferHost.clear();
                       make_hid_cmd_packet(static_cast<quint8>(cmd_buf_tmp.size()));
                  }
              }

              sig_to_host:
                  m_tranferHost.clear();
                  emit sig_bgms_comm_response(resp_bgms_comm);
        });
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

void hid_uart_comm::check_bgms_protocol_type()
{
    int result = 0;

    cmd_buf[0] = 0x01;
    cmd_buf[1] = 0x80;     //BGMS echo command

    result = write(fd, cmd_buf, 0x2);

    resp_timer->start(response_time_msec);  
}

void hid_uart_comm::ReadyRead()
{
    read(fd, resp_buf, 64);

    m_tranferHost.append(resp_buf[1]).toHex();

    resp_timer->start(response_time_msec);

//  Log()<<QString::number(resp_buf[1], 16);
}

/*
void hid_uart_comm::Error()
{

}
*/

int hid_uart_comm::make_hid_cmd_packet(quint8 packet_size)
{
    int result = 0;

    cmd_buf[0] =  packet_size;

    for(quint8 cmd_size=0; cmd_size<cmd_buf_tmp.size()+1; cmd_size++)
        cmd_buf[cmd_size+1] = cmd_buf_tmp[cmd_size];

    result = write(fd, cmd_buf, cmd_buf[0]+1);

    resp_timer->start(response_time_msec);

    return result;
}

void hid_uart_comm::cmd_from_host(sys_cmd_resp *cmd)
{

    Log()<<QVariant::fromValue(cmd->m_comm_cmd).toString();

    switch (cmd->m_comm_cmd)
    {
        case sys_cmd_resp::CMD_COMM_OPEN:
        break;

        case sys_cmd_resp::CMD_COMM_CLOSE:
        break;

        case sys_cmd_resp::CMD_COMM_BGMS_CHECK:

                resp_bgms_comm->m_comm_status = sys_cmd_resp::CMD_COMM_BGMS_CHECK;
                check_bgms_protocol_type();

        break;

        case sys_cmd_resp::CMD_COMM_GET_TIME:
            {
                resp_bgms_comm->m_comm_status = sys_cmd_resp::CMD_COMM_GET_TIME;
                cmd_buf_tmp = comm_protocol->requestCommand(Sp::ReadTimeInformation);
                make_hid_cmd_packet(static_cast<quint8>(cmd_buf_tmp.size()));
            }

        break;

        case sys_cmd_resp::CMD_COMM_READ_SERIAL:
            {                
                resp_bgms_comm->m_comm_status = sys_cmd_resp::CMD_COMM_READ_SERIAL;
                cmd_buf_tmp = comm_protocol->requestCommand(Sp::ReadSerialNumber);
                make_hid_cmd_packet(static_cast<quint8>(cmd_buf_tmp.size()));
            }
        break;

        case sys_cmd_resp::CMD_COMM_SET_TIME:
            {                
                resp_bgms_comm->m_comm_status = sys_cmd_resp::CMD_COMM_SET_TIME;
                cmd_buf_tmp = comm_protocol->requestCommand(Sp::WriteTimeInformation);
                make_hid_cmd_packet(static_cast<quint8>(cmd_buf_tmp.size()));
            }
        break;

        case sys_cmd_resp::CMD_COMM_GET_STORED_VALUE_COUNT:                
            {
                resp_bgms_comm->m_comm_status = sys_cmd_resp::CMD_COMM_GET_STORED_VALUE_COUNT;                
                cmd_buf_tmp = comm_protocol->requestCommand(Sp::CurrentIndexOfGluecose);
                make_hid_cmd_packet(static_cast<quint8>(cmd_buf_tmp.size()));
            }
        break;

        case sys_cmd_resp::CMD_COMM_MEM_DELETE:
            {                
                resp_bgms_comm->m_comm_status = sys_cmd_resp::CMD_COMM_MEM_DELETE;
                cmd_buf_tmp = comm_protocol->requestCommand(Sp::DeleteData);
                make_hid_cmd_packet(static_cast<quint8>(cmd_buf_tmp.size()));
            }
        break;

        case sys_cmd_resp::CMD_COMM_DOWNLOAD:
            {
                resp_bgms_comm->m_comm_status = sys_cmd_resp::CMD_COMM_DOWNLOAD;
                resp_bgms_comm->m_bgms_data_download_cmd = sys_cmd_resp::DOWNLOAD_READ_SERIAL;
                cmd_buf_tmp = comm_protocol->requestCommand(Sp::ReadSerialNumber);
                make_hid_cmd_packet(static_cast<quint8>(cmd_buf_tmp.size()));
            }
        break;

        case sys_cmd_resp::CMD_COMM_UNKNOWN:
        break;
    }
}

hid_uart_comm::~hid_uart_comm()
{
    resp_bgms_comm->m_comm_resp = sys_cmd_resp::RESP_COMM_PORT_CLOSE_SUCCESS;
    emit sig_bgms_comm_response(resp_bgms_comm);
}
