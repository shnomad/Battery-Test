#include "tcpsocketrw.h"

//TcpSocketRW::TcpSocketRW(QObject *parent) : QObject(parent)
TcpSocketRW::TcpSocketRW(QString ip_address ,QObject *parent) : QObject(parent)
{
     Log() << "TCPSocketRW Started";

     /*Get Local Host Ip address*/
     Server_ip = ip_address;

     /*connecting to Host Timer settings */
     m_timer_doconnect = new QTimer;
     m_timer_doconnect->setSingleShot(true);
     m_timer_doconnect->setInterval(100);

     connect(this, SIGNAL(sig_connetion_status(socket_status)), this, SLOT(manage_connection(socket_status)));
     connect(m_timer_doconnect, SIGNAL(timeout()), this, SLOT(doConnect()));

     m_timer_doconnect->start();
}

TcpSocketRW::~TcpSocketRW()
{

}

void TcpSocketRW::doConnect()
{
     Log() <<"TCP Socket doConnet";

     qsocket = new QTcpSocket(this);

     connect(qsocket, SIGNAL(connected()),this, SLOT(connected()));
     connect(qsocket, SIGNAL(disconnected()),this, SLOT(disconnectedfromHost()));
//   connect(qsocket, SIGNAL(bytesWritten(qint64)),this, SLOT(bytesWritten(qint64)));
     connect(qsocket, SIGNAL(readyRead()),this, SLOT(readyRead()));

     qsocket->connectToHost(QHostAddress(Server_ip),60010);

     if(!qsocket->waitForConnected(1000))
     {
         Log()<<"Socket connect Fail";

         emit sig_connetion_status(ERROR_CONNECT_HOST);
     }
     else
     {
        Log()<<"Socket connect Success";
     }
}

void TcpSocketRW::connected()
{
      Log() << "Socket server connected";
}

void TcpSocketRW::disconnectedfromHost()
{

}

void TcpSocketRW::disconnectedfromMeter()
{
  //  qDebug() << "disconnected from METER";
}

void TcpSocketRW::removeSocket(bool qtsock)
{

}

void TcpSocketRW::writedata(const QByteArray &data)
{    
    if(qsocket->state() == QAbstractSocket::ConnectedState)
    {
        qsocket->write(data);
    }
}

//void TcpSocketRW::bytesWritten()
void TcpSocketRW::bytesWritten(qint64 bytes)
{
    Log() <<" bytes written from UI Main"<< bytes;
    qsocket->write(QByteArray::number(TcpSocketRW::socket_command::COMMAND_INST_START));
}

void TcpSocketRW::readyRead()
{
  // qDebug() << "reading : " << m_readData.toHex();

    QByteArray resp, cmd;

    resp = qsocket->readAll();

    switch(resp[0])
    {

        case  RESPONSE_SOCKET_CONNECTED_SUCCESS: //0x80

            qsocket->write(cmd.append(QByteArray::number(COMMAND_INST_CHECK)));
            emit read_socket_response("RESPONSE_SOCKET_CONNECTED_SUCCESS");

            break;

        case   RESPONSE_INST_CHECK_SUCCESS:       //0x20

             qsocket->write(cmd.append(QByteArray::number(COMMAND_INST_OPEN)));
             emit read_socket_response("RESPONSE_INST_CHECK_SUCCESS");

            break;

        case   RESPONSE_INST_OPEN_SUCCESS:        //0x21

            qsocket->write(cmd.append(QByteArray::number(COMMAND_INST_SETUP)));
             emit read_socket_response("RESPONSE_INST_OPEN_SUCCESS");

            break;

        case   RESPONSE_INST_SETUP_SUCCESS:       //0x22

             qsocket->write(cmd.append(QByteArray::number(COMMAND_CREATE_CSV)));
             emit read_socket_response("RESPONSE_INST_SETUP_SUCCESS");

            break;

        case   RESPONSE_CREATE_CSV_SUCCESS:       //0x23

//           qsocket->write(cmd.append(QByteArray::number(COMMAND_INST_START)));
             emit read_socket_response("RESPONSE_CREATE_CSV_SUCCESS");

            break;

        case   RESPONSE_INST_START_SUCCESS:       //0x24

             emit read_socket_response("RESPONSE_INST_START_SUCCESS");

            break;

        case   RESPONSE_INST_STOP_SUCCESS:        //0x25

            emit read_socket_response("RESPONSE_INST_STOP_SUCCESS");

            break;

        case   RESPONSE_INST_CHECK_FAIL:          //0x30
        case   RESPONSE_INST_OPEN_FAIL:           //0x31            
        case   RESPONSE_INST_SETUP_FAIL:          //0x32
        case   RESPONSE_CREATE_CVS_FAIL:          //0x33
        case   RESPONSE_INST_START_FAIL:          //0x34
        case   RESPONSE_INST_STOP_FAIL:           //0x35
        case   RESPONSE_TIMEOUT:                  //0x60
        case   RESPONSE_UNKNOWN_ERROR:            //0xFF

            emit read_socket_response("RESPONSE_INST_ERROR");

            break;
    }

     resp.clear();
     cmd.clear();
}

void TcpSocketRW::manage_connection(socket_status status)
{
     Log() << "manage socket connection";

     switch (status)
      {
          case ERROR_SOCKET_CREATE:
          case ERROR_BIND:
          case ERROR_SOCKET_EXSIST:

          break;

          case ERROR_CONNECT_HOST:
          case ERROR_DISCONNECT_FROM_HOST:
          case ERROR_DISCONNECT_FROM_METER:

          disconnect(qsocket, SIGNAL(connected()),this, SLOT(connected()));
          disconnect(qsocket, SIGNAL(disconnected()),this, SLOT(disconnectedfromHost()));
          disconnect(qsocket, SIGNAL(bytesWritten(qint64)),this, SLOT(bytesWritten(qint64)));
          disconnect(qsocket, SIGNAL(readyRead()),this, SLOT(readyRead()));

          break;

      }

      m_timer_doconnect->setInterval(3000);
      m_timer_doconnect->start();

      emit sig_doconnect();
}
