#include "measurement.h"
#include "relay_seed_ddl.h"
#include "commondefinition.h"
#include "python_wrapper.h"

measurement::measurement(quint8 ch, QObject *parent) : QObject(parent)
{
    Log()<< "measurement thread channel number : " <<ch;

    Channel = ch;

    //Create Relay function
    measure_relay = new relay_seed_ddl(ch);

    //Create QTimer for measurement
    detect_on_timer = new QTimer(this);
    work_on_timer = new QTimer(this);
    third_on_timer = new QTimer(this);
    detect_off_timer = new QTimer(this);
    test_interval_timer = new QTimer(this);
    count_check_timer = new QTimer(this);
    interval_count_display_timer = new QTimer(this);

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
    connect(interval_count_display_timer, SIGNAL(timeout()),SLOT(interva_time_check()));
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
    Log() <<"Channel:"<<Channel;

    Log() <<"meter type"<< m_test_param.type;
    Log() <<"test channel"<< m_test_param.channel;
    Log() <<"target_measure_count"<<m_test_param.target_measure_count;
    Log() <<"detect_on_time"<< m_test_param.detect_on_time;
    Log() <<"work_on_time"<< m_test_param.work_on_time;
    Log() <<"third_on_time"<< m_test_param.third_on_time;
    Log() <<"detect_off_time"<< m_test_param.detect_off_time;
    Log() <<"test_interval_time"<< m_test_param.test_interval_time;
    Log() <<"use daq970a instrument"<<m_test_param.use_daq970;

     meter_mem_capacity = m_test_param.meter_memory_capacity;
     target_test_count=m_test_param.target_measure_count;
     detect_on_timer->setInterval(m_test_param.detect_on_time);
     work_on_timer->setInterval(m_test_param.work_on_time);
     third_on_timer->setInterval(m_test_param.third_on_time);
     detect_off_timer->setInterval(m_test_param.detect_off_time);
     count_check_timer->setInterval(0);

//   default teset interval                    //15 Sec
     test_interval_timer->setInterval(m_test_param.test_interval_time);

     interval_timer_count =  (m_test_param.test_interval_time)/1000;
     interval_timer_count_tmp = interval_timer_count;
}

void measurement::start()
{
    Log() << Channel;

    detect_on_timer->start();

    emit update_test_count(static_cast<int>(current_test_count));
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
//  measure_relay->port_reset();

    measure_relay->port_control(Channel, relay_seed_ddl::sensor_port::DETECT, GpioControl::SET_VAL::SET_LOW);
    measure_relay->port_control(Channel, relay_seed_ddl::sensor_port::WORK_THIRD, GpioControl::SET_VAL::SET_LOW);

    emit update_test_count(static_cast<int>(current_test_count));
    emit update_action("Stopeed");
    emit update_interval_time(static_cast<int>(interval_timer_count));

    current_test_count=0;

    interval_count_display_timer->stop();

}

void measurement::pause()
{
    Log() << Channel;

    detect_on_timer->stop();
    work_on_timer->stop();
    third_on_timer->stop();
    detect_off_timer->stop();
    interval_count_display_timer->stop();
//  measure_relay->port_reset();
}

void measurement::detect_on()
{    
   Log() <<"meter : "<<Channel ;

   //   display the interval count decrease
   interval_count_display_timer->stop();

   emit update_action("detect on");

// measure_relay->port_control(measure_relay->relay_channel::CH_1, DDL_CH_ON);
   measure_relay->port_control(Channel, relay_seed_ddl::sensor_port::DETECT, GpioControl::SET_VAL::SET_HIGH);

   work_on_timer->start();
}

void measurement::work_on()
{    
    Log() << Channel;

    emit update_action("work on");

//  measure_relay->port_control(measure_relay->relay_channel::CH_2, DDL_CH_ON);
    measure_relay->port_control(Channel, relay_seed_ddl::sensor_port::WORK_THIRD, GpioControl::SET_VAL::SET_HIGH);

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
//  measure_relay->port_reset(Channel);

//  measure_relay->port_control(measure_relay->relay_channel::CH_1, DDL_CH_OFF);
//  measure_relay->port_control(measure_relay->relay_channel::CH_2, DDL_CH_OFF);

    measure_relay->port_control(Channel, relay_seed_ddl::sensor_port::DETECT, GpioControl::SET_VAL::SET_LOW);
    measure_relay->port_control(Channel, relay_seed_ddl::sensor_port::WORK_THIRD, GpioControl::SET_VAL::SET_LOW);

    current_test_count++;

    count_check_timer->start();    
}

