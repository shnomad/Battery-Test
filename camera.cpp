#include "camera.h"
#include <QProcess>

camera::camera(QObject *parent) : QObject(parent)
{
    camera_timer = new QTimer(this);
}

void camera::preview_start()
{
    cap.open(0);

       if(!cap.isOpened())
       {

       }
       else
       {
           system("v4l2-ctl -d /dev/video0 --set-ctrl=exposure_auto=1");

           system("v4l2-ctl -d /dev/video0 --set-ctrl=focus_auto=1");

           connect(camera_timer, SIGNAL(timeout()), this, SLOT(update_image()));
           camera_timer->start(50);
       }
}

void camera::preview_stop()
{

      disconnect(camera_timer, SIGNAL(timeout()), this, SLOT(update_image()));

      cap.release();
      Mat image = Mat::zeros(frame.size(), CV_8UC3);
      qt_image = QImage((const unsigned char*)(image.data), image.cols, image.rows, QImage::Format_RGB888);

 //   ui->preview->setPixmap(QPixmap::fromImage(qt_image));
}

void camera::update_image()
{
        cap >> frame;
        cvtColor(frame, frame, COLOR_BGR2RGB);
/*
        if(ui->capture->isChecked())
        {
             QDateTime Current_Time = QDateTime::currentDateTime();
             QString filename = "/home/pi/capture_file/" + Current_Time.toString("yyyyMMddhhmmsszzz") + ".jpg";
             imwrite(filename.toStdString(),frame);
        }
*/
        qt_image = QImage((const unsigned char*)(frame.data), frame.cols, frame.rows, QImage::Format_RGB888);

        emit sig_preview_image_update(qt_image);
}

void camera::operation(sys_cmd_resp *cmd_from_host)
{
    switch (cmd_from_host->m_comm_cmd)
    {
        case sys_cmd_resp::CMD_CAMERA_START:
                preview_start();
            break;

        case sys_cmd_resp::CMD_CAMERA_STOP:
                preview_stop();
            break;
    }

}

