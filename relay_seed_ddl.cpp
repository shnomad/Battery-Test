
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
        device_id = DEVICE_ID_CH_1;
    else
        device_id = DEVICE_ID_CH_2;

    if (ioctl(file_i2c, I2C_SLAVE, device_id) < 0)
    {
        exit(0);
    }
}

relay_seed_ddl::~relay_seed_ddl()
{
    close(file_i2c);
}

void relay_seed_ddl::port_reset(quint8 m_ch)
{
    length = 2;			//<<< Number of bytes to write

    if(m_ch == 0x1)
    {
        for (quint8 channel=0x1; channel<0x5; channel++)
        {
            buffer[0] = channel;
            buffer[1] = 0x00;

            write(file_i2c, buffer, length);
        }
    }
    else
    {
        buffer[0] = 0x06;
        buffer[1] = 0xff;

        /*relay all off*/
        buffer[1] |= (0xf <<0);
        write(file_i2c, buffer, length);
    }
}

void relay_seed_ddl::port_open(quint8 m_ch)
{
//   for(quint8 Channel=0x5; Channel>0x0; Channel--)
//       wiringPiI2CWriteReg8(fd_seed_ddl, Channel, 0xff);
}

void relay_seed_ddl::port_control(quint8 m_ch, relay_seed_ddl::relay_channel Channel, quint8 OnOff)
{
    if(m_ch == 0x1)
    {
        buffer[0] = Channel;
        buffer[1] = OnOff;
    }
    else
    {
        if(OnOff)
           buffer[1] &= ~(0x1 << (Channel-1));
        else
           buffer[1] |= (0xf << (Channel-1));
    }

    write(file_i2c, buffer, length);
}
