#ifndef CLI_MONITOR_H
#define CLI_MONITOR_H

#include <QObject>
#include <QProcess>
#include "sys_cmd_resp.h"

class CLI_monitor : public QObject
{
    Q_OBJECT
public:
    explicit CLI_monitor(QObject *parent = nullptr);

    ~CLI_monitor();
    void sendCommand(QString command);

signals:

void sig_result_mass_storage_attach(sys_cmd_resp::comm_resp, QString);

public slots:

void readyReadStandardOutput();
void readyReadStandardError();

private:
    QProcess mProcess;
    sys_cmd_resp *resp_mass_storage_status;

};

#endif // CLI_MONITOR_H
