#include "measurement.h"
#include "relay_seed_ddl.h"
#include "commondefinition.h"

measurement::measurement(quint8 ch, QObject *parent) : QObject(parent)
{
    Log()<< "measurement thread channel number : " <<ch;

    Channel = ch;

    //Create Relay function
    measure_relay = new relay_seed_ddl(ch);
    measure_relay->port_reset();

    //Create QTimer for measurement
    detect_on_timer = new QTimer(this);
    work_on_timer = new QTimer(this);
    third_on_timer = new QTimer(this);
    detect_off_timer = new QTimer(this);
    test_interval_timer = new QTimer(this);

    detect_on_timer->setSingleShot(true);
    work_on_timer->setSingleShot(true);
    third_on_timer->setSingleShot(true);
    detect_off_timer->setSingleShot(true);
    test_interval_timer->setSingleShot(true);

    connect(detect_on_timer, SIGNAL(timeout()),SLOT(detect_on()));
    connect(work_on_timer, SIGNAL(timeout()),SLOT(work_on()));
    connect(third_on_timer, SIGNAL(timeout()),SLOT(third_on()));
    connect(detect_off_timer, SIGNAL(timeout()),SLOT(detect_off()));
    connect(test_interval_timer, &QTimer::timeout,[=]()
        {
           //emit measure_cnt_check(SIGNAL_FROM_MEASURE_DETECT_OFF);
        });
}

void measurement::setup()
{
    Log() << Channel;
    //Glucose : detect -> (3.5 sec) -> work/third on -> (7 sec) -> detect off -> (4 sec)

//   detect_on_timer->setInterval();
//   work_on_timer->setInterval();
//   third_on_timer->setInterval();
//   detect_off_timer->setInterval();

//   default teset interval                    //15 Sec
//   test_interval_timer->setInterval();

}

void measurement::start()
{
    Log() << Channel;

    detect_on_timer->start();
}

void measurement::stop()
{
    Log() << Channel ;

    detect_on_timer->stop();
    work_on_timer->stop();
    third_on_timer->stop();
    detect_off_timer->stop();

    disconnect(detect_on_timer, SIGNAL(timeout()),this,SLOT(detect_on()));
    disconnect(work_on_timer, SIGNAL(timeout()),this,SLOT(work_on()));
    disconnect(third_on_timer, SIGNAL(timeout()),this,SLOT(third_on()));
    disconnect(detect_off_timer, SIGNAL(timeout()),this,SLOT(detect_off()));
}

void measurement::pause()
{
    Log() << Channel;

    detect_on_timer->stop();
    work_on_timer->stop();
    third_on_timer->stop();
    detect_off_timer->stop();
}

void measurement::detect_on()
{    
    Log() << Channel ;

    work_on_timer->start();
}

void measurement::work_on()
{    
    Log() << Channel;

    third_on_timer->start();
}

void measurement::third_on()
{
    Log() << Channel;
    detect_off_timer->start();
}

void measurement::detect_off()
{
    Log() << Channel;

    test_interval_timer->start();
}

void measurement::test_count_check()
{
    Log() << Channel;
}

measurement::~measurement()
{

}
