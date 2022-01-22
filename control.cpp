
#include "control.h"

control::control(QObject *parent) : QObject(parent)
{
#if 0
    m_ch[0] = new measurement(0x1);
    m_ch[1] = new measurement(0x2);

    m_pThread[0] = new QThread(this);
    m_pThread[1] = new QThread(this);

    m_ch[0]->moveToThread(m_pThread[0]);
    m_ch[1]->moveToThread(m_pThread[1]);

    connect(m_pThread[0], &QThread::finished, m_ch[0], &QObject::deleteLater);
    connect(m_pThread[1], &QThread::finished, m_ch[1], &QObject::deleteLater);

    m_pThread[0]->start();
    m_pThread[1]->start();
#endif

}

control::~control()
{
    qDebug()<<"QThread deleted";

//    delete m_ch[2];
//    delete m_pThread[2];
}
