#ifndef SYS_CMD_RESP_H
#define SYS_CMD_RESP_H

#include <QObject>
#include <QMap>

class sys_cmd_resp : public QObject
{
    Q_OBJECT
public:
    explicit sys_cmd_resp(QObject *parent = nullptr);

    enum comm_cmd {
                     CMD_COMM_OPEN =0x10, CMD_COMM_CLOSE, CMD_COMM_BGMS_CHECK, CMD_COMM_READ_SERIAL, CMD_COMM_SET_TIME,\
                     CMD_COMM_GET_TIME, CMD_COMM_GET_STORED_VALUE_COUNT, CMD_COMM_DOWNLOAD, CMD_COMM_MEM_DELETE, CMD_CAMERA_START, CMD_CAMERA_STOP,CMD_COMM_UNKNOWN=0x1f
                 };Q_ENUM(comm_cmd)
    enum comm_resp {
                     RESP_COMM_PORT_OPEN_SUCCESS =0x20, RESP_COMM_PORT_CLOSE_SUCCESS, RESP_COMM_BGMS_CHECK_SUCCESS, RESP_COMM_READ_SERIAL_SUCCESS, RESP_COMM_SET_TIME_SUCCESS, RESP_COMM_GET_TIME_SUCCESS, RESP_COMM_GET_STORED_VALUE_COUNT_SUCCESS, RESP_COMM_MEM_DELETE_SUCCESS, RESP_COMM_DOWNLOAD_SUCCESS,\
                     RESP_USB_MASS_STORAGE_LIST, RESP_USB_MASS_STORAGE_MOUNT_SUCCESS,\
                     RESP_COMM_PORT_OPEN_FAIL=0x40, RESP_COMM_PORT_CLOSE_FAIL, RESP_COMM_BGMS_CHECK_FAIL, RESP_COMM_READ_SERIAL_FAIL, RESP_COMM_GET_TIME_FAIL, RESP_COMM_SET_TIME_FAIL, RESP_COMM_GET_STORED_VALUE_COUNT_FAIL, RESP_COMM_DOWNLOAD_FAIL,\
                     RESP_COMM_MEM_DELETE_FAIL, RESP_COMM_BGMS_RESP_FAIL, RESP_USB_MASS_STORAGE_MOUNT_ERROR, RESP_COMM_UNKNOWN=0xff
                 };Q_ENUM(comm_resp)    

    enum protocol_type {VERSION_01=0x50, VERSION_02, VERSION_03, VERSION_UNKNOWN=0x5f};

    enum bgms_data_download_cmd{DOWNLOAD_READ_SERIAL, DOWNLOAD_READ_COUNT, DOWLOAD_CONTINUE, DOWNLOAD_DONE, DOWNLOAD_UNKNOWN};Q_ENUM(bgms_data_download_cmd)

    QString current_bgms_time, serial_number;
    quint16 measured_result;

    comm_cmd m_comm_cmd = CMD_COMM_UNKNOWN;
    comm_cmd m_comm_status = CMD_COMM_UNKNOWN;
    comm_cmd m_comm_status_sub = CMD_COMM_UNKNOWN;
    comm_resp m_comm_resp = RESP_COMM_UNKNOWN;
    protocol_type m_protocol_type = VERSION_UNKNOWN;

    bgms_data_download_cmd m_bgms_data_download_cmd = DOWNLOAD_UNKNOWN;

    QMap <QString, QString>mass_storage_device_info;

signals:

public slots:
};

#endif // SYS_CMD_RESP_H
