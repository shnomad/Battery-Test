#ifndef BGM_COMM_PROTOCOL_H
#define BGM_COMM_PROTOCOL_H

#include <QObject>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#define STX		0x02
#define ETX		0x03

#define ECHO_REQUSET_SIZE								1
#define SERIAL_REQUSET_SIZE								13
#define TIME_SET_REQUSET_SIZE							19
#define MEASUREMENT_RESULT_INDEX_REQUSET_SIZE			13
#define MEASUREMENT_RESULT_DATA_REQUSET_SIZE			16

#define ECHO_REQUSET_RESPONSE_SIZE						3
#define SERIAL_REQUSET_RESPONSE_SIZE					27
#define TIME_SET_REQUSET_RESPONSE_SIZE					19
#define MEASUREMENT_RESULT_INDEX_REQUSET_RESPONSE_SIZE	15
#define MEASUREMENT_RESULT_DATA_REQUSET_RESPONSE_SIZE   22

#define ECHO_REQUSET_RESPONSE_TIMEOUT						100
#define SERIAL_REQUSET_RESPONSE_TIMEOUT                     100
#define TIME_SET_REQUSET_RESPONSE_TIMEOUT					100
#define MEASUREMENT_RESULT_INDEX_REQUSET_RESPONSE_TIMEOUT	100
#define MEASUREMENT_RESULT_DATA_REQUSET_RESPONSE_TIMEOUT    300

class usb_comm;

class bgm_comm_protocol : public QObject
{
    Q_OBJECT

    public:

    quint8 cmd_buffer[32];
//    quint8 resp_buffer[];

    quint8 system_time[6];
    quint16 set_result_index=1 ,get_result_count =0;
    quint16 stored_result_count;
    quint32 resp_timeout, tx_length, rx_length;

    explicit bgm_comm_protocol();
            ~bgm_comm_protocol();

    enum bgm_command
    {
        ECHO_REQUSET =0x0,
        SERIAL_REQUSET,
        MEASUREMENT_RESULT_INDEX_REQUSET,
        MEASUREMENT_RESULT_DATA_REQUSET,
        MEASUREMENT_RESULT_DELETE_REQUEST
    };

    enum bgm_comm_error
    {
        COMM_SUCCESS = 0,
        COMM_TIME_OUT = -1,
        COMM_PACKET_HEADER_ERROR = -2,
        COMM_PACKET_SIZE_ERROR = -3,
        COMM_PACKET_CRC_ERROR = -4,
        COMM_PACKET_COMMAND_ERROR = -5
    };

    public slots:

      void cmd_send_read_response(bgm_command);
      void cmd_construct(bgm_comm_protocol::bgm_command cmd);
      bgm_comm_error response_parsing(void);
      uint16_t crc_calculate(quint8 *buffer, quint16 length);
      uint32_t get_system_time(void);

     private:

      usb_comm *usb_hid_comm;

};


#endif // BGM_COMM_PROTOCOL_H
