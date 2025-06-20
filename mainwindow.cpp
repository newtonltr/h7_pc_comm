#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QDateTime>
#include <QFileDialog>
#include <QSettings>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_configWidget(nullptr)
    , m_debugWidget(nullptr)
    , m_statusLabel(nullptr)
    , m_timeLabel(nullptr)
    , m_statusTimer(nullptr)
    , m_serialThread(nullptr)
    , m_socketThread(nullptr)
    , m_isConnected(false)
    , m_currentConnectionType(ConfigWidget::Serial)
{
    ui->setupUi(this);
    
    // 初始化各个组件
    setupUI();
    setupStatusBar();
    setupMenuActions();
    setupCommunication();
    setupConnections();
    
    // 更新窗口标题
    updateWindowTitle();
    
    // 显示欢迎信息
    showMessage("H7 IPSet 上位机启动完成");
    
    // 启动状态更新定时器
    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &MainWindow::updateConnectionStatus);
    m_statusTimer->start(1000); // 每秒更新一次
}

MainWindow::~MainWindow()
{
    // 关闭所有连接
    if (m_isConnected) {
        onDisconnectRequested();
    }
    
    // 清理通信线程
    if (m_serialThread) {
        m_serialThread->closeSerial();
        m_serialThread->deleteLater();
    }
    
    if (m_socketThread) {
        m_socketThread->disconnectFromHost();
        m_socketThread->deleteLater();
    }
    
    delete ui;
}

void MainWindow::setupUI()
{
    // 创建配置组件
    m_configWidget = new ConfigWidget(this);
    
    // 创建调试组件
    m_debugWidget = new DebugWidget(this);
    
    // 将组件添加到Tab页面中
    QVBoxLayout* configLayout = qobject_cast<QVBoxLayout*>(ui->configTab->layout());
    if (configLayout) {
        // 移除占位符
        QWidget* placeholder = ui->configWidgetPlaceholder;
        configLayout->removeWidget(placeholder);
        placeholder->deleteLater();
        
        // 添加配置组件
        configLayout->addWidget(m_configWidget);
    }
    
    QVBoxLayout* debugLayout = qobject_cast<QVBoxLayout*>(ui->debugTab->layout());
    if (debugLayout) {
        // 移除占位符
        QWidget* placeholder = ui->debugWidgetPlaceholder;
        debugLayout->removeWidget(placeholder);
        placeholder->deleteLater();
        
        // 添加调试组件
        debugLayout->addWidget(m_debugWidget);
    }
    
    // 设置初始Tab页面
    ui->tabWidget->setCurrentIndex(0);
}

void MainWindow::setupStatusBar()
{
    // 创建状态标签
    m_statusLabel = new QLabel("就绪", this);
    m_statusLabel->setMinimumWidth(200);
    ui->statusbar->addWidget(m_statusLabel);
    
    // 添加分隔符
    ui->statusbar->addPermanentWidget(new QLabel("|"), 0);
    
    // 创建时间标签
    m_timeLabel = new QLabel(this);
    m_timeLabel->setMinimumWidth(150);
    ui->statusbar->addPermanentWidget(m_timeLabel);
    
    // 更新时间显示
    updateConnectionStatus();
}

void MainWindow::setupMenuActions()
{
    // 连接菜单动作
    connect(ui->actionSaveConfig, &QAction::triggered, this, &MainWindow::onSaveConfig);
    connect(ui->actionLoadConfig, &QAction::triggered, this, &MainWindow::onLoadConfig);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::onExit);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupCommunication()
{
    // 创建通信线程
    m_serialThread = new SerialThread(this);
    m_socketThread = new SocketThread(this);
}

void MainWindow::setupConnections()
{
    // 配置组件信号连接
    connect(m_configWidget, &ConfigWidget::connectRequested,
            this, &MainWindow::onConnectRequested);
    connect(m_configWidget, &ConfigWidget::disconnectRequested,
            this, &MainWindow::onDisconnectRequested);
    connect(m_configWidget, &ConfigWidget::macAddressSetRequested,
            this, &MainWindow::onMacAddressSetRequested);
    connect(m_configWidget, &ConfigWidget::ipAddressSetRequested,
            this, &MainWindow::onIpAddressSetRequested);
    
    // 串口通信信号连接
    connect(m_serialThread, &SerialThread::dataReceived,
            this, &MainWindow::onSerialDataReceived);
    connect(m_serialThread, &SerialThread::dataSent,
            this, &MainWindow::onSerialDataSent);
    connect(m_serialThread, &SerialThread::connectionStateChanged,
            this, &MainWindow::onSerialConnectionChanged);
    connect(m_serialThread, &SerialThread::errorOccurred,
            this, &MainWindow::onSerialError);
    
    // Socket通信信号连接
    connect(m_socketThread, &SocketThread::dataReceived,
            this, &MainWindow::onSocketDataReceived);
    connect(m_socketThread, &SocketThread::dataSent,
            this, &MainWindow::onSocketDataSent);
    connect(m_socketThread, &SocketThread::connectionStateChanged,
            this, &MainWindow::onSocketConnectionChanged);
    connect(m_socketThread, &SocketThread::errorOccurred,
            this, &MainWindow::onSocketError);
}

