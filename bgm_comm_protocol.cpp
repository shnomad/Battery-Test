#include "bgm_comm_protocol.h"
#include "usb_hid_comm.h"

bgm_comm_protocol::bgm_comm_protocol()
{

}


bgm_comm_protocol::~bgm_comm_protocol()
{

}

void bgm_comm_protocol::cmd_send_read_response(bgm_comm_protocol::bgm_command cmd)
{
    cmd_construct(cmd);

    memset((void*)resp_buffer, 0x0, sizeof(resp_buffer));

    usb_hid_comm->usb_hid_data_transfer(cmd_buffer, resp_buffer, tx_length, rx_length, resp_timeout);

    return;
}

void bgm_comm_protocol::cmd_construct(bgm_comm_protocol::bgm_command cmd)
{

    qint32 numBytes = 0;  /* Actual bytes transferred. */
    quint8 tr_size = 0;  //  hid packet size
    quint8 cmd_temp;

    quint16 hid_tx_size, hid_rx_size;

    switch (cmd)
    {
        case ECHO_REQUSET:

            cmd_buffer[0] = 0x80;

            hid_tx_size = ECHO_REQUSET_SIZE;
            hid_rx_size = ECHO_REQUSET_RESPONSE_SIZE;
            resp_timeout = ECHO_REQUSET_RESPONSE_TIMEOUT;
            tx_length = ECHO_REQUSET_SIZE + 1;
            rx_length = hid_rx_size;

            break;

        case SERIAL_REQUSET:

            break;

        case MEASUREMENT_RESULT_INDEX_REQUSET:

            break;

        case MEASUREMENT_RESULT_DATA_REQUSET:

            break;

        case MEASUREMENT_RESULT_DELETE_REQUEST:

            break;

        default:
            break;
    }

    for (uint8_t cmd_shift_cnt = hid_tx_size; cmd_shift_cnt > 0; cmd_shift_cnt--)
    {
        cmd_buffer[cmd_shift_cnt+1] = cmd_buffer[cmd_shift_cnt-1];
    }

    cmd_buffer[0] = 0x1;
    cmd_buffer[1] = hid_tx_size;
}

bgm_comm_protocol::bgm_comm_error bgm_comm_protocol::response_parsing(void)
{
    return COMM_SUCCESS;
}

uint16_t bgm_comm_protocol::crc_calculate(quint8 *buffer, quint16 length)
{

    return 0;
}

uint32_t bgm_comm_protocol::get_system_time(void)
{
    return 0;
}
