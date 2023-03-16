#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QThread>
#include "measurement.h"
#include "measurement_param.h"
#include "commondefinition.h"
#include "control.h"
#include <QStringList>
#include "sys_cmd_resp.h"
#include "hid_uart_comm.h"
#include "udev_monitor_usb.h"
#include "cli_monitor.h"

QT_BEGIN_NAMESPACE

using namespace std;

namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class measurement_param;
class measurement;
class control;
class TcpSocketRW;

#define RASPBERRY_PI3_B         0x0
#define RASPBERRY_PI3_B_PLUS    0x1
#define RASPBERRY_PI_UNKNOWN    0x2

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();    

    enum meter_channel {CH_1=0x1, CH_2=0x2, CH_3=0x3, CH_4=0x4};
    enum comm_set {SET_CLOSE=0x0, SET_OPEN};

private slots:
    void UpdateTime();
    void ui_action_status_ch1(QString);
    void ui_test_count_ch1(int);
    void ui_interval_time_update_ch1(int);

    void ui_action_status_ch2(QString);
    void ui_test_count_ch2(int);
    void ui_interval_time_update_ch2(int);

    void ui_action_status_ch3(QString);
    void ui_test_count_ch3(int);
    void ui_interval_time_update_ch3(int);

    void ui_action_status_ch4(QString);
    void ui_test_count_ch4(int);
    void ui_interval_time_update_ch4(int);

    void ui_action_status_ch5(QString);
    void ui_test_count_ch5(int);
    void ui_interval_time_update_ch5(int);

    void ui_bgms_comm_ch_1_response(sys_cmd_resp *);
    void ui_bgms_comm_ch_2_response(sys_cmd_resp *);
    void ui_bgms_comm_ch_3_response(sys_cmd_resp *);

    void dmm_working_status(QString);

private Q_SLOTS:

    void currentMeterIndexChanged(int index);

    void on_test_start_ch1_clicked();
    void on_test_pause_ch1_clicked();
    void on_test_stop_ch1_clicked();

    void on_meter_type_ch1_activated(const QString &arg1);
    void on_measure_count_ch1_valueChanged(const QString &arg1);
    void on_sec_startdelay_ch1_valueChanged(const QString &arg1);
    void on_sec_detoffdelay_ch1_valueChanged(const QString &arg1);
    void on_sec_interval_ch1_valueChanged(const QString &arg1);

    void on_test_start_ch2_clicked();
    void on_test_pause_ch2_clicked();
    void on_test_stop_ch2_clicked();

    void on_meter_type_ch2_activated(const QString &arg1);
    void on_sec_startdelay_ch2_valueChanged(const QString &arg1);
    void on_sec_detoffdelay_ch2_valueChanged(const QString &arg1);
    void on_sec_interval_ch2_valueChanged(const QString &arg1);
    void on_measure_count_ch2_valueChanged(const QString &arg1);     

    void on_test_start_ch3_clicked();
    void on_test_pause_ch3_clicked();
    void on_test_stop_ch3_clicked();

    void on_meter_type_ch3_activated(const QString &arg1);
    void on_measure_count_ch3_valueChanged(const QString &arg1);
    void on_sec_startdelay_ch3_valueChanged(const QString &arg1);
    void on_sec_detoffdelay_ch3_valueChanged(const QString &arg1);
    void on_sec_interval_ch3_valueChanged(const QString &arg1);

    void on_test_start_ch4_clicked();
    void on_test_pause_ch4_clicked();
    void on_test_stop_ch4_clicked();

    void on_meter_type_ch4_activated(const QString &arg1);
    void on_measure_count_ch4_valueChanged(const QString &arg1);
    void on_sec_startdelay_ch4_valueChanged(const QString &arg1);
    void on_sec_detoffdelay_ch4_valueChanged(const QString &arg1);
    void on_sec_interval_ch4_valueChanged(const QString &arg1);

    void on_test_start_ch5_clicked();
    void on_test_pause_ch5_clicked();
    void on_test_stop_ch5_clicked();

    void on_meter_type_ch5_activated(const QString &arg1);
    void on_measure_count_ch5_valueChanged(const QString &arg1);
    void on_sec_startdelay_ch5_valueChanged(const QString &arg1);
    void on_sec_detoffdelay_ch5_valueChanged(const QString &arg1);
    void on_sec_interval_ch5_valueChanged(const QString &arg1);

    void on_download_ch2_clicked();
    void on_reboot_clicked();
    void on_quit_clicked();
    void on_dmm_capture_stateChanged(int arg1);

    void on_device_open_ch1_clicked();
    void on_device_open_ch2_clicked();
    void on_device_open_ch3_clicked();

    void on_device_close_ch1_clicked();
    void on_device_close_ch2_clicked();
    void on_device_close_ch3_clicked();

    void on_time_sync_ch1_clicked();

    void on_mem_delete_ch1_clicked();

    void on_download_ch1_clicked();

    void on_audo_download_ch1_stateChanged(int arg1);