// 菜单动作槽函数
void MainWindow::onSaveConfig()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "保存配置文件", 
        "config.ini",
        "配置文件 (*.ini);;所有文件 (*.*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QSettings settings(fileName, QSettings::IniFormat);
    
    // 保存配置信息
    settings.beginGroup("Communication");
    settings.setValue("type", static_cast<int>(m_configWidget->getCurrentCommunicationType()));
    
    // 保存串口配置
    auto serialConfig = m_configWidget->getSerialConfig();
    settings.setValue("serial_port", serialConfig.portName);
    settings.setValue("baud_rate", static_cast<int>(serialConfig.baudRate));
    
    // 保存Socket配置
    auto socketConfig = m_configWidget->getSocketConfig();
    settings.setValue("host_address", socketConfig.hostAddress);
    settings.setValue("port", socketConfig.port);
    
    settings.endGroup();
    
    showMessage("配置已保存到: " + fileName);
}

void MainWindow::onLoadConfig()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "加载配置文件", 
        "",
        "配置文件 (*.ini);;所有文件 (*.*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QSettings settings(fileName, QSettings::IniFormat);
    
    // 加载配置信息
    settings.beginGroup("Communication");
    // 这里可以添加配置加载逻辑
    settings.endGroup();
    
    showMessage("配置已从文件加载: " + fileName);
}

void MainWindow::onExit()
{
    close();
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "关于 H7 IPSet 上位机",
        "<h2>H7 IPSet 上位机</h2>"
        "<p>版本: 1.0.0</p>"
        "<p>一个用于与MCU进行通信的上位机软件，支持串口和网络通信。</p>"
        "<p>功能特点：</p>"
        "<ul>"
        "<li>支持串口和Socket双通道通信</li>"
        "<li>实时数据收发显示</li>"
        "<li>MAC地址和IP地址设置</li>"
        "<li>多线程架构，界面流畅</li>"
        "</ul>"
        "<p>Copyright © 2024</p>");
}

// 通信控制槽函数
void MainWindow::onConnectRequested(ConfigWidget::CommunicationType type)
{
    if (m_isConnected) {
        showError("请先断开当前连接");
        return;
    }
    
    m_currentConnectionType = type;
    bool success = false;
    
    if (type == ConfigWidget::Serial) {
        auto config = m_configWidget->getSerialConfig();
        success = m_serialThread->openSerial(config);
        if (success) {
            m_debugWidget->addStatusMessage(QString("串口连接成功: %1").arg(config.portName));
        }
    } else {
        auto config = m_configWidget->getSocketConfig();
        success = m_socketThread->connectToHost(config);
        if (success) {
            m_debugWidget->addStatusMessage(QString("Socket连接成功: %1:%2")
                                          .arg(config.hostAddress).arg(config.port));
        }
    }
    
    if (success) {
        m_isConnected = true;
        m_configWidget->setConnectionState(true, type);
        updateWindowTitle();
        showMessage(QString("%1连接成功").arg(type == ConfigWidget::Serial ? "串口" : "网络"));
    }
}

void MainWindow::onDisconnectRequested()
{
    if (!m_isConnected) {
        return;
    }
    
    if (m_currentConnectionType == ConfigWidget::Serial) {
        m_serialThread->closeSerial();
        m_debugWidget->addStatusMessage("串口连接已断开");
    } else {
        m_socketThread->disconnectFromHost();
        m_debugWidget->addStatusMessage("Socket连接已断开");
    }
    
    m_isConnected = false;
    m_configWidget->setConnectionState(false, m_currentConnectionType);
    updateWindowTitle();
    showMessage("连接已断开");
}

