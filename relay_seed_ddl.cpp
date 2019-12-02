#include "relay_seed_ddl.h"
#include "wiringPiI2C.h"

relay_seed_ddl::relay_seed_ddl()
{
    fd_seed_ddl = wiringPiI2CSetup(DEVICE_ID_COMM);

    if(!fd_seed_ddl)
        exit(0);
}

relay_seed_ddl::~relay_seed_ddl()
{

}

void relay_seed_ddl::measure_port_reset()
{
    for(quint8 Channel=0x1; Channel<0x5; Channel++)
        wiringPiI2CWriteReg8(fd_seed_ddl, Channel, 0x00);
}

void relay_seed_ddl::measure_port_open()
{
   for(quint8 Channel=0x5; Channel>0x0; Channel--)
       wiringPiI2CWriteReg8(fd_seed_ddl, Channel, 0xff);
}

void relay_seed_ddl::measure_port_control(relay_seed_ddl::relay_channel Channel, quint8 OnOff)
{
       wiringPiI2CWriteReg8(fd_seed_ddl, Channel, OnOff);
}