signals:
    void measure_setup_ch1(measurement_param);
    void measure_start_ch1();
    void measure_stop_ch1();
    void measure_pause_ch1();

    void measure_setup_ch2(measurement_param);
    void measure_start_ch2();
    void measure_stop_ch2();
    void measure_pause_ch2();

    void measure_setup_ch3(measurement_param);
    void measure_start_ch3();
    void measure_stop_ch3();
    void measure_pause_ch3();

    void measure_setup_ch4(measurement_param);
    void measure_start_ch4();
    void measure_stop_ch4();
    void measure_pause_ch4();

    void measure_setup_ch5(measurement_param);
    void measure_start_ch5();
    void measure_stop_ch5();
    void measure_pause_ch5();

    void update_action(QString);
    void update_test_count(int);
    void update_interval_time(int);
    void update_dmm_status(QString);

    void sig_bgms_comm_cmd(sys_cmd_resp *);

Q_SIGNALS:
    void currentIndexChanged(int index);

private:

    Ui::MainWindow *ui = nullptr;

    void ui_init_measurement();
    void ui_create_measurement();
    void ui_system_info_setup();

    void ui_set_measurement_start_ch1();
    void ui_set_measurement_stop_ch1();
    void ui_set_measurement_pause_ch1();

    void ui_set_measurement_start_ch2();
    void ui_set_measurement_stop_ch2();
    void ui_set_measurement_pause_ch2();

    void ui_set_measurement_start_ch3();
    void ui_set_measurement_stop_ch3();
    void ui_set_measurement_pause_ch3();

    void ui_set_measurement_start_ch4();
    void ui_set_measurement_stop_ch4();
    void ui_set_measurement_pause_ch4();

    void ui_set_measurement_start_ch5();
    void ui_set_measurement_stop_ch5();
    void ui_set_measurement_pause_ch5();

    void ui_set_comm_open_close(meter_channel, comm_set);

    string do_console_command_get_result (char* command);

    QTimer *timer_sec;
    QString board_info = nullptr;
    quint8 board_version = 0x0;
    measurement_param m_test_param_tmp{}, m_test_param_ch1{}, m_test_param_ch2{}, m_test_param_ch3{}, m_test_param_ch4{}, m_test_param_ch5{};

   /*measurement thread control*/
    control *m_control;
    bool system_init_done=false;
    bool meter_type_index_selected_measure_count = false, meter_type_index_selected_start_delay = false, meter_type_index_selected_detoff_delay = false, meter_type_index_selected_interval=false;

    QString server_ip;

    /*simplized the code*/
    const QStringList meter_types = {"BGMS BASIC", "BGMS BLE", "BGMS VOICE", "BGMS VOICE_BLE", "KETONE BASIC", "KETONE BLE"};
    hid_uart_comm *m_hid_uart_comm[3];
    QThread *comm_Thread[3];

    QMap<quint8, QString> comm_response_msg;

    udev_monitor_usb *usb_mass_detect;
    CLI_monitor *m_console_command;
    sys_cmd_resp *comm_cmd;

    /*usb mass storage handling*/
    QString dev_path;
};

#endif // MAINWINDOW_H
