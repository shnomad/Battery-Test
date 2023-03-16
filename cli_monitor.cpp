#include "cli_monitor.h"
#include "commondefinition.h"

CLI_monitor::CLI_monitor(QObject *parent) : QObject(parent)
{
    connect(&mProcess,SIGNAL(readyReadStandardOutput()),this,SLOT(readyReadStandardOutput()));
    connect(&mProcess,SIGNAL(readyReadStandardError()),this,SLOT(readyReadStandardError()));
}

void CLI_monitor::sendCommand(QString command)
{
    mProcess.start(command);
    mProcess.waitForFinished(-1);
}

void CLI_monitor::readyReadStandardOutput()
{
    QString result = QString::fromStdString(mProcess.readAllStandardOutput().toStdString());

    Log()<<result;

     emit sig_result_mass_storage_attach(sys_cmd_resp::RESP_USB_MASS_STORAGE_MOUNT_SUCCESS, result);
}

void CLI_monitor::readyReadStandardError()
{
    QString result = QString::fromStdString(mProcess.readAllStandardError().toStdString());

    Log()<<result;

    emit sig_result_mass_storage_attach(sys_cmd_resp::RESP_USB_MASS_STORAGE_MOUNT_ERROR, result);
}

CLI_monitor::~CLI_monitor()
{

}
