#ifndef RELAY_SEED_H
#define RELAY_SEED_H

#include <QObject>

#define DEVICE_ID_MEASURE		0x20		//measure Connection

#define REG_MODE        0x06
#define REG_DATA        0xff

#define CH1_DETECT		0x01
#define CH2_WORK		0x02
#define CH3_THIRD		0x03
#define CH4_NONE		0x04

#define Relay_On        0x1
#define Relay_Off       0x0

class relay_seed : public QObject
{
    Q_OBJECT

    public:

    quint16 result;
    quint32 fd_measure;
    quint8 reg_data=0xff;

    explicit relay_seed();
            ~relay_seed();

    enum relay_channel{
        DETECT = 0,
        WORK = 1,
        THIRD =2,
        NONE =3
    };

public slots:
    void measure_port_reset();
    void measure_work(relay_channel, quint8 OnOff);
};

#endif // RELAY_SEED_H
