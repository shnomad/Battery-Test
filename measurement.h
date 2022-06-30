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
    void meter_working_status();

    /*U1272A operation*/
    bool dmm_init();
    void dmm_read();
    bool dmm_open();
    void dmm_close();

    /*create csv file*/
    bool init_csv();
    bool write_csv();

    private:

    QTimer *meter_working_status_timer;
    relay_seed_ddl *measure_relay;
    measurement_param m_test_param{}, m_test_param_tmp{}, m_test_param_delay{};
    quint8 Channel=0;
    quint32 target_test_count=0, current_test_count=0, target_test_count_rest=0, meter_mem_capacity=0, interval_timer_count=0, interval_timer_count_tmp=0;
    measurement_param::meter_status m_measure_status;

    /*Python API*/
    PyObject *pModule, *klass, *Instance, *pyResult, *pyValue;

};

#endif // MEASUREMENT_H
