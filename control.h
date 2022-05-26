#ifndef CONTROL_H
#define CONTROL_H

#include <QObject>
#include <QThread>
#include "measurement.h"

//class measurement;

class control : public QObject
{
    Q_OBJECT
public:
    explicit control(QObject *parent = nullptr);
    ~control();

    measurement *m_ch[5];
    QThread *m_pThread[5];

signals:

public slots:
};

#endif // CONTROL_H
