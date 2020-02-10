#ifndef SERIALDEFINITION
#define SERIALDEFINITION

#include <QDebug>
#include <QTime>
#include "commondefinition.h"

//
#define SERIALCOM_QC
//#define SERIALCOM_SMARTLOG

//
#define TESTER_TIMER_INTERVAL        1000
#define TESTER_TIMER_INTERVAL_STM32  2000 // STM32_DEBUG // 1000에서 2000으로 변경 // receiveData전에 echo를 날리는 현상을 피하기 위해서
#define RECV_TIMER_INTERVAL 15

#define INVALID_BLE "zz||xx"

#endif // SERIALDEFINITION

