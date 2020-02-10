#ifndef STMHIDTESTER_H
#define STMHIDTESTER_H

#include <QObject>
#include <QTimer>
#include "stmhidport.h"
#include "serialprotocolabstract.h"
#include "commondefinition.h"

class STMHIDTester : public QObject
{
    Q_OBJECT

public:
    STMHIDPort *selectstmport;
    Sp::CommProtocolType protocol;

    explicit STMHIDTester(STMHIDPort *stmport ,QObject *parent = 0);
    ~STMHIDTester();

    void check();
    bool CheckState();
    void unsetCheckState();

signals:
    void timeoutError(STMHIDTester *sender);
    void responseUnknown(STMHIDTester *sender);
    void responseSuccess(STMHIDTester *sender);
    void textMessageSignal(QString text);

public slots:
     void start();
     void readResponse();
     void checkResponse();
     void timeoutErrorFromPort();

protected:
    void timerEvent(QTimerEvent *event);

private:
     int timeoutTimerID;
     bool isCheckState;
     QByteArray receivedData;
};

#endif // STMHIDTESTER_H
