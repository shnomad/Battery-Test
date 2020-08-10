#ifndef RELAY_SEED_DDL_H
#define RELAY_SEED_DDL_H

#include <QObject>
#define DEVICE_ID_I		0x10
#define DEVICE_ID_II    0x12

#define DDL_CH_ON   0xff
#define DDL_CH_OFF  0x00

class relay_seed_ddl : public QObject
{
    Q_OBJECT

public:
    quint32 fd_seed_ddl;
    quint16 result;

    explicit relay_seed_ddl();
            ~relay_seed_ddl();

    enum relay_channel{
        CH_1 = 1,
        CH_2 = 2,
        CH_3 =3,
        CH_4 =4
    };

public slots:
    void measure_port_reset();
    void measure_port_open();
    void measure_port_control(relay_channel, quint8 OnOff);
};

#endif // RELAY_SEED_DDL_H
