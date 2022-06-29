#include "control.h"

control::control(QObject *parent) : QObject(parent)
{
    for (quint8 thread_count=0; thread_count<5; thread_count++)
    {
        m_ch[thread_count] = new measurement(thread_count+1);
        m_pThread[thread_count] = new QThread(this);
        m_ch[thread_count]->moveToThread(m_pThread[thread_count]);
        connect(m_pThread[thread_count], &QThread::finished, m_ch[thread_count], &QObject::deleteLater);
        m_pThread[thread_count]->start();
    }
}

void control::delay_mSec(unsigned int msec)
{
    QEventLoop loop;
    QTimer::singleShot(msec, &loop, SLOT(quit()));
    loop.exec();
}

control::~control()
{
    qDebug()<<"QThread deleted";

//    delete m_ch[2];
//    delete m_pThread[2];
}
