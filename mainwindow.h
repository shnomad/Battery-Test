#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QThread>
#include "measurement.h"
#include "measurement_param.h"
#include "commondefinition.h"
#include "control.h"

QT_BEGIN_NAMESPACE

using namespace std;

namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class measurement_param;
class measurement;
class control;

#define RASPBERRY_PI3_B         0x0
#define RASPBERRY_PI3_B_PLUS    0x1
#define RASPBERRY_PI_UNKNOWN    0x2

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();    

    enum meter_channel {CH_1=0x1, CH_2=0x2};

private slots:

    void UpdateTime();
    void ui_action_status_ch1(QString);
    void ui_test_count_ch1(int);
    void ui_action_status_ch2(QString);
    void ui_test_count_ch2(int);

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
    void on_download_ch2_clicked();

    void on_reboot_clicked();
    void on_quit_clicked();

signals:
    void measure_setup_ch1(measurement_param);
    void measure_start_ch1();
    void measure_stop_ch1();
    void measure_pause_ch1();

    void measure_setup_ch2(measurement_param);
    void measure_start_ch2();
    void measure_stop_ch2();
    void measure_pause_ch2();

    void update_action(QString);
    void update_test_count(int);

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
    string do_console_command_get_result (char* command);

    QTimer *timer_sec;
    QString board_info = NULL;
    quint8 board_version = 0x0;
    measurement_param m_test_param_tmp{}, m_test_param_ch1{}, m_test_param_ch2{};

   /*measurement thread control*/
    control *m_control;

   /*measurement thread */
//  measurement *m_ch[2];
//  QThread *m_pThread[2];
};

#endif // MAINWINDOW_H
