#include "settings.h"
#include <QApplication>
#include "commondefinition.h"
#include "setting_flagname_definition.h"

static Settings* m_pInstance = NULL;
Settings *Settings::Instance()
{
    if (!m_pInstance)
    {
        m_pInstance = new Settings();
    }
    return m_pInstance;
}

Settings::Settings(QObject *parent) : QSettings(getAppPath() + "/Settings.ini", QSettings::IniFormat, parent)
{
    //check first run
    if(value(SFD_q_unitset).isNull() == true)
    {
        //insert run setting info
        setValue(SFD_baudrate,0);
        setValue(SFD_meter_type,1);
        setValue(SFD_q_unitset,0);
        setValue(SFD_q_tempset, 0);
        setValue(SFD_q_dateformat, 0);
        //default start end year & memory count
        setValue(SFD_q_startyear, 2017);
        setValue(SFD_q_endyear, 2030);
        setValue(SFD_q_memory_count, 1000);

        setValue(SFD_u_setbleonoff, 0);
        //data flag
        setValue(SFD_u_staticdataflag, 1);
        setValue(SFD_q_flag_default, 1);
        setValue(SFD_q_flag_premeal, 1);
        setValue(SFD_q_flag_postmeal,1);
        setValue(SFD_q_flag_fasting, 1);

        //average days
        setValue(SFD_q_enabled_avg_days_count, 5);
        setValue(SFD_q_flag_avg_days_1, 1);
        setValue(SFD_q_flag_avg_days_7, 1);
        setValue(SFD_q_flag_avg_days_14, 1);
        setValue(SFD_q_flag_avg_days_30, 1);
        setValue(SFD_q_flag_avg_days_60, 0);
        setValue(SFD_q_flag_avg_days_90, 1);

        setValue(SFD_q_flag_year, 0);

        setValue(SFD_q_ble_meter, 0);  // 0:non ble , 1:ble
        setValue(SFD_q_savedata_count, 0);

        setValue(SFD_u_measure_range_index, 1);
        setValue(SFD_q_hyper_on_off, 1);
        setValue(SFD_q_hypo_on_off,1);
        setValue(SFD_q_buzzer_on_off, 0);

        //hyper hypo initial
        setValue(SFD_q_hypo_initial, 90);
        setValue(SFD_q_hyper_initial, 240);

        //검사지
        setValue(SFD_g_strip_type_cspro, QVariant(1));

        //ti setcode
        setValue(SFD_u_ti_setcode, -1);//default - no use , -1

        //audio settings default        
        setValue(SFD_q_setaudiovolume, 4);
        setValue(SFD_q_setaudiolangcount, 3);

        //n iot
        setValue(SFD_q_skt_sn, "0000001");
        setValue(SFD_q_server_address, "api.sktiot.com:1883");
        setValue(SFD_q_service_name, "glucose");

        //vetmate
        setValue(SFD_q_animal_type, 0);

        //otg mode
        setValue(SFD_q_otg_mode, 0); //0:normal, 1:china
    }
}

Settings::~Settings()
{

}

void Settings::setSaveDataCnt(int data_count)
{
    setValue(SFD_q_savedata_count, QVariant(data_count));
    sync();
}

void Settings::setQcStringValue(QString keyname, QString value)
{
    setValue(keyname, QVariant(value));
    sync();
}

QString Settings::getQCValue(QString keyname)
{
    return value(keyname).toString();
}

void Settings::setBaudrate(int baudrate) {
    if(baudrate <= 0) {
        baudrate = 9600;
    }
    setValue(SFD_baudrate, baudrate);
    sync();
}

int Settings::getBaudrate() {
    int baudrate = value(SFD_baudrate).toInt();

    // parsing old version of baudrate index
    // old version stored the index of baudrate (0/9600,1/19200,2/115200)
    if(baudrate < 1200) {
        if(baudrate == 0) {
            baudrate = 9600;
        }
        else if(baudrate == 1) {
            baudrate = 19200;
        }
        else if(baudrate == 2) {
            baudrate = 115200;
        }
        else {
            baudrate = 9600; // default
        }
    }

    return baudrate;
}

void Settings::setCodingMode(int value)
{
    setValue(SFD_q_code_mode, value);
    sync();
}

int Settings::getCodingMode()
{
    return value(SFD_q_code_mode).toInt();
}

void Settings::setOtgMode(int mode)
{
    setValue(SFD_q_otg_mode, mode);
    sync();
}

int Settings::getOtgMode()
{
    return value(SFD_q_otg_mode).toInt();
}

void Settings::setMeterType(int index)
{
    setValue(SFD_meter_type, index);
    sync();
}

