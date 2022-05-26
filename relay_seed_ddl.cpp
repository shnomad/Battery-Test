
#include "relay_seed_ddl.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

relay_seed_ddl::relay_seed_ddl(quint8 ch, QObject *parent) : GpioControl(parent)
{
    switch (ch)
    {
        case CH_1:

            GpioControl::exportGPIO(NO_1);
            GpioControl::exportGPIO(NO_2);

            GpioControl::setDirection(NO_1,SET_OUT);
            GpioControl::setDirection(NO_2,SET_OUT);

            GpioControl::setValue(NO_1, SET_LOW);
            GpioControl::setValue(NO_2, SET_LOW);

        break;

        case CH_2:

            GpioControl::exportGPIO(NO_3);
            GpioControl::exportGPIO(NO_4);

            GpioControl::setDirection(NO_3,SET_OUT);
            GpioControl::setDirection(NO_4,SET_OUT);

            GpioControl::setValue(NO_3, SET_LOW);
            GpioControl::setValue(NO_4, SET_LOW);

        break;

        case CH_3:

            GpioControl::exportGPIO(NO_5);
            GpioControl::exportGPIO(NO_6);

            GpioControl::setDirection(NO_5,SET_OUT);
            GpioControl::setDirection(NO_6,SET_OUT);

            GpioControl::setValue(NO_5, SET_LOW);
            GpioControl::setValue(NO_6, SET_LOW);

        break;

        case CH_4:

            GpioControl::exportGPIO(NO_7);
            GpioControl::exportGPIO(NO_8);

            GpioControl::setDirection(NO_7,SET_OUT);
            GpioControl::setDirection(NO_8,SET_OUT);

            GpioControl::setValue(NO_7, SET_LOW);
            GpioControl::setValue(NO_8, SET_LOW);

        break;

        case CH_5:

            GpioControl::exportGPIO(NO_9);
            GpioControl::exportGPIO(NO_10);

            GpioControl::setDirection(NO_9,SET_OUT);
            GpioControl::setDirection(NO_10,SET_OUT);

            GpioControl::setValue(NO_9, SET_LOW);
            GpioControl::setValue(NO_10, SET_LOW);

        break;
    }
}

relay_seed_ddl::~relay_seed_ddl()
{

}

void relay_seed_ddl::port_reset(quint8 ch)
{

    switch (ch)
    {
        case CH_1:

            GpioControl::unexportGPIO(NO_1);
            GpioControl::unexportGPIO(NO_2);

            GpioControl::exportGPIO(NO_1);
            GpioControl::exportGPIO(NO_2);

            GpioControl::setDirection(NO_1,SET_OUT);
            GpioControl::setDirection(NO_2,SET_OUT);

            GpioControl::setValue(NO_1, SET_LOW);
            GpioControl::setValue(NO_2, SET_LOW);

        break;

        case CH_2:

            GpioControl::unexportGPIO(NO_3);
            GpioControl::unexportGPIO(NO_4);

            GpioControl::exportGPIO(NO_3);
            GpioControl::exportGPIO(NO_4);

            GpioControl::setDirection(NO_3,SET_OUT);
            GpioControl::setDirection(NO_4,SET_OUT);

            GpioControl::setValue(NO_3, SET_LOW);
            GpioControl::setValue(NO_4, SET_LOW);

        break;

        case CH_3:

            GpioControl::unexportGPIO(NO_5);
            GpioControl::unexportGPIO(NO_6);

            GpioControl::exportGPIO(NO_5);
            GpioControl::exportGPIO(NO_6);

            GpioControl::setDirection(NO_5,SET_OUT);
            GpioControl::setDirection(NO_6,SET_OUT);

            GpioControl::setValue(NO_5, SET_LOW);
            GpioControl::setValue(NO_6, SET_LOW);

        break;

        case CH_4:

            GpioControl::unexportGPIO(NO_7);
            GpioControl::unexportGPIO(NO_8);

            GpioControl::exportGPIO(NO_7);
            GpioControl::exportGPIO(NO_8);

            GpioControl::setDirection(NO_7,SET_OUT);
            GpioControl::setDirection(NO_8,SET_OUT);

            GpioControl::setValue(NO_7, SET_LOW);
            GpioControl::setValue(NO_8, SET_LOW);

        break;

        case CH_5:

            GpioControl::unexportGPIO(NO_9);
            GpioControl::unexportGPIO(NO_10);

            GpioControl::exportGPIO(NO_9);
            GpioControl::exportGPIO(NO_10);

            GpioControl::setDirection(NO_9,SET_OUT);
            GpioControl::setDirection(NO_10,SET_OUT);

            GpioControl::setValue(NO_9, SET_LOW);
            GpioControl::setValue(NO_10, SET_LOW);

        break;
    }

}

void relay_seed_ddl::port_open(relay_seed_ddl::jig_channel ch)
{
//   for(quint8 Channel=0x5; Channel>0x0; Channel--)
//       wiringPiI2CWriteReg8(fd_seed_ddl, Channel, 0xff);
}

void relay_seed_ddl::port_control(quint8 ch, sensor_port port, GpioControl::SET_VAL OnOff)
{
    switch(ch)
    {
        case CH_1:

            if(port == DETECT)
            {
                GpioControl::setValue(NO_1, OnOff);
            }
            else
            {
                GpioControl::setValue(NO_2, OnOff);
            }

        break;

        case CH_2:

            if(port == DETECT)
            {
                GpioControl::setValue(NO_3, OnOff);
            }
            else
            {
                GpioControl::setValue(NO_4, OnOff);
            }

        break;

        case CH_3:

            if(port == DETECT)
            {
                GpioControl::setValue(NO_5, OnOff);
            }
            else
            {
                GpioControl::setValue(NO_6, OnOff);
            }

        break;

        case CH_4:

            if(port == DETECT)
            {
                GpioControl::setValue(NO_7, OnOff);
            }
            else
            {
                GpioControl::setValue(NO_8, OnOff);
            }

        break;

        case CH_5:

            if(port == DETECT)
            {
                GpioControl::setValue(NO_9, OnOff);
            }
            else
            {
                GpioControl::setValue(NO_10, OnOff);
            }

        break;
    }
}
