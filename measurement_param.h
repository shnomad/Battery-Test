#ifndef MEASUREMENT_PARAM_H
#define MEASUREMENT_PARAM_H

#include<QObject>

using namespace std;

struct measurement_param
{    
    enum  meter_type{GLUCOSE_BASIC, GLUCOSE_BLE, GLUCOSE_VOICE, GLUCOSE_VOICE_BLE, KETONE_BASIC, KETONE_BLE};
    enum  meter_channel{CH_1=0x1, CH_2=0x2, CH_3=0x3, CH_4=0x4, CH_5=0x5};

    meter_type type;
    meter_channel channel;
    quint32 target_measure_count; //1000;
    quint32 detect_on_time;
    quint32 work_on_time; //3000;
    quint32 third_on_time; //6000;
    quint32 detect_off_time; //9000; //14000;
    quint32 test_interval_time; // 15000;
    quint32 meter_memory_capacity=0;    
    quint32 hub_port_delay_time;        //2000;
    quint32 measure_count_read_from_meter;
    bool use_daq970;
    bool auto_download;
}; Q_DECLARE_METATYPE(measurement_param)

#endif // MEASUREMENT_PARAM_H
