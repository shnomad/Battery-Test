#ifndef SEED_RELAY_H
#define SEED_RELAY_H

#include <QObject>

#define DEVICE_ID	0x10
#define CH1_DETECT	0x01
#define CH2_WORK	0x02
#define CH3_THIRD	0x03
#define CH4_COMM	0x04

#define CH_ON	0xFF
#define CH_OFF	0x00

class seed_relay : public QObject
{
    Q_OBJECT

    quint32 fd;
    quint16 result;

    public:

    explicit seed_relay();
            ~seed_relay();
public slots:
    void port_reset(void);
    void work(quint8 Channel, quint8 OnOff);

};

#endif // SEED_RELAY_H
