#ifndef TCPSOCKETRW_H
#define TCPSOCKETRW_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QtNetwork>
#include <QMap>
#include "commondefinition.h"

QT_BEGIN_NAMESPACE

QT_END_NAMESPACE

class TcpSocketRW : public QObject
{
    Q_OBJECT
public:
        explicit TcpSocketRW(QString ,QObject *parent = nullptr);
        ~TcpSocketRW();

    enum socket_status
    {
        ERROR_SOCKET_CREATE =1,
        ERROR_BIND,
        ERROR_CONNECT_HOST,
        ERROR_DISCONNECT_FROM_HOST,
        ERROR_DISCONNECT_FROM_METER,
        ERROR_SOCKET_EXSIST
    };

    enum socket_command
    {
        COMMAND_INST_CHECK = 0x10,
        COMMAND_INST_OPEN = 0x11,
        COMMAND_INST_SETUP = 0x12,
        COMMAND_CREATE_CSV = 0x13,
        COMMAND_INST_START = 0x14,
        COMMAND_INST_STOP = 0x15,
    };

    enum socket_response
    {
        RESPONSE_SOCKET_CONNECTED_SUCCESS = 0x80,

        RESPONSE_INST_CHECK_SUCCESS = 0x20,
        RESPONSE_INST_OPEN_SUCCESS =  0x21,
        RESPONSE_INST_SETUP_SUCCESS = 0x22,
        RESPONSE_CREATE_CSV_SUCCESS = 0x23,
        RESPONSE_INST_START_SUCCESS = 0x24,
        RESPONSE_INST_STOP_SUCCESS = 0x25,

        RESPONSE_INST_CHECK_FAIL = 0x30,
        RESPONSE_INST_OPEN_FAIL = 0x31,
        RESPONSE_INST_SETUP_FAIL = 0x32,
        RESPONSE_CREATE_CVS_FAIL = 0x33,
        RESPONSE_INST_START_FAIL = 0x34,
        RESPONSE_INST_STOP_FAIL = 0x35,
        RESPONSE_TIMEOUT = 0x60,
        RESPONSE_UNKNOWN_ERROR = 0xFF
    };

signals:
    void sig_read_from_socket(const QByteArray &data);
    void sig_doconnect();
    void sig_connetion_status(socket_status);

    /* DAQ970A command/response */
    void send_socket_command(const QByteArray &data);
// void send_sock_command_tmp();
    void read_socket_response(QString);

public slots:
    void doConnect();
    void connected();
    void disconnectedfromHost();
    void disconnectedfromMeter();
    void removeSocket(bool);
    void writedata(const QByteArray &data);
    void bytesWritten(qint64 bytes);
//  void bytesWritten();
    void readyRead();
    void manage_connection(socket_status);

private:

    QByteArray m_readData;
    QTimer *m_timer_doconnect;

    /*TCP Socket Create*/
    QTcpSocket *qsocket;
    QString Server_ip;

};

#endif // TCPSOCKETRW_H
