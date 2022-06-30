#ifndef MEASUREMENT_PARAM_H
#define MEASUREMENT_PARAM_H

#include<QObject>

using namespace std;

struct measurement_param
{    
    enum  meter_type{GLUCOSE_BASIC, GLUCOSE_BLE, GLUCOSE_VOICE, GLUCOSE_VOICE_BLE, KETONE_BASIC, KETONE_BLE};
    enum  meter_channel{CH_1=0x1, CH_2=0x2, CH_3=0x3, CH_4=0x4, CH_5=0x5};
    enum  meter_status{STAND_BY=0x0, MEASURE_START, DETECT_ON, WORK_ON, THIRD_ON, DETECT_OFF, TEST_COUNT_CHECK,MEASURE_INTERVAL, MEASURE_STOP, MEASURE_PAUSE,COMM_MODE=0x10, DATA_DOWNLOAD};

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
    bool use_u1272a = false;
    bool auto_download = false;
}; Q_DECLARE_METATYPE(measurement_param)


#endif // MEASUREMENT_PARAM_H
