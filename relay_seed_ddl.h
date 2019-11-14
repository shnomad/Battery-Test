#ifndef RELAY_SEED_DDL_H
#define RELAY_SEED_DDL_H

#include <QObject>
#define DEVICE_ID_COMM		0x10		//USB connection

#define CH1_GND		0x01
#define CH2_VCC		0x02
#define CH3_DP		0x03
#define CH4_DM		0x04

#define DDL_CH_ON  0xff
#define DDL_CH_OFF  0x00

class relay_seed_ddl : public QObject
{
    Q_OBJECT

public:
    quint32 fd_comm;
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
    void comm_port_reset();
    void comm_port_open();
    void comm_port_control(relay_channel, quint8 OnOff);
};

#endif // RELAY_SEED_DDL_H
