#include <QEventLoop>
#include "udev_monitor_usb.h"
#include "commondefinition.h"

udev_monitor_usb::udev_monitor_usb(QObject *parent) : QThread(parent)
{
     resp_usb_monitor = new sys_cmd_resp;
     connect(this, SIGNAL(sig_usb_mass_update()), this, SLOT(update()));
}

void udev_monitor_usb::update()
{

    struct udev *udev = udev_new();

      if (!udev)
      {
           exit(0);
      }

    enumerate_usb_mass_storage(udev);
    udev_unref(udev);
}

void udev_monitor_usb::print_device(struct udev_device* dev)
{   

       const char* action = udev_device_get_action(dev);
       if (! action)
           action = "exists";

       const char* vendorid = udev_device_get_sysattr_value(dev, "idVendor");
       if (! vendorid)
           vendorid = "0000";

       const char* productid = udev_device_get_sysattr_value(dev, "idProduct");
       if (! productid)
           productid = "0000";

       const char* manufacturer = udev_device_get_sysattr_value(dev, "manufacturer");
       if (! manufacturer)
           manufacturer = "unknown";

       Log()<<"udev_device_get_subsystem :"<<udev_device_get_subsystem(dev);
       Log()<<"udev_device_get_devtype :"<<udev_device_get_devtype(dev);
       Log()<<"action :" <<action;
       Log()<<"vendor :" <<vendorid;
       Log()<<"product :" <<productid;
       Log()<<"manufacturer :"<<manufacturer;

       /*check USB Mass Storage*/
       if(QString(action) == "bind" || QString(action) == "remove")
       {
           Log()<<action;
           delay(3000);

           emit sig_usb_mass_update();
       }
}

void udev_monitor_usb::process_device(struct udev_device* dev)
{
    if (dev)
    {
      if (udev_device_get_devnode(dev))
      {
          Log();
          print_device(dev);
      }
          udev_device_unref(dev);
    }
}

void udev_monitor_usb::enumerate_devices(struct udev* udev)
{
    struct udev_enumerate* enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_subsystem(enumerate, SUBSYSTEM);
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry* entry;

    udev_list_entry_foreach(entry, devices)
    {
        const char* path = udev_list_entry_get_name(entry);
        struct udev_device* dev = udev_device_new_from_syspath(udev, path);
        process_device(dev);
    }

    udev_enumerate_unref(enumerate);
}

struct udev_device* udev_monitor_usb::get_child(struct udev* udev, struct udev_device* parent, const char* subsystem)
{
    struct udev_device* child = NULL;
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_parent(enumerate, parent);
    udev_enumerate_add_match_subsystem(enumerate, subsystem);
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *entry;

    udev_list_entry_foreach(entry, devices)
    {
        const char *path = udev_list_entry_get_name(entry);
        child = udev_device_new_from_syspath(udev, path);
        break;
    }

    udev_enumerate_unref(enumerate);
    return child;
}

void udev_monitor_usb::enumerate_usb_mass_storage(struct udev* udev)
{

    struct udev_enumerate* enumerate = udev_enumerate_new(udev);

    udev_enumerate_add_match_subsystem(enumerate, "scsi");
    udev_enumerate_add_match_property(enumerate, "DEVTYPE", "scsi_device");
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *entry;

//    device_info.clear();
//
//   Log()<<"device_info :" <<device_info;

    resp_usb_monitor->mass_storage_device_info.clear();

    udev_list_entry_foreach(entry, devices)
    {
        const char* path = udev_list_entry_get_name(entry);
        struct udev_device* scsi = udev_device_new_from_syspath(udev, path);

        struct udev_device* block = get_child(udev, scsi, "block");
        struct udev_device* scsi_disk = get_child(udev, scsi, "scsi_disk");

        struct udev_device* usb = udev_device_get_parent_with_subsystem_devtype(scsi, "usb", "usb_device");

        if (block && scsi_disk && usb)
        {
         // usb_device_location << udev_device_get_devnode(block);

            resp_usb_monitor->mass_storage_device_info[udev_device_get_devnode(block)] = udev_device_get_sysattr_value(scsi, "vendor");
         // device_info[udev_device_get_devnode(block)] = udev_device_get_sysattr_value(scsi, "vendor");

            Log()<<"path :" << udev_device_get_devnode(block);
            Log()<<"VID :" << udev_device_get_sysattr_value(usb, "idVendor");
            Log()<<"PID :" << udev_device_get_sysattr_value(usb, "idProduct");
            Log()<<"Manufacturer :" << udev_device_get_sysattr_value(scsi, "vendor");
        }

        if (block)
        {
            udev_device_unref(block);
        }

        if (scsi_disk)
        {
            udev_device_unref(scsi_disk);
        }

        udev_device_unref(scsi);
    }

//  Log()<<"device_info :" <<device_info;

    resp_usb_monitor->m_comm_resp = sys_cmd_resp::RESP_USB_MASS_STORAGE_LIST;
    emit sig_resp_to_main(resp_usb_monitor);

    udev_enumerate_unref(enumerate);
}

void udev_monitor_usb::run()
{

    struct udev *udev = udev_new();

      if (!udev)
      {
           exit(0);
      }

    struct udev_monitor* mon = udev_monitor_new_from_netlink(udev, "udev");

    udev_monitor_filter_add_match_subsystem_devtype(mon, SUBSYSTEM, NULL);
    udev_monitor_enable_receiving(mon);

    int fd = udev_monitor_get_fd(mon);

    udev_unref(udev);

    forever
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        int ret = select(fd+1, &fds, NULL, NULL, NULL);

        if (ret <= 0)
            break;

        if (FD_ISSET(fd, &fds))
        {            
              struct udev_device* dev = udev_monitor_receive_device(mon);
              process_device(dev);
        }

        if ( QThread::currentThread()->isInterruptionRequested() )
        {
                   qDebug() << Q_FUNC_INFO << " terminated";
                   return;
        }
    }
}

inline void udev_monitor_usb::delay(int millisecondsWait)
{
    QEventLoop loop;
    QTimer t;
    t.connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
    t.start(millisecondsWait);
    loop.exec();
}

udev_monitor_usb::~udev_monitor_usb()
{

}
