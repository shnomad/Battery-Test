#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

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

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_device_check_clicked();
    void on_test_start_clicked();
    void on_camera_start_clicked();
    void on_camera_stop_clicked();
    void update_camera();

private:
    Ui::MainWindow *ui = nullptr;    
    QTimer *timer;
    VideoCapture cap;
    Mat frame;
    QImage qt_image;
    seed_relay *relay;
    void PrintMessage(quint8 messageType);

};

#endif // MAINWINDOW_H
