#ifndef COMMONDEFINITION
#define COMMONDEFINITION

#include <QDebug>
#include <QDateTime>
#include <QEventLoop>

 #define Log() qDebug() <<"["<<QDateTime::currentDateTime().toString("MM.dd hh:mm:ss") << __PRETTY_FUNCTION__ << __LINE__ << "]"
//#define Log() qDebug()<<"["<< __PRETTY_FUNCTION__ << __LINE__ << "]"
//#define Logf() qDebug(logDebug()) <<"["<< __PRETTY_FUNCTION__ << __LINE__ << "]"

// ==========================
// comment all for normal QC Program
//
// Loop Program for EMC Test
//#define LOOP_FOR_EMC_TEST      "EMC/Safety"
//#define LOOP_FOR_EMC_TEST_DOWNLOAD_ONE // uncomment for one download

// Communication Test for ExpertPlus Cradle
//#define LOOP_FOR_XP_CRADLE

// JIG Mode
//#define JIG_MODE
// ==========================

// N-ISO QC
#define PROGRAM_QC         "N-ISO QC Program"
#define PROGRAM_QC_FDA         "N-ISO QC Program_FDA"
#define PROGRAM_QC_DEL         "Memory Delete Program"
#define QC_TYPE_DEL       "QC_TYPE_DEL"
#define QC_TYPE_GENERAL "QC_TYPE_GENERAL"
#define QC_TYPE_FDA "QC_TYPE_FDA"
#define QC_VERSION          "(v2.1.5.18)"
#define QC_FDA_VERSION     "(v2.1.5.15)"
#define QC_DEL_VERSION      "(v2.0.0)"
// N-ISO Color QC
#define PROGRAM_POST_COLOR  "_color"
#define QC_VERSION_COLOR    "(v2.0.1.1)"

// Loop Program for EMC/Safety
#define PROGRAM_EMC        "Loop Program for EMC Test"
#define EMC_VERSION         "(v0.4.1)"
// JIG Mode
//#define PROGRAM_JIG         "JIGMode Program"
//#define JIG_VERSION          "(v0.2)"
// COMM_TEST_FOR_XP_CRADLE
#define PROGRAM_NAME_XP_CRADLE "Loop Program for XP Cradle"
#define PROGRAM_VERSION_XP_CRADLE "(v0.1.0)"

#if defined(LOOP_FOR_EMC_TEST)
#define PROGRAM_NAME PROGRAM_EMC
#define PROGRAM_VER EMC_VERSION
#elif defined(JIG_MODE)
#define PROGRAM_NAME PROGRAM_JIG
#define PROGRAM_VER JIG_VERSION
#elif defined(LOOP_FOR_XP_CRADLE)
#define PROGRAM_NAME PROGRAM_NAME_XP_CRADLE
#define PROGRAM_VER PROGRAM_VERSION_XP_CRADLE
#else // default
#define PROGRAM_NAME PROGRAM_QC
#define PROGRAM_VER QC_VERSION

#endif

#define PW_VALUE			"2645"
#define PW_HYPO             "hypo"

#define MSG_NoConnection   "--------"
#define MSG_NoSN           "----"

#define MSG_SET_INIT "INIT"
#define MSG_SET_NODATA "설정없음"

#define MSG_UNIT_MGDL "mg/dL fix"
#define MSG_UNIT_MMOLL "mmol/L fix"

#define MSG_TEMP_CENTIGRADE "섭씨(ºC)"
#define MSG_TEMP_FAHRENHEIT "화씨(ºF)"

#define MSG_DATEFORMAT_MMDD "MM-DD"
#define MSG_DATEFORMAT_DDMM "DD-MM"

#define MSG_HOURMODE_12H    "12H"
#define MSG_HOURMODE_24H    "24H"

#define DATAFLAG_NOMARK         "Nomark"
#define DATAFLAG_PREMEAL        "식전"
#define DATAFLAG_POSTMEAL       "식후"
#define DATAFLAG_FASTING        "공복"
#define DATAFLAG_ENABLE         "활성화"
#define DATAFLAG_DISABLE         "비활성화"
#define DATAFLAG_DIVIDE         " : "

#define BLEWAY_PASSKEY          "Passkey"
#define BLEWAY_JUSTWORKS        "Just Works"
#define BLEWAY_NO               "No BLE"

#define MEASURE_RANGE_10_600          "10~600(mg/dL)"
#define MEASURE_RANGE_20_600          "20~600(mg/dL)"

#define TEMPERATURE_RANGE_5_50          "5~50(ºC)"
#define TEMPERATURE_RANGE_5_45          "5~45(ºC)"
#define TEMPERATURE_RANGE_6_44          "6~44(ºC)"

#define APNKEY_KT  "alwayson-r6.ktfwing.com"
#define APNKEY_SKT "web.sktelecom.com"

#define PROPERTY_USE            "USE"
#define PROPERTY_NOTUSE         "NOT USE"
#define STRIP_NAME_BAROZEN      "바로잰"
#define STRIP_NAME_BAROZEN_K    "바로잰 케톤"
#define STRIP_NAME_CSPRO        "CareSensPRO"
#define STRIP_NAME_CSPRO_K      "케톤"

#define AUDIO_MODE_OFF          "OFF"
#define AUDIO_MODE_BEEP    "BEEP"
#define AUDIO_MODE_VOICE    "VOICE"

#define AUDIO_MODE    "Audio 모드"
#define AUDIO_VOLUME    "Volume"
#define AUDIO_LANGUAGE           "지원언어"

#define ANIMAL_TYPE_DOG     "개"
#define ANIMAL_TYPE_CAT     "고양이"

#endif // COMMONDEFINITION
