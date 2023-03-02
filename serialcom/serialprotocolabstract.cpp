#include "serialprotocolabstract.h"
//#include "serialcomm.h"

//SerialProtocolAbstract::SerialProtocolAbstract(SerialComm *serialComm, QObject *parent) : QObject(parent), comm(serialComm), currentState(Sp::Idle),lastCommand(Sp::ProtocolCommandNone)
SerialProtocolAbstract::SerialProtocolAbstract(QObject *parent): QObject(parent)
{

}

SerialProtocolAbstract::~SerialProtocolAbstract()
{

}

Sp::ProtocolState SerialProtocolAbstract::state()
{
    return currentState;
}

bool SerialProtocolAbstract::isIdle()
{
    return (currentState == Sp::Idle);
}

ushort SerialProtocolAbstract::calcCrc(const QByteArray &array)
{
    Q_ASSERT(array.count());

    ushort crc = 0xffff;
    foreach (uchar c, array) {
        crc = (crc >> 8)|(crc << 8);
        crc ^= (ushort)c;
        crc ^= ((crc & 0xff) >> 4);
        crc ^= (crc << 12);
        crc ^= ((crc & 0xff) << 5);
    }
    return crc;
}

bool SerialProtocolAbstract::isEqualCrc(const QByteArray &array, ushort crc)
{
    ushort acrc = calcCrc(array);
    return (crc == acrc);
}

Sp::CommProtocolType SerialProtocolAbstract::checkProtocol(const QByteArray &array)
{
    if(array.count() >= 3) {
        if(array.at(1) == 0x10 && array.at(2) == 0x20) {
            return Sp::CommProtocol1;
        } else if(array.at(1) == 0x1e && array.at(2) == 0x2e) {
            return Sp::CommProtocol3;
        }
        else if(array.at(1) == 0x1f && array.at(2) == 0x2f) {
            return Sp::CommProtocol2;
        }
    }
    return Sp::CommProtocolUnknown;
}

void SerialProtocolAbstract::parseReceivedData(QByteArray rcvPacket)
{
    Q_UNUSED(rcvPacket);
}
