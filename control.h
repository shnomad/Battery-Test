#ifndef CONTROL_H
#define CONTROL_H

#include <QObject>
#include "measurement.h"

class measurement;

class control : public QObject
{
    Q_OBJECT
public:
    explicit control(QObject *parent = nullptr);
    ~control();

signals:

public slots:


private:
    measurement *m_ch[2];
    QThread *m_pThread[5];

};

#endif // CONTROL_H
