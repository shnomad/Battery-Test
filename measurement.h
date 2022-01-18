#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <QThread>
#include "measurement_param.h"

class measurement : public QObject
{
    Q_OBJECT

public:
    explicit measurement(quint8, QObject *parent = nullptr);
        ~measurement();

    signals:

    public slots:

    private:
};

#endif // MEASUREMENT_H
