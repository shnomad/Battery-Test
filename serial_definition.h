#ifndef SERIAL_DEFINITION_H
#define SERIAL_DEFINITION_H

#include <QDebug>
#include <QTime>

#define Log()\
    qDebug() <<"["<<QTime::currentTime().toString("mm:ss.sss") << __PRETTY_FUNCTION__ << __LINE__ << "]"

//
#define TESTER_TIMER_INTERVAL        1000 // STM32_DEBUG // avoid to send echo before receiveData
#define TESTER_TIMER_INTERVAL_STM32  2000
#define RECV_TIMER_INTERVAL          15

#define MODE_DEFAULT            "MODE_DEFAULT"
#define MODE_CAREASTLINK        "MODE_CAREFASTLINK"
#define MODE_COLORMETER         "MODE_COLORMETER"
#define MODE_ALL                "MODE_ALL"
#define INVALID_BLE "zz||xx"

#endif // SERIAL_DEFINITION_H
