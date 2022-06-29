#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QObject>
#include "measurement_param.h"
#include "python_wrapper.h"

class relay_seed_ddl;

class measurement : public QObject
{
    Q_OBJECT

public:
    explicit measurement(quint8,QObject *parent = nullptr);
        ~measurement();

    void dmm_operation();

    signals:
    void update_action(QString);
    void update_test_count(int);
    void update_interval_time(int);
    void update_dmm_status(QString);
    void set_start();
    void set_detect();
    void set_work_on();
    void set_detect_off();
    void set_test_count_check();
    void set_detect_off_interval();

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
//  void interva_time_check();
    void meter_working_status();

    private:

    //QTimer *detect_on_timer, *work_on_timer, *third_on_timer, *detect_off_timer, *count_check_timer, *test_interval_timer, *interval_count_display_timer, *meter_working_status_timer;
    QTimer *meter_working_status_timer;
    relay_seed_ddl *measure_relay;
    measurement_param m_test_param{}, m_test_param_tmp{}, m_test_param_delay{};
    quint8 Channel=0;
    quint32 target_test_count=0, current_test_count=0, target_test_count_rest=0, meter_mem_capacity=0, interval_timer_count=0, interval_timer_count_tmp=0;
    measurement_param::meter_status m_measure_status;
//    volatile unsigned int start_to_detect_on_delay=0, detect_to_work_on_delay=0, work_on_to_third_on_delay=0, third_on_to_detect_off_delay=0, detect_off_to_test_count_delay=0, test_count_to_interval_delay=0;
};

#endif // MEASUREMENT_H
