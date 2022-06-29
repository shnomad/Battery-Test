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

    /**/
    meter_working_status_timer = new QTimer(this);
    connect(meter_working_status_timer, SIGNAL(timeout()),SLOT(meter_working_status()));

    connect(this, SIGNAL(set_start()),SLOT(start()));
    connect(this, SIGNAL(set_detect()),SLOT(detect_on()));
    connect(this, SIGNAL(set_work_on()),SLOT(work_on()));
    connect(this, SIGNAL(set_detect_off()),SLOT(detect_off()));
    connect(this, SIGNAL(set_test_count_check()),SLOT(test_count_check()));

    m_measure_status = measurement_param::meter_status::STAND_BY;
}

void measurement::setup(measurement_param m_test_param)
{
    //Glucose : detect -> (3.5 sec) -> work/third on -> (7 sec) -> detect off -> (4 sec)
    Log() <<"Channel:"<<Channel;

    Log()<<Channel<<": meter type"<< m_test_param.type;
    Log()<<Channel <<": test channel"<< m_test_param.channel;
    Log()<<Channel <<": target_measure_count"<<m_test_param.target_measure_count;
    Log()<<Channel <<": detect_on_time"<< m_test_param.detect_on_time;
    Log()<<Channel <<": work_on_time"<< m_test_param.work_on_time;
    Log()<<Channel <<": third_on_time"<< m_test_param.third_on_time;
    Log()<<Channel <<": detect_off_time"<< m_test_param.detect_off_time;
    Log()<<Channel <<": test_interval_time"<< m_test_param.test_interval_time;
    Log()<<Channel <<": use daq970a instrument"<<m_test_param.use_daq970;

     meter_mem_capacity = m_test_param.meter_memory_capacity;
     target_test_count=m_test_param.target_measure_count;

//   default teset interval                    //15 Sec
//   test_interval_timer->setInterval(static_cast<int>(m_test_param.test_interval_time));
     interval_timer_count =  (m_test_param.test_interval_time)/1000;
     interval_timer_count_tmp = interval_timer_count;

     memcpy(&m_test_param_tmp, &m_test_param, sizeof(m_test_param));
     memcpy(&m_test_param_delay, &m_test_param_tmp, sizeof(m_test_param));

     Log()<<Channel<<": start_to_detect_on_delay"<<m_test_param_delay.detect_on_time;
     Log()<<Channel<<": detect_to_work_on_delay"<<m_test_param_delay.work_on_time;
     Log()<<Channel<<": third_on_to_detect_off_delay"<<m_test_param_delay.detect_off_time;
     Log()<<Channel<<": detect_off_to_measurement_interval_delay" <<m_test_param_delay.test_interval_time;
}

void measurement::start()
{
    Log() << Channel;

    memcpy(&m_test_param_delay, &m_test_param_tmp, sizeof(m_test_param));

    m_measure_status = measurement_param::meter_status::MEASURE_START;
    emit update_test_count(static_cast<int>(current_test_count));

    Log()<<Channel <<": detect_on_time"<< m_test_param_delay.detect_on_time;
    Log()<<Channel <<": work_on_time"<< m_test_param_delay.work_on_time;
    Log()<<Channel <<": detect_off_time"<< m_test_param_delay.detect_off_time;
    Log()<<Channel <<": test_interval_time"<< m_test_param_delay.test_interval_time;

    meter_working_status_timer->start(1000);
}

void measurement::stop()
{
    Log() << Channel ;

    m_measure_status = measurement_param::meter_status::MEASURE_STOP;

    meter_working_status_timer->stop();

    measure_relay->port_control(Channel, relay_seed_ddl::sensor_port::DETECT, GpioControl::SET_VAL::SET_LOW);
    measure_relay->port_control(Channel, relay_seed_ddl::sensor_port::WORK_THIRD, GpioControl::SET_VAL::SET_LOW);

    emit update_test_count(static_cast<int>(current_test_count));
    emit update_action("Stopeed");
    emit update_interval_time(static_cast<int>(interval_timer_count));

    /*measurement status transition delay*/

    current_test_count=0;
}

