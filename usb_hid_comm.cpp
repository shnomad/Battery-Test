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

qint8 usb_comm::usb_hid_data_transfer(quint8 hid_tx_size, quint8 hid_rx_size)
{

    return 0;
}

qint8 usb_comm::usb_hid_close(void)
{

    return 0;
}
