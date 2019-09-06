
#include "seed_relay.h"
#include "wiringPiI2C.h"

seed_relay::seed_relay()
{
    fd_measure = wiringPiI2CSetup(DEVICE_ID_MEASURE);
    fd_comm = wiringPiI2CSetup(DEVICE_ID_COMM);

    if(!fd_measure || !fd_comm)
        exit(0);
}

seed_relay::~seed_relay()
{

}

void seed_relay::port_reset(quint32 fd)
{
    for(quint8 Channel=0; Channel<0x4; Channel++)
    {
        wiringPiI2CWriteReg8(fd, Channel, CH_OFF);
    }

}

void seed_relay::work(quint32 fd, quint8 Channel, quint8 OnOff)
{
    result = wiringPiI2CWriteReg8(fd, Channel, OnOff);
}
