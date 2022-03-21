#ifndef RELAY_SEED_DDL_H
#define RELAY_SEED_DDL_H

#include <QObject>
#define DEVICE_ID_CH_1   0x10
#define DEVICE_ID_CH_2   0x20

#define DDL_CH_ON   0xff
#define DDL_CH_OFF  0x00

class relay_seed_ddl : public QObject
{
    Q_OBJECT

public:
    quint32 fd_seed_ddl;
    quint16 result;

    explicit relay_seed_ddl(quint8, QObject *parent = nullptr);
            ~relay_seed_ddl();

    enum relay_channel{
        CH_1 = 1,
        CH_2 = 2,
        CH_3 =3,
        CH_4 =4
    };

public slots:
    void port_reset(quint8);
    void port_open(quint8);
    void port_control(quint8,relay_channel, quint8 OnOff);

private:

    int file_i2c;
    int length;
    unsigned char buffer[60] = {0};
    char *filename = (char*)"/dev/i2c-1";
    int device_id;
};

#endif // RELAY_SEED_DDL_H
