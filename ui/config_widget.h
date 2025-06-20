#ifndef CONFIG_WIDGET_H
#define CONFIG_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QSplitter>
#include <QRadioButton>
#include <QButtonGroup>
#include "../communication/serial_thread.h"
#include "../communication/socket_thread.h"

class ConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigWidget(QWidget *parent = nullptr);
    ~ConfigWidget();

    // 通信方式枚举
    enum CommunicationType {
        Serial = 0,
        Socket = 1
    };

    // 获取当前通信方式
    CommunicationType getCurrentCommunicationType() const;
    
    // 获取串口配置
    SerialThread::SerialConfig getSerialConfig() const;
    
    // 获取Socket配置
    SocketThread::SocketConfig getSocketConfig() const;
    
    // 获取MAC地址高字节
    uint8_t getMacHighByte() const;
    
    // 获取IP地址
    QString getIpAddress() const;
    
    // 设置连接状态
    void setConnectionState(bool connected, CommunicationType type);

signals:
    // 连接/断开信号
    void connectRequested(CommunicationType type);
    void disconnectRequested();
    
    // MAC地址设置信号
    void macAddressSetRequested(uint8_t macHighByte);
    
    // IP地址设置信号
    void ipAddressSetRequested(const QString& ipAddress);

private slots:
    void onCommunicationTypeChanged();
    void onConnectClicked();
    void onDisconnectClicked();
    void onSetMacClicked();
    void onSetIpClicked();
    void onSerialPortRefreshClicked();

private:
    // 主布局
    QVBoxLayout* m_mainLayout;
    QSplitter* m_splitter;
    
    // 通信设置组
    QGroupBox* m_commGroupBox;
    QRadioButton* m_serialRadio;
    QRadioButton* m_socketRadio;
    QButtonGroup* m_commTypeGroup;
    
    // 串口设置组
    QGroupBox* m_serialGroupBox;
    QComboBox* m_serialPortCombo;
    QPushButton* m_serialRefreshBtn;
    QComboBox* m_baudRateCombo;
    QComboBox* m_dataBitsCombo;
    QComboBox* m_parityCombo;
    QComboBox* m_stopBitsCombo;
    QComboBox* m_flowControlCombo;
    
    // Socket设置组
    QGroupBox* m_socketGroupBox;
    QLineEdit* m_hostEdit;
    QSpinBox* m_portSpinBox;
    QSpinBox* m_timeoutSpinBox;
    
    // 参数设置组
    QGroupBox* m_paramGroupBox;
    
    // MAC地址设置
    QGroupBox* m_macGroupBox;
    QLineEdit* m_macEdit;
    QLabel* m_macDisplayLabel;
    QPushButton* m_setMacBtn;
    
    // IP地址设置
    QGroupBox* m_ipGroupBox;
    QLineEdit* m_ipEdit;
    QPushButton* m_setIpBtn;
    
    // 连接控制
    QGroupBox* m_controlGroupBox;
    QPushButton* m_connectBtn;
    QPushButton* m_disconnectBtn;
    QLabel* m_statusLabel;
    
    // 当前状态
    bool m_isConnected;
    CommunicationType m_currentType;
    
    // 初始化方法
    void initializeUI();
    void initializeCommunicationGroup();
    void initializeSerialGroup();
    void initializeSocketGroup();
    void initializeParameterGroup();
    void initializeControlGroup();
    void setupConnections();
    void updateUI();
    void populateSerialPorts();
    void populateBaudRates();
    void populateSerialSettings();
    bool validateMacInput(const QString& input);
    bool validateIpInput(const QString& input);
};

#endif // CONFIG_WIDGET_H 