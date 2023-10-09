#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <QTimer>
#include <QImage>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "sys_cmd_resp.h"

QT_BEGIN_NAMESPACE

using namespace std;
using namespace cv;

QT_END_NAMESPACE

class camera : public QObject
{
    Q_OBJECT
public:
    explicit camera(QObject *parent = nullptr);

signals:
    void sig_preview_image_update(QImage);

public slots:
    void operation(sys_cmd_resp *);
    void update_image();

private:

    void preview_start();
    void preview_stop();

    QTimer *camera_timer;
    VideoCapture cap;
    Mat frame;
    QImage qt_image;
};

#endif // CAMERA_H
