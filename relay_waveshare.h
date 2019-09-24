#ifndef RELAY_WAVESHARE_H
#define RELAY_WAVESHARE_H

#include <QObject>

#define CH_ON	0x0
#define CH_OFF	0x1

class relay_waveshare : public QObject
{
    Q_OBJECT

    public:
    quint16 result;
    explicit relay_waveshare();
            ~relay_waveshare();

    enum relay_channel{
        DETECT = 26,
        WORK = 20,
        THIRD =21
    };

public slots:
    void measure_port_reset();
    void measure_work(relay_channel, quint8 OnOff);

};

#endif // RELAY_WAVESHARE_H
