#ifndef STATUS_WIDGET_H
#define STATUS_WIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QTabWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QProgressBar>
#include <QTimer>
#include <QDateTime>

extern "C" {
#include "../pc_protocol.h"
}

class StatusWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatusWidget(QWidget *parent = nullptr);
    ~StatusWidget();

    // 数据显示方法
    void displayHardFaultInfo(const hardfault_info_t& hardFaultData);
    void displayVcuInfo(const state_def_t& vcuData);
    
    // 状态管理
    void setReadingStatus(bool isReading, const QString& message = QString());
    void showErrorMessage(const QString& error);

signals:
    // 请求信号
    void hardFaultInfoReadRequested();
    void vcuInfoReadRequested();

private slots:
    // 按键槽函数
    void onHardFaultReadClicked();
    void onVcuReadClicked();
    
    // 状态更新
    void updateStatusDisplay();

private:
    // UI初始化
    void initializeUI();
    void initializeControlGroup();
    void initializeDisplayTabs();
    void initializeHardFaultTab();
    void initializeVcuTab();
    void setupConnections();
    
    // 数据格式化工具
    QString formatTimestamp(uint32_t timestamp);
    QString formatHexValue(uint32_t value);
    QString formatFloatValue(float value, int precision = 2);
    QString formatBoolValue(uint8_t value);
    QString formatIpAddress(const uint8_t ip[4]);
    QString formatVersion(const char* version, int maxLength);
    
    // UI组件
    QVBoxLayout* m_mainLayout;
    
    // 控制区域
    QGroupBox* m_controlGroupBox;
    QPushButton* m_hardFaultReadBtn;
    QPushButton* m_vcuReadBtn;
    QLabel* m_statusLabel;
    QProgressBar* m_progressBar;
    
    // 显示区域
    QTabWidget* m_displayTabWidget;
    
    // HardFault信息显示
    QWidget* m_hardFaultTab;
    QScrollArea* m_hardFaultScrollArea;
    QLineEdit* m_magicNumberEdit;
    QLineEdit* m_timestampEdit;
    QLineEdit* m_pcValueEdit;
    QLineEdit* m_spValueEdit;
    QLineEdit* m_lrValueEdit;
    QLineEdit* m_faultCountEdit;
    QLabel* m_hardFaultLastUpdateLabel;
    
    // VCU信息显示
    QWidget* m_vcuTab;
    QScrollArea* m_vcuScrollArea;
    QLineEdit* m_softwareVersionEdit;
    QLineEdit* m_hardwareVersionEdit;
    QLineEdit* m_bootVersionEdit;
    QLineEdit* m_electricEdit;
    QLineEdit* m_voltageEdit;
    QLineEdit* m_currentEdit;
    QLineEdit* m_wirelessVoltageEdit;
    QLineEdit* m_wirelessCurrentEdit;
    QLineEdit* m_temperatureEdit;
    QLineEdit* m_humidityEdit;
    QLineEdit* m_ipAddressEdit;
    QLineEdit* m_portEdit;
    QLineEdit* m_crashHeadEdit;
    QLineEdit* m_crashRearEdit;
    QLineEdit* m_proximityEdit;
    QLineEdit* m_emergencyStopEdit;
    QLineEdit* m_ctrlModeEdit;
    QLineEdit* m_clearModeEdit;
    QLineEdit* m_joyVcEdit;
    QLineEdit* m_joyVwEdit;
    QLineEdit* m_twistVcEdit;
    QLineEdit* m_twistVwEdit;
    QLineEdit* m_batTemperatureEdit;
    QLineEdit* m_cmdVcEdit;
    QLineEdit* m_cmdVwEdit;
    QLineEdit* m_devLockStaEdit;
    QLineEdit* m_fireSensorEdit;
    QLineEdit* m_fallSensorEdit;
    QLineEdit* m_ultrasonicFEdit;
    QLineEdit* m_ultrasonicREdit;
    QLineEdit* m_ultrasonicTlEdit;
    QLineEdit* m_ultrasonicTrEdit;
    QLineEdit* m_lifterHEdit;
    
    // 气体传感器数据
    QLineEdit* m_airH2sEdit;
    QLineEdit* m_airCoEdit;
    QLineEdit* m_airO2Edit;
    QLineEdit* m_airExEdit;
    QLineEdit* m_airEdcEdit;
    QLineEdit* m_airC2h4Edit;
    QLineEdit* m_airHclEdit;
    QLineEdit* m_airCl2Edit;
    QLineEdit* m_airC3h6Edit;
    QLineEdit* m_airH2Edit;
    QLineEdit* m_airTempEdit;
    QLineEdit* m_airHumEdit;
    QLineEdit* m_airSf6Edit;
    QLineEdit* m_cocl2Edit;
    QLineEdit* m_c2h6oEdit;
    QLineEdit* m_ch4Edit;
    
    // 驱动器电流数据
    QLineEdit* m_drv0CurrentCh0Edit;
    QLineEdit* m_drv0CurrentCh1Edit;
    QLineEdit* m_drv1CurrentCh0Edit;
    QLineEdit* m_drv1CurrentCh1Edit;
    
    // 遥控器通道数据
    QLineEdit* m_joyCh0Edit;
    QLineEdit* m_joyCh1Edit;
    QLineEdit* m_joyCh2Edit;
    QLineEdit* m_joyCh3Edit;
    
    // 序列号数据
    QLineEdit* m_serialNumber0Edit;
    QLineEdit* m_serialNumber1Edit;
    QLineEdit* m_serialNumber2Edit;
    
    // BMS和标志位
    QLineEdit* m_stsBmsEdit;
    QLineEdit* m_flagAirInvailEdit;
    
    // 电机电流数据
    QLineEdit* m_lfMotorCurrentEdit;
    QLineEdit* m_rfMotorCurrentEdit;
    QLineEdit* m_rrMotorCurrentEdit;
    QLineEdit* m_lrMotorCurrentEdit;
    
    QLabel* m_vcuLastUpdateLabel;
    
    // 状态管理
    bool m_isReading;
    QTimer* m_statusTimer;
    QDateTime m_lastHardFaultUpdate;
    QDateTime m_lastVcuUpdate;
};

#endif // STATUS_WIDGET_H 