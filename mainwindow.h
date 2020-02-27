#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "commondefinition.h"
#include "serialcom/serialprotocol.h"
#include "serialcom/serialcomm.h"
#include "serialcom/serialprotocol3.h"

QT_BEGIN_NAMESPACE

using namespace std;

namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

//class relay_seed;
class relay_seed_ddl;
class QElapsedTimer;
class SerialProtocolAbstract;
//class usb_comm;
//class bgm_comm_protocol;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    SerialComm *serialComm;
    SerialProtocolAbstract *protocol;
    QString m_qcType;

    quint16 measure_coount=0;
    quint16 measure_capacity=0;
    quint16 changed_interval=0;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    quint32 detect_on_time = 0;
    quint32 work_on_time = 5000;
    quint32 third_on_time =1000; // 6000;
    quint32 detect_off_time = 8000; //14000;
    quint32 port_reset_time = 4000; //18000;
    quint32 bluetooth_time = 0;
    quint32 hub_port_delay_time = 2000;

private slots:
    void on_test_start_clicked();
    void on_test_stop_clicked();
    void measurement();
    void detect_on();
    void work_on();
    void third_on();
    void detect_off();
    void measure_port_reset();
    void measure_count_check();
    void hub_port_open();
    void hub_port_close();
    void hub_port_reset();
    void on_quit_clicked();
    void on_device_open_clicked();
    void on_device_close_clicked();
    void meter_comm_start();
    void meter_comm_end();
    void on_times_valueChanged(const QString &arg1);
    void on_sec_valueChanged(const QString &arg1);
    void UpdateTime();
    void comm_polling_event();          //periodical polling for device
    void comm_polling_event_start();          //periodical polling for device
    void comm_polling_event_stop();          //periodical polling for device

private Q_SLOTS:
    void portReady();
    void connectionError();
    void textMessage(QString text);
    // 다운로드 과정에서 protocol에서 보내오는 시그널과 연결되는 함수
    void timeoutError(Sp::ProtocolCommand command);
    void errorOccurred(Sp::ProtocolCommand command, Sp::ProtocolCommand preCommand);  // 미터에서 보내온 오류
    void errorCrc();                                    // 수신데이터 CRC 오류
    void errorUnresolvedCommand(Sp::ProtocolCommand command);     // 다운로드 과정에서 수행하지 않는 커맨드 패킷 수신
    void packetReceived();
    void finishReadingQcData();
    void finishDoCommands(bool bSuccess, Sp::ProtocolCommand lastcommand);
    void maintainConnection(bool isOK);
    void needReopenSerialComm();                                            // 시리얼 포트 오류로 인해 다시 포트를 열어 복구가 필요한 경우

signals:
    void measure_start();
    void measure_cnt_check();
    void measure_end();
    void comm_check();

private:
    Ui::MainWindow *ui = nullptr;    
    QTimer *camera_timer, *detect_on_timer, *work_on_timer, *third_on_timer, *detect_off_timer, *port_reset_timer, *timer_sec, *hub_port_delay_timer, *comm_polling_timer;
    QImage qt_image;
    relay_seed_ddl *measure_relay;
    QElapsedTimer *mesure_time_check;

    bool bAutoSetSN;    //각 단말의 연결 상태를 위한 필드

    QHash<QString, QVariant> m_settings;
    bool isEnableBLE;
    void EnableBLEControls(bool value);
    bool isCaresensC(); //V176.110.x.x

    bool checkProtocol();
    void doCommands(QList<Sp::ProtocolCommand> list);

    void InsertListLog(QString str);
    void InsertListStateLog(int state, QString str);
    void ClearListLog();
    bool isOtgModeVisible(); //CareSens N Premier BLE – V89.110.x.x, CareSens N(N-ISO) – V39.200.x.x, CareSens N(N-ISO) Notch – V129.100.x.x

};

#endif // MAINWINDOW_H
