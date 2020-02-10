#ifndef HIDTESTER_H
#define HIDTESTER_H

#include <QObject>
#include <QTimer>
#include "hidport.h"
#include "serialprotocolabstract.h"

class HIDTester : public QObject
{
    Q_OBJECT

public:
    HIDPort *selecthidport;
    Sp::CommProtocolType protocol;

    explicit HIDTester(HIDPort *hidport ,QObject *parent = 0);
    ~HIDTester();

    void check();
    bool CheckState();
    void unsetCheckState();

signals:
    void timeoutError(HIDTester *sender);
    void responseUnknown(HIDTester *sender);
    void responseSuccess(HIDTester *sender);

public slots:
     void start();
     void readResponse();     
     void checkResponse();

protected:
    void timerEvent(QTimerEvent *event);

private:
     int timeoutTimerID;
     bool isCheckState;
     QByteArray receivedData;
};

#endif // HIDTESTER_H