void measurement::test_count_check()
{        

    Log() <<"meter channel: "<< Channel;
    Log() <<"meter memory capacitor is "<< meter_mem_capacity;
    Log() <<"Target test count is  "<< target_test_count;
    Log() <<"Current test count is  "<< current_test_count;
    Log() <<"Target test count rest is  "<< target_test_count_rest;

    emit update_test_count(static_cast<int>(current_test_count));

    if(target_test_count <= meter_mem_capacity)
    {
        Log() <<"target count is smaller than meter memory capacitor";

        if(current_test_count < target_test_count)
        {
            Log() <<"Target test count : "<< target_test_count;
            Log() <<"Current test count : "<< current_test_count;
            Log() <<"Test Continue";

            test_interval_timer->start();

            //   display the interval count decrease
            interval_count_display_timer->start(1000);
        }
        else
        {
            /*add auto download here*/
            Log() <<"data download :";
            Log() <<"Target test count : "<< target_test_count;
            Log() <<"Current test count : "<< current_test_count;
            Log() <<"Test Stop : "<< Channel;

            emit update_action("stop");
            emit stop();
        }
    }
    else
    {
         Log() <<"target count is bigger than meter memory capacitor";

          /*Target count is integer multiple of meter's memory*/
         if(!(static_cast<uint32_t>(target_test_count)%static_cast<uint32_t>(meter_mem_capacity)))
         {
               /*When current test count is reached at multiple of meters memory*/
               if(!(current_test_count%static_cast<uint32_t>(meter_mem_capacity)))
               {
                    /*add auto download here*/
                   Log() <<"data download :";
                   Log() <<"Target test count : "<< target_test_count;
                   Log() <<"Current test count : "<< current_test_count;

                   if(current_test_count < target_test_count)
                   {
                       Log() <<"Test Continue";
                       test_interval_timer->start();

                       //   display the interval count decrease
                       interval_count_display_timer->start(1000);
                   }
                   else
                   {
                       Log() <<"Target test count : "<< target_test_count;
                       Log() <<"Current test count : "<< current_test_count;
                       Log() <<"Test Stop : "<< Channel;
                       emit update_action("stop");
                       emit stop();
                   }
               }
               else
               {
                   Log() <<"Test Continue";
                   test_interval_timer->start();

                   //   display the interval count decrease
                   interval_count_display_timer->start(1000);
               }
         }
         else if((static_cast<uint32_t>(target_test_count)%static_cast<uint32_t>(meter_mem_capacity)))
         {
             /*When current test count is reached at multiple of meters memory*/
             if(!(current_test_count%static_cast<uint32_t>(meter_mem_capacity)))
             {
                  /*add auto download here*/
                 Log() <<"data download :";
                 Log() <<"Target test count : "<< target_test_count;
                 Log() <<"Current test count : "<< current_test_count;
                 Log() <<"Test Continue";
                 test_interval_timer->start();

                 // display the interval count decrease
                 interval_count_display_timer->start(1000);
             }
             else if(static_cast<uint32_t>(target_test_count) == current_test_count)
             {
                /*add auto download here*/
                 Log() <<"data download :";
                 Log() <<"Target test count : "<< target_test_count;
                 Log() <<"Current test count : "<< current_test_count;
                 Log() <<"Test Stop : "<< Channel;
                 emit update_action("stop");
                 emit stop();
             }
             else
             {
                 Log() <<"Target test count : "<< target_test_count;
                 Log() <<"Current test count : "<< current_test_count;
                 Log() <<"Test Continue";
                 test_interval_timer->start();

                 //   display the interval count decrease
                 interval_count_display_timer->start(1000);
             }
         }
         else
         {
             Log() <<"Need to debug";
             Log() <<"meter channel: "<< Channel;
             Log() <<"meter memory capacitor is "<< meter_mem_capacity;
             Log() <<"Target test count is  "<< target_test_count;
             Log() <<"Current test count is  "<< current_test_count;
         }
      }      
}

void measurement::interva_time_check()
{
     interval_timer_count_tmp--;

    if(interval_timer_count_tmp > 0)
    {
       emit update_interval_time((int)interval_timer_count_tmp);
    }
    else
    {
       interval_timer_count_tmp = interval_timer_count;
       interval_count_display_timer->stop();
    }
}

measurement::~measurement()
{

}
