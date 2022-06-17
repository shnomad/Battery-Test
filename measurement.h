#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QObject>
#include "measurement_param.h"

class relay_seed_ddl;

class measurement : public QObject
{
    Q_OBJECT

public:
    explicit measurement(quint8,QObject *parent = nullptr);
        ~measurement();

    signals:
    void update_action(QString);
    void update_test_count(int);
    void update_interval_time(int);

    public slots:

    void setup(measurement_param);
    void start();
    void stop();
    void pause();
    void detect_on();
    void work_on();
    void third_on();
    void detect_off();
    void test_count_check();
    void interva_time_check();

    private:

    QTimer *detect_on_timer, *work_on_timer, *third_on_timer, *detect_off_timer, *count_check_timer, *test_interval_timer, *interval_count_display_timer;
    relay_seed_ddl *measure_relay;
    measurement_param m_test_param{};
    quint8 Channel=0;
    quint32 target_test_count=0, current_test_count=0, target_test_count_rest=0, meter_mem_capacity=0, interval_timer_count=0, interval_timer_count_tmp=0;
};

#endif // MEASUREMENT_H
