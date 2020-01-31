#ifndef USB_HID_COMM_H
#define USB_HID_COMM_H

#include <QObject>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "hidapi.h"

#define STM32HID_VID 0x483
#define STM32HID_PID 0xa18b

#define CP2110_VID	0x10c4
#define CP2110_PID	0x8a32

#define HID_SEND	0x1

#define MAX_STR 255

class usb_comm : public QObject
{
    Q_OBJECT

public:

    qint32 result;
    hid_device *handle;
    wchar_t manufacture[MAX_STR];
    struct hid_device_info *devs, *cur_dev;
    quint16 vid, pid;

    explicit usb_comm();
            ~usb_comm();

    enum ERROR_CODE{
        USB_HID_WORK_SUCCESS =0,
        USB_HID_INIT_FAIL =-1,
        USB_HID_OPEN_FAIL = -2,
        USB_HID_DATA_TRANSFER_FAIL =-3,
        USB_HID_ECHO_FAIL = -4,
        USB_HID_RELEASE_FAIL =-5
    };

    enum COMM_TYPE{
        STM32 =0,
        CP2110,
        FT230X
    };

public slots:

    qint8 usb_hid_init(void);
    qint8 usb_hid_device_open(COMM_TYPE);
    qint8 usb_hid_get_manufacturer(hid_device *handle, wchar_t *str);
    void usb_hid_enumerate(void);
    void usb_hid_free_enumerate(void);
    qint8 usb_hid_get_product(hid_device *handle, wchar_t *str);
    qint8 usb_hid_data_transfer(quint8 hid_tx_size, quint8 hid_rx_size);
    qint8 usb_hid_close(void);
};

#endif // USB_HID_COMM_H