void Settings::setAutoSN(int value)
{
    setValue(SFD_u_auto_sn, value);
    sync();
}

void Settings::setAutoTimesync(int value)
{
    setValue(SFD_u_auto_timesync, value);
    sync();
}

void Settings::setTemperature(QString value)
{
    setValue(SFD_q_currentTemperature, QVariant(value));
    sync();
}

QString Settings::getTemperature()
{
    return value(SFD_q_currentTemperature).toString();
}

void Settings::setSerialNumber(QString current_sn, QString new_sn)
{
    setValue(SFD_q_serialnumber, QVariant(current_sn));
    setValue(SFD_q_new_serialnumber, QVariant(new_sn));
    sync();
}

void Settings::setBleMeter(int value)
{
    setValue(SFD_q_ble_meter, value);
    sync();
}

int Settings::isBleMeter()
{
    return value(SFD_q_ble_meter).toInt();
}

bool Settings::isTI_setyear()
{
    return value(SFD_q_flag_year).toInt() == 1? true : false;
}

bool Settings::isSetdataflag()
{
    return value(SFD_u_setdataflag).toInt() == 1? true : false;
}

bool Settings::isSetAvgDays()
{
    return value(SFD_u_setavgdays).toInt() == 1? true : false;
}

bool Settings::isSethyperhypo()
{
    return value(SFD_u_sethyperhypo).toInt() == 1 ? true : false;
}

bool Settings::isSetstrips()
{
    return value(SFD_u_setstrips).toInt() == 1 ? true : false;
}

bool Settings::isSetAudioSettings()
{
    return value(SFD_u_setAudioSettings).toInt() == 1 ? true : false;
}

int Settings::getSaveDataCnt()
{
    return value(SFD_q_savedata_count).toInt();
}

void Settings::setAESKey(QString current_aeskey, QString new_aeskey)
{
    setValue(SFD_q_aeskey, QVariant(current_aeskey));
    setValue(SFD_q_new_aeskey, QVariant(new_aeskey));
    sync();
}

void Settings::setAPNKey(QString setAPNKey)
{
    setValue(SFD_q_apnkey, QVariant(setAPNKey));
    sync();
}

void Settings::setServerAddress(QString server_address)
{
    setValue(SFD_q_server_address, QVariant(server_address));
    sync();
}

void Settings::setDeviceToken(QString device_token)
{
    setValue(SFD_q_device_token, QVariant(device_token));
    sync();
}

void Settings::setServiceName(QString service_name)
{
    setValue(SFD_q_service_name, QVariant(service_name));
    sync();
}

void Settings::setIMEI(QString imei)
{
    setValue(SFD_q_imei, QVariant(imei));
    sync();
}

void Settings::setICCID(QString iccid)
{
    setValue(SFD_q_iccid, QVariant(iccid));
    sync();
}

void Settings::setModuleSoftwareVer(QString module_software_ver)
{
    setValue(SFD_q_module_software_ver, QVariant(module_software_ver));
    sync();
}

void Settings::setIMSI(QString imsi)
{
    setValue(SFD_q_imsi, QVariant(imsi));
    sync();
}

void Settings::setSktSN(QString skt_sn)
{
    setValue(SFD_q_skt_sn, QVariant(skt_sn));
    sync();
}

void Settings::setRegistrationURL(QString url)
{
    setValue(SFD_q_registration_url, QVariant(url));
    sync();
}

void Settings::setMeterTime(QString time)
{
    setValue(SFD_q_meter_date_time, QVariant(time));
    sync();
}

QString Settings::getMeterTime()
{
    return value(SFD_q_meter_date_time).toString();
}

void Settings::setSystemTime(QString time)
{
    setValue(SFD_q_system_date_time, QVariant(time));
    sync();
}

QString Settings::getSystemTime()
{
    return value(SFD_q_system_date_time).toString();
}

QString Settings::getSerialNumber()
{
    return value(SFD_q_serialnumber).toString();
}

void Settings::setBleName(QString value)
{
    setValue(SFD_q_blename, QVariant(value));
    sync();
}

QString Settings::getBleName()
{
    return value(SFD_q_blename).toString();
}

const QHash<QString, QVariant> Settings::getSettings() {
    QHash<QString, QVariant> ret;
    QStringList keys = allKeys();
    foreach(QString key, keys) {
        //Log() << key << value(key);
        ret.insert(key, value(key));
    }
    return ret;
}

