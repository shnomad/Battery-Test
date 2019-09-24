#include "relay_seed.h"
#include "wiringPiI2C.h"

relay_seed::relay_seed()
{
    fd_comm = wiringPiI2CSetup(DEVICE_ID_COMM);

    if(!fd_comm)
        exit(0);
}

relay_seed::~relay_seed()
{

}

void relay_seed::comm_port_reset()
{
    for(quint8 Channel=0x1; Channel<0x5; Channel++)
        wiringPiI2CWriteReg8(fd_comm, Channel, 0x00);
}

void relay_seed::comm_port_open()
{
   for(quint8 Channel=0x5; Channel>0x0; Channel--)
       wiringPiI2CWriteReg8(fd_comm, Channel, 0xff);
}
