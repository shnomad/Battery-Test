#include "relay_seed.h"
#include "wiringPiI2C.h"

relay_seed::relay_seed()
{
    fd_measure = wiringPiI2CSetup(DEVICE_ID_MEASURE);

    if(!fd_measure)
        exit(0);
}

relay_seed::~relay_seed()
{

}

void relay_seed::measure_port_reset()
{
    wiringPiI2CWriteReg8(fd_measure, REG_MODE, REG_DATA);
}

void relay_seed::measure_work(relay_seed::relay_channel channel, quint8 OnOff)
{

    if(OnOff)
    {
        reg_data &= ~(0x1 << channel);
    }
    else
    {
        reg_data |= (0x1 << channel);
    }

    wiringPiI2CWriteReg8(fd_measure, REG_MODE, reg_data);
}
