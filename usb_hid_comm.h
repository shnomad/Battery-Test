#ifndef USB_HID_COMM_H
#define USB_HID_COMM_H

#include <QObject>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "hidapi.h"

#define STM32HID_VID 0x0483
#define STM32HID_PID 0xa18b

#define CP2110_VID	0x10c4
#define CP2110_PID	0x8a32

#define HID_SEND	0x1

class usb_hid_comm : public QObject
{
    Q_OBJECT

public:

    qint32 result;
    hid_device *handle;

    explicit usb_hid_comm();
            ~usb_hid_comm();

    enum ERROR_CODE{
        USB_HID_WORK_SUCCESS =0,
        USB_HID_INIT_FAIL =-1,
        USB_HID_OPEN_FAIL = -2,
        USB_HID_DATA_TRANSFER_FAIL =-3,
        USB_HID_ECHO_FAIL = -4,
        USB_HID_RELEASE_FAIL =-5
    };

public slots:

    qint8 usb_hid_init(void);
    qint8 usb_hid_device_open(quint16 vendor_id, quint16 product_id);
    qint8 usb_hid_data_transfer(quint8 hid_tx_size, quint8 hid_rx_size);
    qint8 usb_hid_close(void);
};

#endif // USB_HID_COMM_H