void measurement::pause()
{
    Log() << Channel;

    m_measure_status = measurement_param::meter_status::MEASURE_PAUSE;

    meter_working_status_timer->stop();
}

void measurement::detect_on()
{    
   Log() <<"meter : "<<Channel ;
   m_measure_status = measurement_param::meter_status::DETECT_ON;

   emit update_action("detect on");

   measure_relay->port_control(Channel, relay_seed_ddl::sensor_port::DETECT, GpioControl::SET_VAL::SET_HIGH);
}

void measurement::work_on()
{    
    Log() <<"meter"<<Channel;
    m_measure_status = measurement_param::meter_status::WORK_ON;

    emit update_action("work on");

    measure_relay->port_control(Channel, relay_seed_ddl::sensor_port::WORK_THIRD, GpioControl::SET_VAL::SET_HIGH);
}

void measurement::third_on()
{
    Log() <<"meter : "<< Channel;
    m_measure_status = measurement_param::meter_status::THIRD_ON;

    emit update_action("third on");
}

void measurement::detect_off()
{
    Log() <<"meter : "<< Channel;
    m_measure_status = measurement_param::meter_status::DETECT_OFF;

    emit update_action("detect off");

    measure_relay->port_control(Channel, relay_seed_ddl::sensor_port::DETECT, GpioControl::SET_VAL::SET_LOW);
    measure_relay->port_control(Channel, relay_seed_ddl::sensor_port::WORK_THIRD, GpioControl::SET_VAL::SET_LOW);

    current_test_count++;
}

void measurement::test_count_check()
{
    Log() <<"meter : "<< Channel;

    meter_working_status_timer->stop();

    m_measure_status = measurement_param::meter_status::TEST_COUNT_CHECK;

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

    m_measure_status = measurement_param::meter_status::MEASURE_INTERVAL;
    meter_working_status_timer->start(1000);
}

void measurement::meter_working_status()
{
      Log() <<"meter : "<< Channel;
      Log() <<"meter_working_status"<< m_measure_status;

    switch(m_measure_status)
    {
        case measurement_param::meter_status::MEASURE_START :

            if(m_test_param_delay.detect_on_time > 0)
            {
                Log()<<"start_delay :"<< m_test_param_delay.detect_on_time;
                m_test_param_delay.detect_on_time -=1000;
            }
            else
            {
                emit set_detect();
            }

        break;

        case measurement_param::meter_status::DETECT_ON :

            if(m_test_param_delay.work_on_time > 0)
            {
                Log()<<"detect_on_delay :"<< m_test_param_delay.work_on_time;
                m_test_param_delay.work_on_time -=1000;
            }
            else
            {
                emit set_work_on();
            }

        break;

        case measurement_param::meter_status::WORK_ON :
        case measurement_param::meter_status::THIRD_ON :

            if(m_test_param_delay.detect_off_time > 0)
            {
                Log()<<"work_on_delay :"<<m_test_param_delay.detect_off_time;
                 m_test_param_delay.detect_off_time -=1000;
            }
            else
            {
                emit set_detect_off();
            }

        break;

        case measurement_param::meter_status::DETECT_OFF:

                emit set_test_count_check();

        break;

        case measurement_param::meter_status::MEASURE_INTERVAL:

            if(m_test_param_delay.test_interval_time > 0)
            {
                Log()<<"measurement_interval_delay :"<<m_test_param_delay.test_interval_time;

                emit update_interval_time(static_cast<int>(m_test_param_delay.test_interval_time/1000));

                m_test_param_delay.test_interval_time -=1000;
            }
            else
            {
                emit set_start();                
            }

        break;

        default:
        break;

    }
}

measurement::~measurement()
{

}
