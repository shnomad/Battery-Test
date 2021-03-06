#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "commondefinition.h"
#include "serialcom/serialprotocol.h"
#include "serialcom/serialcomm.h"
#include "serialcom/serialprotocol3.h"

QT_BEGIN_NAMESPACE

using namespace std;

namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

//class relay_seed;
class relay_seed_ddl;
class QElapsedTimer;
class SerialProtocolAbstract;

#define GLUCOSE_BASIC       0x0
#define GLUCOSE_BLE         0x1
#define GLUCOSE_VOICE       0x2
#define GLUCOSE_VOICE_BLE   0x3
#define KETONE_BASIC        0x4
#define KETONE_BLE          0x5

#define RASPBERRY_PI3_B         0x0
#define RASPBERRY_PI3_B_PLUS    0x1
#define RASPBERRY_PI_UNKNOWN    0x2

class MainWindow : public QMainWindow
{
    Q_OBJECT
    SerialComm *serialComm;
    SerialProtocolAbstract *protocol;
    QString m_qcType;

    quint32 current_measure_count=0;
    quint32 target_measure_count=0, target_measure_count_rest=0;
    quint32 meter_mem_capacity =1000;
    quint8 target_test_cycle = 0, current_test_cycle = 0;    
    quint8 comm_retry_count =0;

    bool measure_test_active = false;
    bool GluecoseResultDataExpanded = false;
    bool isDeviceOpened = false;
    bool meter_comm_user_request=0;
    bool meter_comm_measure_count_check_request=0;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    quint32 detect_on_time = 0;
    quint32 work_on_time = 0; //3000;
    quint32 third_on_time =0; // 6000;
    quint32 detect_off_time = 0; //9000; //14000;
    quint32 test_interval_time = 0; //15000;
    quint32 hub_port_delay_time = 2000;
    quint32 measure_count_read_from_meter=0;

    enum SIGNAL_SENDER{
        SIGNAL_FROM_MEASURE_DETECT_OFF = 0x0,
        SIGNAL_FROM_FINISH_DO_COMMAND,
        SIGNAL_FROM_COMM_ERROR
    };

private slots:
    void on_test_start_clicked();
    void on_test_stop_clicked();
    void measurement();
    void detect_on();
    void work_on();
    void third_on();
    void detect_off();
    void measure_port_reset();
    void measure_port_init();
    void measure_count_check(SIGNAL_SENDER);    
    quint8 hub_port_open();
    void hub_port_close();
    void hub_port_reset();
    void on_quit_clicked();
    void on_device_open_clicked();
    void on_device_close_clicked();
    void meter_comm_start();
    void meter_comm_end();
    void on_times_valueChanged(const QString &arg1);
    void UpdateTime();
    void comm_polling_event();          //periodical polling for device
    void comm_polling_event_start();    //periodical polling for device
    void comm_polling_event_stop();     //periodical polling for device

private Q_SLOTS:
    void portReady();
    void connectionError();
    void textMessage(QString text);
    // 다운로드 과정에서 protocol에서 보내오는 시그널과 연결되는 함수
    void downloadProgress(float progress);                                  // 0~1 : 1 = 100%
    void downloadComplete(QJsonArray* datalist);
    void timeoutError(Sp::ProtocolCommand command);
    void errorOccurred(Sp::ProtocolCommand command, Sp::ProtocolCommand preCommand);  // 미터에서 보내온 오류
    void errorCrc();                                    // 수신데이터 CRC 오류
    void errorUnresolvedCommand(Sp::ProtocolCommand command);     // 다운로드 과정에서 수행하지 않는 커맨드 패킷 수신
    void packetReceived();
    void finishReadingQcData();
    void finishDoCommands(bool bSuccess, Sp::ProtocolCommand lastcommand);
    void maintainConnection(bool isOK);
    void needReopenSerialComm();                                            // 시리얼 포트 오류로 인해 다시 포트를 열어 복구가 필요한 경우
    void on_mem_delete_clicked();
    void on_time_sync_clicked();
    void SaveCSVFile(QJsonArray datalist);
    void SaveCSVFile_default(QString filepath, QJsonArray datalist);
    void on_reboot_clicked();
    void currentMeterIndexChanged(int index);
    void on_download_clicked();        
    void on_test_pause_clicked();

    void on_sec_startdelay_valueChanged(const QString &arg1);
    void on_sec_detoffdelay_valueChanged(const QString &arg1);
    void on_sec_interval_valueChanged(const QString &arg1);

    void on_checkBox_stateChanged(int arg1);

signals:
    void measure_start();
    void measure_cnt_check(SIGNAL_SENDER);
    void measure_end();
    void comm_check();
    void activated(const QString &text);

Q_SIGNALS:
    void currentIndexChanged(int index);

private:
    Ui::MainWindow *ui = nullptr;
//    QSignalMapper *idMapper;
    QTimer *camera_timer, *detect_on_timer, *work_on_timer, *third_on_timer, *detect_off_timer, *port_reset_timer, *test_interval_timer, *timer_sec, *hub_port_delay_timer, *comm_polling_timer;
    QImage qt_image;
    relay_seed_ddl *measure_relay;

    void measurement_timer_setup();
    void measurement_ui_setup();
    void system_info_setup();
    void ui_set_measurement_start();
    void ui_set_measurement_stop();
    void ui_set_measurement_pause();
    void ui_set_comm_start();
    void ui_set_comm_stop();
    void makeProgressView();
    void makeDownloadCompleteView(QJsonArray datalist);
    QString parseTime(QString timevalue);

    bool bAutoSetSN;    //각 단말의 연결 상태를 위한 필드
    QHash<QString, QVariant> m_settings;

    bool isEnableBLE;

    void EnableBLEControls(bool value);
    bool isCaresensC(); //V176.110.x.x
    bool checkProtocol();
    void doCommands(QList<Sp::ProtocolCommand> list);
    void InsertListLog(QString str);
    void InsertListStateLog(int state, QString str);
    void ClearListLog();
    bool isOtgModeVisible(); //CareSens N Premier BLE – V89.110.x.x, CareSens N(N-ISO) – V39.200.x.x, CareSens N(N-ISO) Notch – V129.100.x.x

    QString board_info = NULL, usb_port_info=NULL;
    string do_console_command_get_result (char* command);

    int m_logcount;
    quint8 board_version = 0x0;

};

#endif // MAINWINDOW_H
