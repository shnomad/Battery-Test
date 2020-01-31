#include "usb_hid_comm.h"

usb_comm::usb_comm()
{

}

usb_comm::~usb_comm()
{
    hid_exit();
}

qint8 usb_comm::usb_hid_init(void)
{
     if(hid_init()!= USB_HID_WORK_SUCCESS)
        return USB_HID_INIT_FAIL;

     return USB_HID_WORK_SUCCESS;
}

qint8 usb_comm::usb_hid_device_open(usb_comm::COMM_TYPE device)
{

    switch (device)
    {
        case STM32 :

            vid =STM32HID_VID;
            pid=STM32HID_PID;

            break;

        case CP2110:

            vid =CP2110_VID;
            pid=CP2110_PID;

            break;

        case FT230X:

            break;
        default:
            break;
    }

    handle = hid_open(vid, pid, NULL);

    if(!handle)
        return USB_HID_OPEN_FAIL;

//    hid_set_nonblocking(handle, 1);

//     if(hid_read(handle, hid_buf, 17)< USB_HID_WORK_SUCCESS)
//         return USB_HID_OPEN_FAIL;

    return USB_HID_WORK_SUCCESS;
}

void usb_comm::usb_hid_enumerate(void)
{
    devs = hid_enumerate(0x0, 0x0);
}

void usb_comm::usb_hid_free_enumerate(void)
{
    hid_free_enumeration(devs);
}

qint8 usb_comm::usb_hid_get_manufacturer(hid_device *handle, wchar_t *str)
{
    hid_get_manufacturer_string(handle, str, MAX_STR);
    return 0;
}

qint8 usb_comm::usb_hid_get_product(hid_device *handle, wchar_t *str)
{

    return 0;
}

qint8 usb_comm::usb_hid_data_transfer(quint8 *cmd_buffer, quint8 *resp_buffer, quint8 tx_length, quint8 rx_length, quint32 timeout)
{
    hid_write(handle, cmd_buffer, tx_length);

    hid_read_timeout(handle, resp_buffer, rx_length, timeout);

    return 0;
}

void usb_comm::usb_hid_data_test(void)
{
    quint8 tmp_cmd_buf[3], tmp_rsp_buf[5];

    tmp_cmd_buf[0] = 0x01;  //
    tmp_cmd_buf[1] = 0x01;  //size
    tmp_cmd_buf[2] = 0x80;  //cmd

    hid_write(handle, tmp_cmd_buf, 0x3);

    hid_read_timeout(handle, tmp_rsp_buf, 0x5,500);

    return;
}

qint8 usb_comm::usb_hid_close(void)
{

    return 0;
}
