
#include "relay_seed_ddl.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

relay_seed_ddl::relay_seed_ddl(quint8 m_ch, QObject *parent) : QObject(parent)
{
    if((file_i2c = open(filename, O_RDWR)) < 0)
    {
        exit(0);
    }

    if(m_ch == 0x1)
        device_id = 0x10;
    else
        device_id = 0x13;

    if (ioctl(file_i2c, I2C_SLAVE, device_id) < 0)
    {
        exit(0);
    }
}

relay_seed_ddl::~relay_seed_ddl()
{
    close(file_i2c);
}

void relay_seed_ddl::port_reset()
{
    length = 2;			//<<< Number of bytes to write

    for (quint8 channel=0x1; channel<0x5; channel++)
    {
        buffer[0] = channel;
        buffer[1] = 0x00;

        write(file_i2c, buffer, length);
    }
}

void relay_seed_ddl::port_open()
{
//   for(quint8 Channel=0x5; Channel>0x0; Channel--)
//       wiringPiI2CWriteReg8(fd_seed_ddl, Channel, 0xff);
}

void relay_seed_ddl::port_control(relay_seed_ddl::relay_channel Channel, quint8 OnOff)
{
    buffer[0] = Channel;
    buffer[1] = OnOff;

    write(file_i2c, buffer, length);
}
