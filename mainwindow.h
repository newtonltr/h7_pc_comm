#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QMessageBox>
#include "ui/config_widget.h"
#include "ui/debug_widget.h"
#include "communication/serial_thread.h"
#include "communication/socket_thread.h"
#include "protocol/protocol_frame.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 菜单动作槽函数
    void onSaveConfig();
    void onLoadConfig();
    void onExit();
    void onAbout();
    
    // 通信控制槽函数
    void onConnectRequested(ConfigWidget::CommunicationType type);
    void onDisconnectRequested();
    
    // 设备参数设置槽函数
    void onMacAddressSetRequested(uint8_t macHighByte);
    void onIpAddressSetRequested(const QString& ipAddress);
    void onMaskAddressSetRequested(const QString& maskAddress);
    void onGatewayAddressSetRequested(const QString& gatewayAddress);
    void onVcuParamSetRequested(const QString& rearObstacleDistance, const QString& speedCorrectionFactor);
    
    // 串口通信槽函数
    void onSerialDataReceived(const QByteArray& data);
    void onSerialDataSent(const QByteArray& data);
    void onSerialConnectionChanged(bool connected);
    void onSerialError(const QString& error);
    
    // Socket通信槽函数
    void onSocketDataReceived(const QByteArray& data);
    void onSocketDataSent(const QByteArray& data);
    void onSocketConnectionChanged(bool connected);
    void onSocketError(const QString& error);
    
    // 状态更新
    void updateConnectionStatus();

private:
    Ui::MainWindow *ui;
    
    // UI组件
    ConfigWidget* m_configWidget;
    DebugWidget* m_debugWidget;
    QLabel* m_statusLabel;
    QLabel* m_timeLabel;
    QTimer* m_statusTimer;
    
    // 通信组件
    SerialThread* m_serialThread;
    SocketThread* m_socketThread;
    
    // 当前连接状态
    bool m_isConnected;
    ConfigWidget::CommunicationType m_currentConnectionType;
    
    // 初始化方法
    void setupUI();
    void setupMenuActions();
    void setupStatusBar();
    void setupConnections();
    void setupCommunication();
    
    // 工具方法
    void showMessage(const QString& message, int timeout = 3000);
    void showError(const QString& error);
    void updateWindowTitle();
    bool sendProtocolFrame(const QByteArray& frameData);
    void processReceivedFrame(const QByteArray& frameData);
};

#endif // MAINWINDOW_H
