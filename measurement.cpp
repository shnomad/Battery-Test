#include "measurement.h"
#include "relay_seed_ddl.h"

measurement::measurement(quint8 ch, QObject *parent) : QObject(parent)
{
    qDebug()<< "measurement thread channel number : " <<ch;

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

    //Create Relay function
    measure_relay = new relay_seed_ddl(ch);
}

void measurement::run()
{

    //Glucose : detect -> (3.5 sec) -> work/third on -> (7 sec) -> detect off -> (4 sec)

//    detect_on_timer->setInterval();
//    work_on_timer->setInterval();
//    third_on_timer->setInterval();
//    detect_off_timer->setInterval();

//    default teset interval                    //15 Sec
//    test_interval_timer->setInterval();
}

void measurement::start()
{

    detect_on_timer->start();
}

void measurement::stop()
{
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
    detect_on_timer->stop();
    work_on_timer->stop();
    third_on_timer->stop();
    detect_off_timer->stop();
}

void measurement::detect_on()
{
    work_on_timer->start();
}

void measurement::work_on()
{
    third_on_timer->start();
}

void measurement::third_on()
{
    detect_off_timer->start();
}

void measurement::detect_off()
{
     test_interval_timer->start();
}

measurement::~measurement()
{

}
