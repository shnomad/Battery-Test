#ifndef SERIALPROTOCOL
#define SERIALPROTOCOL

#define kSTX    (0x02)
#define kETX    (0x03)

class Sp {

public:
    enum CommProtocolType
    {
        CommProtocolUnknown,
        CommProtocol1,
        CommProtocol2,
        CommProtocol3,
        CommProtocol4,
        CommProtocol5,
        CommProtocol6,
        CommProtocol7,
        CommProtocol8,
        CommProtocol9
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
        DefaultBGMS,
        ColorBGMS,
        BleBGMS,
        VoiceBGMS
    };

    enum ProtocolCommand {
        ProtocolCommandNone,
        GluecoseResultDataTx,           //GLUC  0x474C5543  1
        GluecoseResultDataTxExpanded,   //GLUE  0x474C5545  2
        CurrentIndexOfGluecose,         //NCOT  0x4E434F54  3
        ReadSerialNumber,               //RSNB  0x52534E42  4
        WriteSerialNumber,              //WSNB  0x57534E42  5
        ReadTimeInformation,            //RTIM  0x5254494D  6
        WriteTimeInformation,           //WTIM  0x5754494D  7
        SaveData,                       //SVDT  0x53564454  8
        DeleteData,                     //DELD  0x44454C44  9
        WriteAnimalType,                                //10
        ReadAnimalType,                                 //11
        Unlock,
        ReadSavePoint,        
        ProtocolCommandMax,
		ReadBleData,
        ReadQcData,
        ReadTemperature,
        SetHour12H,
        SetHour24H,
        SetHour12H_color,
        SetHour24H_color,
        SetMultichart,
        SetCSMode,        
        SetJIGMode,
        SetBLELog,
        SetReset,
        SetDebugOFF,        
        SetDebugON,
        WriteDateforamt,
        WriteYear,
        WriteMemoryCount,
        WriteBLEWay,
        WriteUnitTemp,
        WriteSetHypo,
        ReadSetHypo,
        WriteBLE,
        ReadBLE,
        WriteCodingMode,
        ReadCodingMode,
        WriteOtgMode,
        ReadOtgMode,
        ChangeBLEMode,                          //CBLE 43424c45
        ChangeBLEMode_EXT,                      //DBLE 44424c45
        TryChangeBLEMode,
        Write_Temperature_Range_Low,
        Write_Temperature_Range_High,
        Write_Measure_Range_Low,
        Write_Measure_Range_High,
        Write_Hypo_initial,
        Write_Hyper_initial,
        Write_Hypo_on_off,
        Write_Hyper_on_off,
        Write_Buzzer_on_off,
        Write_Strip_TYPE_INFO_1,
        Write_Strip_TYPE_INFO_2,
        Write_Strip_TYPE_INFO_3,
        Write_TI_SetCode,
        Write_AudioMode_Volume,
        Write_Audio_LangCount,
        Write_AudioMode_Volume_TI,
        Write_Audio_LangCount_TI,
        SetAvgDays,
        GetAvgDays,
        Set91,
        Set9A01,
        Set9A02,
        Set9A03,
        SetOnNet,
        SetOffNet,
        SetQCFlag_1,
        SetQCFlag_2,
        SetQCFlag_3,
        WriteAESKey,
        ReadAESKey,                         //     524b5931
        GetGlucoseDataFlag,
        SetGlucoseDataFlag,
        SetJIG_color,
        ReadAPN,                            //RAPN 0x5241504E
        WriteAPN,                           //WAPN 0x5741504E
        WriteServerAddress,
        ReadServerAddress,
        WriteDeviceToken,
        ReadDeviceToken,
        WriteServiceName,
        ReadServiceName,
        ReadIMEI,
        ReadICCID,
        ReadModuleSoftwareVer,
        ReadIMSI,
        ReadSktSN,
        WriteSktSN,
        ReadRegistrationURL,
        WriteRegistrationURL,
        CommandPacketVerifyError,                //not implemented(?)
        CommunicationTimeout,                       //TOUT  0x544F5554
        HeaderPacketVerifyError,                    //HEAD  0x48454144
        SizeOfPacketVerifyError,                    //SIZE  0x53495A45
        CRCPacketVerifyError,                       //ECRC  0x45435243
    };

    enum DownloadStartFailReason {
        StartingSuccess,
        WaitingResponse,
        InternalError,
    };

};

#endif // SERIALPROTOCOL
