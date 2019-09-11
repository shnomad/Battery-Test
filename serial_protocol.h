#ifndef SERIAL_PROTOCOL_H
#define SERIAL_PROTOCOL_H

#define kSTX    (0x02)
#define kETX    (0x03)

class Sp {

public:

    enum CommProtocolType
    {
        CommProtocolUnknown,
        CommProtocol1,
        CommProtocol3,
    };

    enum ProtocolState
    {
        Idle,
        RequestWaiting,
        GluecoseDownloading,
        RequestComplete,
        AnError
    };

    enum MeterModelType
    {
        DefaultMeter,
        ColorMeter,
    };

    enum ProtocolCommand
    {
        ProtocolCommandNone,
        GluecoseResultDataTx,           //GLUC  0x474C5543
        GluecoseResultDataTxExpanded,   //GLUE  0x474C5545
        CurrentIndexOfGluecose,         //NCOT  0x4E434F54
        ReadSerialNumber,               //RSNB  0x52534E42
        WriteSerialNumber,              //WSNB  0x57534E42
        ReadTimeInformation,            //RTIM  0x5254494D
        WriteTimeInformation,           //WTIM  0x5754494D
        ReadSavePoint,
        ProtocolCommandMax,
        CommunicationTimeout,
        HeaderPacketVerifyError,
        SizeOfPacketVerifyError,
        CRCPacketVerifyError
    };

    enum DownloadStartFailReason
    {
        StartingSuccess,
        WaitingResponse,
        InternalError,
    };

};

#endif // SERIAL_PROTOCOL_H
