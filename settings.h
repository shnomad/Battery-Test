#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

class Settings : public QSettings
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = 0);
    ~Settings();

    static Settings *Instance();
    const QHash<QString, QVariant> getSettings();
    void setSettings(const QHash<QString, QVariant> &settingValue);
    void setQcStringValue(QString keyname, QString value);
    QString getQCValue(QString keyname);
    QString getComponentStyleSheet();

    // sn
    void setSerialNumber(QString current_sn, QString new_sn);
    QString getSerialNumber();

    //temperature
    void setTemperature(QString value);
    QString getTemperature();

    //ble
    void setBleName(QString value);
    QString getBleName();
	    void setBleMeter(int value);
    void setSaveDataCnt(int data_count);
    int isBleMeter();
    int getSaveDataCnt();

    //meter type
    void setMeterType(int index);
    void setAutoSN(int value);
    void setAutoTimesync(int value);
    void setBaudrate(int baudrate);
    int getBaudrate();

    //code mode
    void setCodingMode(int value);
    int getCodingMode();

    //otg mode
    void setOtgMode(int mode); //0:normal, 1:china
    int getOtgMode();

    //aeskey
    void setAESKey(QString current_aeskey, QString new_aeskey);
    //APNKey
    void setAPNKey(QString setAPNKey);

    //server address
    void setServerAddress(QString server_address);
    //device token
    void setDeviceToken(QString device_token);
    //service name
    void setServiceName(QString service_name);
    //IMEI
    void setIMEI(QString imei);
    //ICCID
    void setICCID(QString iccid);
    //module software version
    void setModuleSoftwareVer(QString module_software_ver);
    //IMSI
    void setIMSI(QString imsi);
    //SKT serial number
    void setSktSN(QString skt_sn);
    //NIoT URL
    void setRegistrationURL(QString url);

    //meter time
    void setMeterTime(QString time);
    QString getMeterTime();
    //system time
    void setSystemTime(QString time);
    QString getSystemTime();

    //function settins
    bool isTI_setyear();
    bool isSetdataflag();
    bool isSethyperhypo();
    bool isSetstrips();
    bool isSetAudioSettings();
    bool isSetAvgDays();

signals:

public slots:

private:
    QString getAppPath();

};

//프로그램 UI의 각종 설정 정보 및 QC Data 등을 저장할 것이다.

#endif // SETTINGS_H
