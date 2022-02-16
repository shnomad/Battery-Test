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
    count_check_timer = new QTimer(this);

    detect_on_timer->setSingleShot(true);
    work_on_timer->setSingleShot(true);
    third_on_timer->setSingleShot(true);
    detect_off_timer->setSingleShot(true);
    test_interval_timer->setSingleShot(true);
    count_check_timer->setSingleShot(true);

#if 1
    connect(detect_on_timer, SIGNAL(timeout()),SLOT(detect_on()));
    connect(work_on_timer, SIGNAL(timeout()),SLOT(work_on()));
    connect(third_on_timer, SIGNAL(timeout()),SLOT(third_on()));
    connect(detect_off_timer, SIGNAL(timeout()),SLOT(detect_off()));
    connect(count_check_timer, SIGNAL(timeout()),SLOT(test_count_check()));
    connect(test_interval_timer, &QTimer::timeout,[=]()
        {
           //emit measure_cnt_check(SIGNAL_FROM_MEASURE_DETECT_OFF);
            emit start();
        });
#endif

}

void measurement::setup(measurement_param m_test_param)
{
    //Glucose : detect -> (3.5 sec) -> work/third on -> (7 sec) -> detect off -> (4 sec)
    Log() << Channel;

    Log() <<"meter type"<< m_test_param.type;
    Log() <<"test channel"<< m_test_param.channel;
    Log() <<"target_measure_count"<<m_test_param.target_measure_count;
    Log() <<"detect_on_time"<< m_test_param.detect_on_time;
    Log() <<"work_on_time"<< m_test_param.work_on_time;
    Log() <<"third_on_time"<< m_test_param.third_on_time;
    Log() <<"detect_off_time"<< m_test_param.detect_off_time;
    Log() <<"test_interval_time"<< m_test_param.test_interval_time;

     meter_mem_capacity = m_test_param.meter_memory_capacity;
     target_test_count=m_test_param.target_measure_count;
     detect_on_timer->setInterval(m_test_param.detect_on_time);
     work_on_timer->setInterval(m_test_param.work_on_time);
     third_on_timer->setInterval(m_test_param.third_on_time);
     detect_off_timer->setInterval(m_test_param.detect_off_time);
     count_check_timer->setInterval(0);

//   default teset interval                    //15 Sec
     test_interval_timer->setInterval(m_test_param.test_interval_time);
}

void measurement::start()
{
    Log() << Channel;

    detect_on_timer->start();

    emit update_test_count(current_test_count);
}

void measurement::stop()
{
    Log() << Channel ;

    detect_on_timer->stop();
    work_on_timer->stop();
    third_on_timer->stop();
    detect_off_timer->stop();
    count_check_timer->stop();   
    test_interval_timer->stop();

    /*relay port reset*/
    measure_relay->port_reset();

    emit update_test_count(current_test_count);

    current_test_count=0;
}

void measurement::pause()
{
    Log() << Channel;

    detect_on_timer->stop();
    work_on_timer->stop();
    third_on_timer->stop();
    detect_off_timer->stop();

    measure_relay->port_reset();
}

void measurement::detect_on()
{    
   Log() <<"meter : "<<Channel ;

   emit update_action("detect on");

   measure_relay->port_control(measure_relay->relay_channel::CH_1, DDL_CH_ON);

   work_on_timer->start();
}

void measurement::work_on()
{    
    Log() << Channel;

    emit update_action("work on");

    measure_relay->port_control(measure_relay->relay_channel::CH_2, DDL_CH_ON);

    third_on_timer->start();
}

void measurement::third_on()
{
    Log() <<"meter : "<< Channel;

    emit update_action("third on");

    detect_off_timer->start();
}

void measurement::detect_off()
{
    Log() <<"meter : "<< Channel;

    emit update_action("detect off");

    measure_relay->port_control(measure_relay->relay_channel::CH_1, DDL_CH_OFF);
    measure_relay->port_control(measure_relay->relay_channel::CH_2, DDL_CH_OFF);

    current_test_count++;

    count_check_timer->start();
}

void measurement::test_count_check()
{        
    Log() <<"meter : "<< Channel;
    Log() <<"Current test count is  "<< current_test_count;

    emit update_test_count(current_test_count);

    if(target_test_count<=1000)
     {
        Log() <<"Target test count is 1000 or less";

       //Target count is 1000                          //target count is under 1000
      if((current_test_count == meter_mem_capacity) || (current_test_count == target_test_count_rest))
      {
          emit update_action("stop");

          emit stop();
      }
      else if(current_test_count < target_test_count)
      {
          test_interval_timer->start();
      }
    }
    else    //over 1000
    {
        Log() <<"Target test count is over 1000";

      quint8 th_current = current_test_count/1000;
      quint8 hund_current = (current_test_count/100)%10;
      quint8 tens_current = (current_test_count/10)%10;
      quint8 unit_current = current_test_count%10;
      quint8 hund_target = (target_test_count/100)%10;
      quint8 tens_target = (target_test_count/10)%10;
      quint8 unit_target = target_test_count%10;

      Log() << "current Count:" << th_current <<hund_current<<tens_current<<unit_current;
      Log() << "target Count Rest:"  << hund_target<<tens_target<<unit_target;

      //target count is reached at 1000, 2000, 3000, 4000 .....                 //Targeet count reached at 100, 200, 300, ......
      if((th_current>=1 && !hund_current && !tens_current && !unit_current) || (target_test_count_rest<1000 && hund_current==hund_target && !tens_target && !unit_target))
      {
          //Log()<<"on_device_open_clicked";
          //emit on_device_open_clicked();
          current_test_count=0;

          emit update_action("stop");

          emit stop();
      }
      else
      {
          Log()<<"measure_start";
            // emit start();
          test_interval_timer->start();
      }

     }
}

measurement::~measurement()
{

}
