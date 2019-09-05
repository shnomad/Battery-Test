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

class seed_relay;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    quint16 measure_coount=0;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_device_check_clicked();
    void on_test_start_clicked();
    void on_camera_start_clicked();
    void on_camera_stop_clicked();
    void update_camera();
    void measurement();
    void detect_on();
    void work_on();
    void third_on();
    void detect_off();
    void port_reset();
    void measure_count_check();

    void on_test_stop_clicked();

signals:
    void measure_start();
    void measure_check();

private:
    Ui::MainWindow *ui = nullptr;    
    QTimer *camera_timer;
    VideoCapture cap;
    Mat frame;
    QImage qt_image;

    seed_relay *relay;
};

#endif // MAINWINDOW_H
