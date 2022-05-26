#ifndef RELAY_SEED_DDL_H
#define RELAY_SEED_DDL_H

#include <QObject>
#include "gpiocontrol.h"

class relay_seed_ddl : public GpioControl
{
    Q_OBJECT

public:
    quint32 fd_seed_ddl;
    quint16 result;

    enum relay_number{
         NO_1 = 17,
         NO_2 = 18,
         NO_3 = 22,
         NO_4 = 23,
         NO_5 = 9,
         NO_6 = 25,
         NO_7 = 6,
         NO_8 = 12,
         NO_9 = 26,
         NO_10 =20
    };

    enum jig_channel {
        CH_1 =0x1,
        CH_2,
        CH_3,
        CH_4,
        CH_5
    };

    enum sensor_port {
        DETECT = 0x0,
        WORK_THIRD
    };

explicit relay_seed_ddl(quint8, QObject *parent = nullptr);
            ~relay_seed_ddl();

public slots:
    void port_reset(quint8);
    void port_open(jig_channel);
    void port_control(quint8, sensor_port, GpioControl::SET_VAL);

private:

};

#endif // RELAY_SEED_DDL_H
