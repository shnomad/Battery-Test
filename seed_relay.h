#ifndef SEED_RELAY_H
#define SEED_RELAY_H

#include <QObject>

#define DEVICE_ID_MEASURE	0x10		//measurement
#define DEVICE_ID_COMM		0x13		//USB connection

#define CH1_DETECT	0x01
#define CH2_WORK	0x02
#define CH3_THIRD	0x03
#define CH4_COMM	0x04

#define CH1_GND		0x01
#define CH2_VCC		0x02
#define CH3_DP		0x03
#define CH4_DM		0x04

#define CH_ON	0xFF
#define CH_OFF	0x00

class seed_relay : public QObject
{
    Q_OBJECT

    public:
    quint32 fd_measure, fd_comm;
    quint16 result;

    explicit seed_relay();
            ~seed_relay();
public slots:
    void port_reset(quint32 fd);
    void work(quint32 fd, quint8 Channel, quint8 OnOff);

};

#endif // SEED_RELAY_H
