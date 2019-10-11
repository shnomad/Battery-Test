#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

QT_BEGIN_NAMESPACE

using namespace std;
using namespace cv;

namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class relay_seed;
class relay_seed_ddl;
class relay_waveshare;
class QElapsedTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    quint16 measure_coount=0;

public:
    quint8 capture_flag=0x0;
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
//  void on_device_check_clicked();
    void on_test_start_clicked();
    void on_test_stop_clicked();
    void on_camera_start_clicked();
    void on_camera_stop_clicked();
    void update_camera();
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
    void on_Capture_on_clicked();
    void on_Capture_off_clicked();
    void on_device_open_clicked();
    void on_device_close_clicked();

signals:
    void measure_start();
    void measure_check();
    void measure_end();
    void comm_check();

private:
    Ui::MainWindow *ui = nullptr;    
    QTimer *camera_timer, *detect_on_timer, *work_on_timer, *third_on_timer, *detect_off_timer, *port_reset_timer;
    VideoCapture cap;
    Mat frame;
    QImage qt_image;
    relay_waveshare *measure_relay;
    relay_seed_ddl *comm_relay;
    relay_seed *relay_measure_i2c;
    QElapsedTimer *mesure_time_check;
};

#endif // MAINWINDOW_H
