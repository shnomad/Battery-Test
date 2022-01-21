#ifndef MEASUREMENT_PARAM_H
#define MEASUREMENT_PARAM_H

#include<QObject>

using namespace std;

#if 1

struct measurement_param
{    
    enum  meter_type{GLUCOSE_BASIC, GLUCOSE_BLE, GLUCOSE_VOICE, GLUCOSE_VOICE_BLE, KETONE_BASIC, KETONE_BLE};
    enum  meter_channel{CH_1=0x1, CH_2=0x2};

    meter_type type;
    meter_channel channel;
    quint32 target_measure_count; //1000;
    quint32 detect_on_time;
    quint32 work_on_time; //3000;
    quint32 third_on_time; //6000;
    quint32 detect_off_time; //9000; //14000;
    quint32 test_interval_time; // 15000;
    quint32 hub_port_delay_time;        //2000;
    quint32 measure_count_read_from_meter;
};

#else

class measurement_param
{

public:

    enum  meter_channel{CH_1=0x1, CH_2=0x2};
    enum  meter_type{GLUCOSE_BASIC=0x0 , GLUCOSE_BLE, GLUCOSE_VOICE, GLUCOSE_VOICE_BLE, KETONE_BASIC, KETONE_BLE};    

    meter_channel channel;
    meter_type type;   
    quint32 target_measure_count = 1000; //1000;
    quint32 detect_on_time = 0;
    quint32 work_on_time = 3000; //3000;
    quint32 third_on_time = 6000; //6000;
    quint32 detect_off_time = 9000; //9000; //14000;
    quint32 test_interval_time = 15000; // 15000;
    quint32 hub_port_delay_time;        //2000;
    quint32 measure_count_read_from_meter;

    measurement_param(meter_channel _ch,meter_type _type, quint32 count,quint32 detect_on, quint32 work_on,quint32 third_on,quint32 detect_off,quint32 interval)
    {
        channel = _ch;
        type = _type;
        target_measure_count = count;
        detect_on_time = detect_on;
        work_on_time = work_on; //3000;
        third_on_time = third_on; //6000;
        detect_off_time = detect_off; //9000; //14000;
        test_interval_time = interval; // 15000;
    }

    measurement_param(const measurement_param& copy)
    {
        quint32 target_measure_count = target_measure_count ; //1000;
        quint32 detect_on_time = detect_on_time;
        quint32 work_on_time = work_on_time; //3000;
        quint32 third_on_time = third_on_time; //6000;
        quint32 detect_off_time = detect_off_time; //9000; //14000;
        quint32 test_interval_time = test_interval_time; // 15000;
        quint32 hub_port_delay_time = hub_port_delay_time;        //2000;
        quint32 measure_count_read_from_meter = measure_count_read_from_meter;
    }

    ~measurement_param()
    {

    }
};


#endif

#endif // MEASUREMENT_PARAM_H
