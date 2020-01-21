
#include "usb_hid_comm.h"

usb_hid_comm::usb_hid_comm()
{

}

usb_hid_comm::~usb_hid_comm()
{

}

qint8 usb_hid_comm::usb_hid_init(void)
{
    result = hid_init();
    return 0;
}

qint8 usb_hid_comm::usb_hid_device_open(quint16 vendor_id, quint16 product_id)
{

    return 0;
}

qint8 usb_hid_comm::usb_hid_data_transfer(quint8 hid_tx_size, quint8 hid_rx_size)
{

    return 0;
}

qint8 usb_hid_comm::usb_hid_close(void)
{

    return 0;
}