void Settings::setSettings(const QHash<QString, QVariant> &settingValue)
{
    QStringList keys = settingValue.keys();
    setIniCodec("UTF-8");
    foreach(QString key, keys)
    {        
        //Log() << key << settingValue.value(key);
        setValue(key, settingValue.value(key));

    }
    sync();
}

QString Settings::getAppPath()
{
    return qApp->applicationDirPath();
}

QString Settings::getComponentStyleSheet()
{
    QString fontfamily = "font-family: NanumGothic;";
    QString styleSheet =
            //"QLabel"
            //
            "QLabel {"
                "color: #2f3b4b;"
                "font: 20px;"
            "min-height: 27px; max-height: 27px;"
                + fontfamily +
            "}"
            "QLabel::disabled {"
                "color: #868787;"
            "}"

            // QTextEdit
            //
//            "QTextEdit {"
//                "color: #868787;"
//                "font: 20px;"
//                + fontfamily +
//            "}"

            // QLineEdit
            //
            "QLineEdit {"
                "color: #FF22FF;"
                "font: 20px;"
                + fontfamily +
                "padding-right: 5px; padding-left: 5px;"
                "border: 1px solid darkgray; border-radius: 7px;"
                "min-height: 35px; max-height: 35px;"
                "background-color: #f5f5f5;"
            "}"
            "QLineEdit:read-only {"
                "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                "stop: 0 #E3E3E3, stop: 0.4 #E8E8E8,"
                "stop: 0.5 #EEEEEE, stop: 1.0 #F1F1F1);"
            "}"
            "QLineEdit:disabled {"
                "background-color: #dcdcdf;"
                "color: #868787;"
            "}"

            //"QCheckBox"
            //
            "QCheckBox {"
                "color: #2f3b4b;"
                "font: 20px;"
            "min-height: 27px; max-height: 27px;"
                + fontfamily +
            "}"
            "QCheckBox::disabled {"
                "color: #868787;"
            "}"

            // QDateEdit
            //
            "QDateEdit {"
            "background-color: #f5f5f5;"
            "color: #235772;"
            "font: 14px;"
            + fontfamily +
            "}"
            "QDateEdit:disabled {"
                "color: #868787;"
            "}"
            // QPushButton
            //

            "QPushButton  {"
            "font: 20px;"
            + fontfamily +
            "padding-right: 8px; padding-left: 8px;"
            "border: 1px solid darkgray; border-radius: 7px;"
            "min-height: 35px; max-height: 50px;"
            "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\
                                stop: 0 #dcdcdf, stop: 0.2 #dcdcdf,\
                                stop: 0.2 #f5f5f5, stop: 1.0 #f5f5f5);"
            "}"
            "QPushButton:hover  {"
            "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\
                                stop: 0 #dcdcdf, stop: 0.5 #dcdcdf,\
                                stop: 0.5 #f5f5f5, stop: 1.0 #f5f5f5);"
            "}"
            "QPushButton::disabled {"
                "background-color: #dcdcdf;"
            "}"


            "QListWidget  {"
            //"color: #FF3322;"
            "font: 20px;"
            + fontfamily +
            "padding-right: 8px; padding-left: 8px;"
            "border: 1px solid darkgray; border-radius: 7px;"
            "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\
                                stop: 0 #dcdcdf, stop: 0.2 #dcdcdf,\
                                stop: 0.2 #f5f5f5, stop: 1.0 #f5f5f5);"
            "}"

            // QComboBox
            //
            "QComboBox  {"
            "color: #235772;"
            "font: 20px;"
            + fontfamily +
            "padding-right: 8px; padding-left: 8px;"
            "border: 1px solid darkgray; border-radius: 7px;"
            "min-height: 35px; max-height: 50px;"
            "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\
                                stop: 0 #dcdcdf, stop: 0.2 #dcdcdf,\
                                stop: 0.2 #f5f5f5, stop: 1.0 #f5f5f5);"
            "}"
            "QComboBox:disabled {"
                    "background-color: #dcdcdf;"
                    "color: #868787;"
            "}"
            "QComboBox::drop-down  {"
            "width: 27px; height: 27px;"
            "image: url(:/images/arrow_normal.png);"
            "}"
            "QComboBox::drop-down:hover  {"
            "width: 27px; height: 27px;"
            "image: url(:/images/arrow_over.png);"
            "}"
            "QComboBox::drop-down:pressed  {"
            "width: 27px; height: 27px;"
            "image: url(:/images/arrow_over.png);"
            "}"
            "QComboBox QAbstractItemView {\
                color: #235772;\
                selection-background-color:  rgb(8, 144, 203);\
                background-color: rgb(245, 245, 245);\
                outline:none;\
            }";
    return styleSheet;
}
