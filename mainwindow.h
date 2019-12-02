#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

QT_BEGIN_NAMESPACE

using namespace std;

namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

//class relay_seed;
class relay_seed_ddl;
class QElapsedTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    quint16 measure_coount=0;
    quint16 measure_capacity=0;
    quint16 changed_interval=0;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    quint32 detect_on_time = 0;
    quint32 work_on_time = 5000;
    quint32 third_on_time = 6000;
    quint32 detect_off_time = 14000;
    quint32 port_reset_time = 18000;
    quint32 bluetooth_time = 0;

private slots:
    void on_test_start_clicked();
    void on_test_stop_clicked();
    void measurement();
    void detect_on();
    void work_on();
    void third_on();
    void detect_off();
    void measure_port_reset();
    void measure_count_check();
    void comm_connect();
    void comm_port_reset();
    void on_quit_clicked();
    void on_device_open_clicked();
    void on_device_close_clicked();
    void on_times_valueChanged(const QString &arg1);
    void on_sec_valueChanged(const QString &arg1);

signals:
    void measure_start();
    void measure_check();
    void measure_end();
    void comm_check();

private:
    Ui::MainWindow *ui = nullptr;    
    QTimer *camera_timer, *detect_on_timer, *work_on_timer, *third_on_timer, *detect_off_timer, *port_reset_timer;
    QImage qt_image;
    relay_seed_ddl *measure_relay;
    QElapsedTimer *mesure_time_check;
};

#endif // MAINWINDOW_H
