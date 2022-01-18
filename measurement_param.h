#ifndef MEASUREMENT_PARAM_H
#define MEASUREMENT_PARAM_H

#include<QObject>
#if 1

class measurement_param
{
    public:

    enum meter_type {GLUCOSE_BASIC , GLUCOSE_BLE, GLUCOSE_VOICE, GLUCOSE_VOICE_BLE, KETONE_BASIC, KETONE_BLE};

    quint32 target_measure_count=0;//1000;
    quint32 detect_on_time = 0;
    quint32 work_on_time = 0; //3000;
    quint32 third_on_time = 0; //6000;
    quint32 detect_off_time = 0; //9000; //14000;
    quint32 test_interval_time =0; // 15000;
    quint32 hub_port_delay_time = 0;        //2000;
    quint32 measure_count_read_from_meter=0;           
};

#else
struct measurement_param
{
    enum{GLUCOSE_BASIC , GLUCOSE_BLE, GLUCOSE_VOICE, GLUCOSE_VOICE_BLE, KETONE_BASIC, KETONE_BLE} METER_TYPE;

    quint32 target_measure_count=0;//1000;
    quint32 detect_on_time = 0;
    quint32 work_on_time = 0; //3000;
    quint32 third_on_time = 0; //6000;
    quint32 detect_off_time = 0; //9000; //14000;
    quint32 test_interval_time =0; // 15000;
    quint32 hub_port_delay_time = 0;        //2000;
    quint32 measure_count_read_from_meter=0;
};
#endif

#endif // MEASUREMENT_PARAM_H