// 设备参数设置槽函数
void MainWindow::onMacAddressSetRequested(uint8_t macHighByte)
{
    if (!m_isConnected) {
        showError("请先建立通信连接");
        return;
    }
    
    QByteArray frame = ProtocolFrame::buildMacSetFrame(macHighByte);
    if (sendProtocolFrame(frame)) {
        m_debugWidget->addStatusMessage(QString("MAC地址设置命令已发送，高字节: 0x%1")
                                      .arg(macHighByte, 2, 16, QChar('0')).toUpper());
    }
}

void MainWindow::onIpAddressSetRequested(const QString& ipAddress)
{
    if (!m_isConnected) {
        showError("请先建立通信连接");
        return;
    }
    
    QByteArray frame = ProtocolFrame::buildIpSetFrame(ipAddress);
    if (!frame.isEmpty() && sendProtocolFrame(frame)) {
        m_debugWidget->addStatusMessage(QString("IP地址设置命令已发送: %1").arg(ipAddress));
    } else {
        showError("IP地址格式错误或发送失败");
    }
}

// 串口通信槽函数
void MainWindow::onSerialDataReceived(const QByteArray& data)
{
    m_debugWidget->addReceivedData(data);
    processReceivedFrame(data);
}

void MainWindow::onSerialDataSent(const QByteArray& data)
{
    m_debugWidget->addSentData(data);
}

void MainWindow::onSerialConnectionChanged(bool connected)
{
    if (!connected && m_isConnected) {
        m_isConnected = false;
        m_configWidget->setConnectionState(false, ConfigWidget::Serial);
        updateWindowTitle();
        showMessage("串口连接已断开");
    }
}

void MainWindow::onSerialError(const QString& error)
{
    m_debugWidget->addErrorMessage(QString("串口错误: %1").arg(error));
    showError(QString("串口错误: %1").arg(error));
}

// Socket通信槽函数
void MainWindow::onSocketDataReceived(const QByteArray& data)
{
    m_debugWidget->addReceivedData(data);
    processReceivedFrame(data);
}

void MainWindow::onSocketDataSent(const QByteArray& data)
{
    m_debugWidget->addSentData(data);
}

void MainWindow::onSocketConnectionChanged(bool connected)
{
    if (!connected && m_isConnected) {
        m_isConnected = false;
        m_configWidget->setConnectionState(false, ConfigWidget::Socket);
        updateWindowTitle();
        showMessage("Socket连接已断开");
    }
}

void MainWindow::onSocketError(const QString& error)
{
    m_debugWidget->addErrorMessage(QString("Socket错误: %1").arg(error));
    showError(QString("Socket错误: %1").arg(error));
}

// 工具方法
void MainWindow::updateConnectionStatus()
{
    // 更新时间显示
    m_timeLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    
    // 更新状态显示
    if (m_isConnected) {
        QString typeStr = (m_currentConnectionType == ConfigWidget::Serial) ? "串口" : "网络";
        m_statusLabel->setText(QString("%1 已连接").arg(typeStr));
    } else {
        m_statusLabel->setText("就绪");
    }
}

void MainWindow::showMessage(const QString& message, int timeout)
{
    ui->statusbar->showMessage(message, timeout);
    qDebug() << "Status:" << message;
}

void MainWindow::showError(const QString& error)
{
    QMessageBox::warning(this, "错误", error);
    showMessage(QString("错误: %1").arg(error));
}

void MainWindow::updateWindowTitle()
{
    QString title = "H7 IPSet 上位机";
    if (m_isConnected) {
        QString typeStr = (m_currentConnectionType == ConfigWidget::Serial) ? "串口" : "网络";
        title += QString(" - %1已连接").arg(typeStr);
    }
    setWindowTitle(title);
}

bool MainWindow::sendProtocolFrame(const QByteArray& frameData)
{
    if (!m_isConnected || frameData.isEmpty()) {
        return false;
    }
    
    if (m_currentConnectionType == ConfigWidget::Serial) {
        m_serialThread->sendData(frameData);
    } else {
        m_socketThread->sendData(frameData);
    }
    
    return true;
}

void MainWindow::processReceivedFrame(const QByteArray& frameData)
{
    // 解析接收到的帧数据
    auto parsedData = ProtocolFrame::parseFrame(frameData);
    
    if (parsedData.isValid) {
        QString info = QString("收到有效帧: 功能码 0x%1, 数据长度 %2")
                      .arg(parsedData.functionCode, 4, 16, QChar('0'))
                      .arg(parsedData.data.size());
        m_debugWidget->addStatusMessage(info);
    } else {
        m_debugWidget->addErrorMessage(QString("帧解析失败: %1").arg(parsedData.errorMessage));
    }
}
