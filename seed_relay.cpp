
#include "seed_relay.h"
#include "wiringPiI2C.h"

seed_relay::seed_relay()
{
    fd = wiringPiI2CSetup(DEVICE_ID);

    if(!fd)
        exit(0);
}

seed_relay::~seed_relay()
{

}

void seed_relay::port_reset(void)
{
    for(quint8 Channel=0; Channel<0x4; Channel++)
    {
        wiringPiI2CWriteReg8(fd, Channel, CH_OFF);
    }
}

void seed_relay::work(quint8 Channel, quint8 OnOff)
{
    result = wiringPiI2CWriteReg8(fd, Channel, OnOff);
}
