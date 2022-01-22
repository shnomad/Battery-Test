#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QObject>
#include <QDebug>
#include "measurement_param.h"

class relay_seed_ddl;

class measurement : public QObject
{
    Q_OBJECT

public:
    explicit measurement(quint8,QObject *parent = nullptr);
        ~measurement();

    signals:

    public slots:

    void setup();    
    void start();
    void stop();
    void pause();
    void detect_on();
    void work_on();
    void third_on();
    void detect_off();
    void test_count_check();

    private:

    QTimer *detect_on_timer, *work_on_timer, *third_on_timer, *detect_off_timer, *test_interval_timer;
    relay_seed_ddl *measure_relay;
    measurement_param m_test_param;
    quint8 Channel=0;
};

#endif // MEASUREMENT_H
