#ifndef UDEV_MONITOR_USB_H
#define UDEV_MONITOR_USB_H

#include <QObject>
#include <QThread>
#include <stdio.h>
#include <unistd.h>
#include <libudev.h>
#include <QTimer>
#include "sys_cmd_resp.h"

#define SUBSYSTEM "usb"

class udev_monitor_usb : public QThread
{
    Q_OBJECT
public:
    explicit udev_monitor_usb(QObject *parent = nullptr);
        ~udev_monitor_usb() override;

    void print_device(struct udev_device* dev);
    void process_device(struct udev_device* dev);
    void enumerate_devices(struct udev* udev);    

    struct udev_device* get_child(struct udev* udev, struct udev_device* parent, const char* subsystem);
    void enumerate_usb_mass_storage(struct udev* udev);

    inline void delay(int millisecondsWait);

signals:
    void sig_usb_mass_update();
    void sig_resp_to_main(sys_cmd_resp *);

public slots:
    void update();

private:
    struct udev_monitor *mon;
//  QMap <QString, QString>device_info;
    sys_cmd_resp *resp_usb_monitor;

protected:
    void run() override;

};

#endif // UDEV_MONITOR_USB_H
