#include "config_widget.h"
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QIntValidator>
#include <QMessageBox>
#include <QDebug>

ConfigWidget::ConfigWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
    , m_isConnected(false)
    , m_currentType(Serial)
{
    initializeUI();
    setupConnections();
    updateUI();
}

ConfigWidget::~ConfigWidget()
{
    // Qt的父子关系会自动清理子对象
}

void ConfigWidget::initializeUI()
{
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    
    // 创建分割器
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_mainLayout->addWidget(m_splitter);
    
    // 初始化各个功能组
    initializeCommunicationGroup();
    initializeSerialGroup();
    initializeSocketGroup();
    initializeParameterGroup();
    initializeControlGroup();
    
    // 将左侧通信设置组合到一个widget中
    QWidget* leftWidget = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->addWidget(m_commGroupBox);
    leftLayout->addWidget(m_serialGroupBox);
    leftLayout->addWidget(m_socketGroupBox);
    leftLayout->addWidget(m_controlGroupBox);
    leftLayout->addStretch();
    
    // 将右侧参数设置添加到分割器
    m_splitter->addWidget(leftWidget);
    m_splitter->addWidget(m_paramGroupBox);
    
    // 设置分割器比例
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 1);
}

void ConfigWidget::initializeCommunicationGroup()
{
    m_commGroupBox = new QGroupBox("通信方式", this);
    QHBoxLayout* layout = new QHBoxLayout(m_commGroupBox);
    
    // 创建单选按钮
    m_serialRadio = new QRadioButton("串口通信", m_commGroupBox);
    m_socketRadio = new QRadioButton("网络通信", m_commGroupBox);
    
    // 创建按钮组
    m_commTypeGroup = new QButtonGroup(this);
    m_commTypeGroup->addButton(m_serialRadio, Serial);
    m_commTypeGroup->addButton(m_socketRadio, Socket);
    
    // 默认选择串口
    m_serialRadio->setChecked(true);
    
    layout->addWidget(m_serialRadio);
    layout->addWidget(m_socketRadio);
    layout->addStretch();
}

void ConfigWidget::initializeSerialGroup()
{
    m_serialGroupBox = new QGroupBox("串口设置", this);
    QGridLayout* layout = new QGridLayout(m_serialGroupBox);
    
    // 串口选择
    layout->addWidget(new QLabel("串口:"), 0, 0);
    m_serialPortCombo = new QComboBox(m_serialGroupBox);
    layout->addWidget(m_serialPortCombo, 0, 1);
    m_serialRefreshBtn = new QPushButton("刷新", m_serialGroupBox);
    layout->addWidget(m_serialRefreshBtn, 0, 2);
    
    // 波特率
    layout->addWidget(new QLabel("波特率:"), 1, 0);
    m_baudRateCombo = new QComboBox(m_serialGroupBox);
    layout->addWidget(m_baudRateCombo, 1, 1, 1, 2);
    
    // 数据位
    layout->addWidget(new QLabel("数据位:"), 2, 0);
    m_dataBitsCombo = new QComboBox(m_serialGroupBox);
    layout->addWidget(m_dataBitsCombo, 2, 1, 1, 2);
    
    // 校验位
    layout->addWidget(new QLabel("校验位:"), 3, 0);
    m_parityCombo = new QComboBox(m_serialGroupBox);
    layout->addWidget(m_parityCombo, 3, 1, 1, 2);
    
    // 停止位
    layout->addWidget(new QLabel("停止位:"), 4, 0);
    m_stopBitsCombo = new QComboBox(m_serialGroupBox);
    layout->addWidget(m_stopBitsCombo, 4, 1, 1, 2);
    
    // 流控制
    layout->addWidget(new QLabel("流控制:"), 5, 0);
    m_flowControlCombo = new QComboBox(m_serialGroupBox);
    layout->addWidget(m_flowControlCombo, 5, 1, 1, 2);
    
    // 填充串口设置项
    populateSerialSettings();
    populateSerialPorts();
}

void ConfigWidget::initializeSocketGroup()
{
    m_socketGroupBox = new QGroupBox("网络设置", this);
    QGridLayout* layout = new QGridLayout(m_socketGroupBox);
    
    // IP地址
    layout->addWidget(new QLabel("IP地址:"), 0, 0);
    m_hostEdit = new QLineEdit("192.168.1.135", m_socketGroupBox);
    layout->addWidget(m_hostEdit, 0, 1);
    
    // 端口
    layout->addWidget(new QLabel("端口:"), 1, 0);
    m_portSpinBox = new QSpinBox(m_socketGroupBox);
    m_portSpinBox->setRange(1, 65535);
    m_portSpinBox->setValue(65000);
    layout->addWidget(m_portSpinBox, 1, 1);
    
    // 超时时间
    layout->addWidget(new QLabel("超时(ms):"), 2, 0);
    m_timeoutSpinBox = new QSpinBox(m_socketGroupBox);
    m_timeoutSpinBox->setRange(1000, 30000);
    m_timeoutSpinBox->setValue(5000);
    layout->addWidget(m_timeoutSpinBox, 2, 1);
    
    // 默认禁用Socket设置
    m_socketGroupBox->setEnabled(false);
}

void ConfigWidget::initializeParameterGroup()
{
    m_paramGroupBox = new QGroupBox("参数设置", this);
    QVBoxLayout* mainLayout = new QVBoxLayout(m_paramGroupBox);
    
    // MAC地址设置组
    m_macGroupBox = new QGroupBox("MAC地址设置", m_paramGroupBox);
    QGridLayout* macLayout = new QGridLayout(m_macGroupBox);
    
    macLayout->addWidget(new QLabel("MAC高字节:"), 0, 0);
    m_macEdit = new QLineEdit(m_macGroupBox);
    m_macEdit->setPlaceholderText("请输入0-255的数值");
    
    // 添加输入验证
    QIntValidator* macValidator = new QIntValidator(0, 255, this);
    m_macEdit->setValidator(macValidator);
    macLayout->addWidget(m_macEdit, 0, 1);
    
    m_setMacBtn = new QPushButton("设置MAC", m_macGroupBox);
    macLayout->addWidget(m_setMacBtn, 0, 2);
    
    macLayout->addWidget(new QLabel("完整MAC:"), 1, 0);
    m_macDisplayLabel = new QLabel("01:00:00:00:00:02", m_macGroupBox);
    m_macDisplayLabel->setStyleSheet("QLabel { color: blue; font-weight: bold; }");
    macLayout->addWidget(m_macDisplayLabel, 1, 1, 1, 2);
    
    // IP地址设置组
    m_ipGroupBox = new QGroupBox("IP地址设置", m_paramGroupBox);
    QGridLayout* ipLayout = new QGridLayout(m_ipGroupBox);
    
    ipLayout->addWidget(new QLabel("IP地址:"), 0, 0);
    m_ipEdit = new QLineEdit("192.168.110.111", m_ipGroupBox);
    m_ipEdit->setPlaceholderText("例: 192.168.110.111");
    
    // IP地址格式验证
    QRegularExpression ipRegex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    QRegularExpressionValidator* ipValidator = new QRegularExpressionValidator(ipRegex, this);
    m_ipEdit->setValidator(ipValidator);
    ipLayout->addWidget(m_ipEdit, 0, 1);
    
    m_setIpBtn = new QPushButton("设置IP", m_ipGroupBox);
    ipLayout->addWidget(m_setIpBtn, 0, 2);

    // 子网掩码设置组
    m_maskGroupBox = new QGroupBox("子网掩码设置", m_paramGroupBox);
    QGridLayout* maskLayout = new QGridLayout(m_maskGroupBox);
    
    maskLayout->addWidget(new QLabel("子网掩码:"), 0, 0);
    m_maskEdit = new QLineEdit("255.255.255.0", m_maskGroupBox);
    maskLayout->addWidget(m_maskEdit, 0, 1);
    
    m_setMaskBtn = new QPushButton("设置子网掩码", m_maskGroupBox);
    maskLayout->addWidget(m_setMaskBtn, 0, 2);

    // 网关地址设置组
    m_gatewayGroupBox = new QGroupBox("网关地址设置", m_paramGroupBox);
    QGridLayout* gatewayLayout = new QGridLayout(m_gatewayGroupBox);

    gatewayLayout->addWidget(new QLabel("网关地址:"), 0, 0);
    m_gatewayEdit = new QLineEdit("192.168.110.1", m_gatewayGroupBox);
    gatewayLayout->addWidget(m_gatewayEdit, 0, 1);

    m_setGatewayBtn = new QPushButton("设置网关", m_gatewayGroupBox);
    gatewayLayout->addWidget(m_setGatewayBtn, 0, 2);

    // VCU参数设置组
    m_vcuParamGroupBox = new QGroupBox("VCU参数设置", m_paramGroupBox);
    QGridLayout* vcuParamLayout = new QGridLayout(m_vcuParamGroupBox);
    
    vcuParamLayout->addWidget(new QLabel("前避障减速距离(m):"), 0, 0);
    m_vcuParamFrontDecObstacleDistanceEdit = new QLineEdit("1.80", m_vcuParamGroupBox);
    vcuParamLayout->addWidget(m_vcuParamFrontDecObstacleDistanceEdit, 0, 1);

    vcuParamLayout->addWidget(new QLabel("前避障停止距离(m):"), 1, 0);
    m_vcuParamFrontStopObstacleDistanceEdit = new QLineEdit("0.30", m_vcuParamGroupBox);
    vcuParamLayout->addWidget(m_vcuParamFrontStopObstacleDistanceEdit, 1, 1);

    vcuParamLayout->addWidget(new QLabel("后避障距离(m):"), 2, 0);
    m_vcuParamRearObstacleDistanceEdit = new QLineEdit("0.16", m_vcuParamGroupBox);
    vcuParamLayout->addWidget(m_vcuParamRearObstacleDistanceEdit, 2, 1);

    vcuParamLayout->addWidget(new QLabel("速度校正系数:"), 3, 0);
    m_vcuParamSpeedCorrectionFactorEdit = new QLineEdit("0.98", m_vcuParamGroupBox);
    vcuParamLayout->addWidget(m_vcuParamSpeedCorrectionFactorEdit, 3, 1);

    m_setVcuParamBtn = new QPushButton("设置VCU参数", m_vcuParamGroupBox);
    vcuParamLayout->addWidget(m_setVcuParamBtn, 4, 0);

    mainLayout->addWidget(m_macGroupBox);
    mainLayout->addWidget(m_ipGroupBox);
    mainLayout->addWidget(m_maskGroupBox);
    mainLayout->addWidget(m_gatewayGroupBox);
    mainLayout->addWidget(m_vcuParamGroupBox);
    mainLayout->addStretch();
}

void ConfigWidget::initializeControlGroup()
{
    m_controlGroupBox = new QGroupBox("连接控制", this);
    QVBoxLayout* layout = new QVBoxLayout(m_controlGroupBox);
    
    // 连接按钮
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_connectBtn = new QPushButton("连接", m_controlGroupBox);
    m_disconnectBtn = new QPushButton("断开", m_controlGroupBox);
    m_disconnectBtn->setEnabled(false);
    
    btnLayout->addWidget(m_connectBtn);
    btnLayout->addWidget(m_disconnectBtn);
    
    // 状态标签
    m_statusLabel = new QLabel("未连接", m_controlGroupBox);
    m_statusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    
    layout->addLayout(btnLayout);
    layout->addWidget(m_statusLabel);
}

void ConfigWidget::setupConnections()
{
    // 通信方式改变
    connect(m_commTypeGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &ConfigWidget::onCommunicationTypeChanged);
    
    // 连接控制
    connect(m_connectBtn, &QPushButton::clicked, this, &ConfigWidget::onConnectClicked);
    connect(m_disconnectBtn, &QPushButton::clicked, this, &ConfigWidget::onDisconnectClicked);
    
    // 参数设置
    connect(m_setMacBtn, &QPushButton::clicked, this, &ConfigWidget::onSetMacClicked);
    connect(m_setIpBtn, &QPushButton::clicked, this, &ConfigWidget::onSetIpClicked);
    connect(m_setMaskBtn, &QPushButton::clicked, this, &ConfigWidget::onSetMaskClicked);
    connect(m_setGatewayBtn, &QPushButton::clicked, this, &ConfigWidget::onSetGatewayClicked);
    connect(m_setVcuParamBtn, &QPushButton::clicked, this, &ConfigWidget::onSetVcuParamClicked);
    // 串口刷新
    connect(m_serialRefreshBtn, &QPushButton::clicked, this, &ConfigWidget::onSerialPortRefreshClicked);
    
    // MAC编辑框变化时更新显示
    connect(m_macEdit, &QLineEdit::textChanged, [this](const QString& text) {
        bool ok;
        int value = text.toInt(&ok);
        if (ok && value >= 0 && value <= 255) {
            m_macDisplayLabel->setText(QString("%1:00:00:00:00:02").arg(value, 2, 16, QChar('0')).toUpper());
        } else {
            m_macDisplayLabel->setText("01:00:00:00:00:02");
        }
    });
}

// 事件处理方法
void ConfigWidget::onCommunicationTypeChanged()
{
    int id = m_commTypeGroup->checkedId();
    m_currentType = static_cast<CommunicationType>(id);
    updateUI();
}

void ConfigWidget::onConnectClicked()
{
    emit connectRequested(m_currentType);
}

void ConfigWidget::onDisconnectClicked()
{
    emit disconnectRequested();
}

void ConfigWidget::onSetMacClicked()
{
    if (!validateMacInput(m_macEdit->text())) {
        QMessageBox::warning(this, "输入错误", "请输入有效的MAC地址高字节值(0-255)");
        return;
    }
    
    uint8_t macHighByte = static_cast<uint8_t>(m_macEdit->text().toInt());
    emit macAddressSetRequested(macHighByte);
}

void ConfigWidget::onSetIpClicked()
{
    if (!validateIpInput(m_ipEdit->text())) {
        QMessageBox::warning(this, "输入错误", "请输入有效的IP地址格式(如: 192.168.110.111)");
        return;
    }
    
    emit ipAddressSetRequested(m_ipEdit->text());
}

void ConfigWidget::onSetMaskClicked()
{
    emit maskAddressSetRequested(m_maskEdit->text());
}

void ConfigWidget::onSetGatewayClicked()
{
    emit gatewayAddressSetRequested(m_gatewayEdit->text());
}

void ConfigWidget::onSetVcuParamClicked()
{
    emit vcuParamSetRequested(m_vcuParamFrontDecObstacleDistanceEdit->text(), m_vcuParamFrontStopObstacleDistanceEdit->text(), m_vcuParamRearObstacleDistanceEdit->text(), m_vcuParamSpeedCorrectionFactorEdit->text());
}


void ConfigWidget::onSerialPortRefreshClicked()
{
    populateSerialPorts();
}

// 数据获取方法
ConfigWidget::CommunicationType ConfigWidget::getCurrentCommunicationType() const
{
    return m_currentType;
}

SerialThread::SerialConfig ConfigWidget::getSerialConfig() const
{
    SerialThread::SerialConfig config;
    
    config.portName = m_serialPortCombo->currentText();
    
    // 波特率
    switch (m_baudRateCombo->currentIndex()) {
    case 0: config.baudRate = QSerialPort::Baud1200; break;
    case 1: config.baudRate = QSerialPort::Baud2400; break;
    case 2: config.baudRate = QSerialPort::Baud4800; break;
    case 3: config.baudRate = QSerialPort::Baud9600; break;
    case 4: config.baudRate = QSerialPort::Baud19200; break;
    case 5: config.baudRate = QSerialPort::Baud38400; break;
    case 6: config.baudRate = QSerialPort::Baud57600; break;
    case 7: config.baudRate = QSerialPort::Baud115200; break;
    default: config.baudRate = QSerialPort::Baud9600; break;
    }
    
    // 数据位
    switch (m_dataBitsCombo->currentIndex()) {
    case 0: config.dataBits = QSerialPort::Data5; break;
    case 1: config.dataBits = QSerialPort::Data6; break;
    case 2: config.dataBits = QSerialPort::Data7; break;
    case 3: config.dataBits = QSerialPort::Data8; break;
    default: config.dataBits = QSerialPort::Data8; break;
    }
    
    // 校验位
    switch (m_parityCombo->currentIndex()) {
    case 0: config.parity = QSerialPort::NoParity; break;
    case 1: config.parity = QSerialPort::EvenParity; break;
    case 2: config.parity = QSerialPort::OddParity; break;
    case 3: config.parity = QSerialPort::SpaceParity; break;
    case 4: config.parity = QSerialPort::MarkParity; break;
    default: config.parity = QSerialPort::NoParity; break;
    }
    
    // 停止位
    switch (m_stopBitsCombo->currentIndex()) {
    case 0: config.stopBits = QSerialPort::OneStop; break;
    case 1: config.stopBits = QSerialPort::OneAndHalfStop; break;
    case 2: config.stopBits = QSerialPort::TwoStop; break;
    default: config.stopBits = QSerialPort::OneStop; break;
    }
    
    // 流控制
    switch (m_flowControlCombo->currentIndex()) {
    case 0: config.flowControl = QSerialPort::NoFlowControl; break;
    case 1: config.flowControl = QSerialPort::HardwareControl; break;
    case 2: config.flowControl = QSerialPort::SoftwareControl; break;
    default: config.flowControl = QSerialPort::NoFlowControl; break;
    }
    
    return config;
}

SocketThread::SocketConfig ConfigWidget::getSocketConfig() const
{
    SocketThread::SocketConfig config;
    config.hostAddress = m_hostEdit->text();
    config.port = static_cast<quint16>(m_portSpinBox->value());
    config.connectTimeout = m_timeoutSpinBox->value();
    config.readTimeout = 3000;
    config.autoReconnect = true;
    config.reconnectInterval = 3000;
    
    return config;
}

uint8_t ConfigWidget::getMacHighByte() const
{
    bool ok;
    int value = m_macEdit->text().toInt(&ok);
    if (ok && value >= 0 && value <= 255) {
        return static_cast<uint8_t>(value);
    }
    return 1; // 默认值
}

QString ConfigWidget::getIpAddress() const
{
    return m_ipEdit->text();
}

void ConfigWidget::setConnectionState(bool connected, CommunicationType type)
{
    m_isConnected = connected;
    m_currentType = type;
    
    // 更新UI状态
    m_connectBtn->setEnabled(!connected);
    m_disconnectBtn->setEnabled(connected);
    
    // 更新状态标签
    if (connected) {
        QString typeStr = (type == Serial) ? "串口" : "网络";
        m_statusLabel->setText(QString("%1 已连接").arg(typeStr));
        m_statusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
    } else {
        m_statusLabel->setText("未连接");
        m_statusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    }
    
    // 连接时禁用通信方式选择
    m_serialRadio->setEnabled(!connected);
    m_socketRadio->setEnabled(!connected);
}

// 工具方法
void ConfigWidget::updateUI()
{
    // 根据当前通信类型启用/禁用相应的设置组
    m_serialGroupBox->setEnabled(m_currentType == Serial);
    m_socketGroupBox->setEnabled(m_currentType == Socket);
}

void ConfigWidget::populateSerialPorts()
{
    m_serialPortCombo->clear();
    QStringList ports = SerialThread::getAvailablePorts();
    m_serialPortCombo->addItems(ports);
    
    if (ports.isEmpty()) {
        m_serialPortCombo->addItem("无可用串口");
    }
}

void ConfigWidget::populateBaudRates()
{
    m_baudRateCombo->clear();
    m_baudRateCombo->addItems({
        "1200", "2400", "4800", "9600", "19200", 
        "38400", "57600", "115200"
    });
    m_baudRateCombo->setCurrentText("115200"); // 默认选择115200
}

void ConfigWidget::populateSerialSettings()
{
    // 填充波特率
    populateBaudRates();
    
    // 填充数据位
    m_dataBitsCombo->addItems({"5", "6", "7", "8"});
    m_dataBitsCombo->setCurrentText("8");
    
    // 填充校验位
    m_parityCombo->addItems({"无校验", "偶校验", "奇校验", "空格校验", "标记校验"});
    m_parityCombo->setCurrentText("无校验");
    
    // 填充停止位
    m_stopBitsCombo->addItems({"1", "1.5", "2"});
    m_stopBitsCombo->setCurrentText("1");
    
    // 填充流控制
    m_flowControlCombo->addItems({"无流控", "硬件流控", "软件流控"});
    m_flowControlCombo->setCurrentText("无流控");
}

bool ConfigWidget::validateMacInput(const QString& input)
{
    bool ok;
    int value = input.toInt(&ok);
    return ok && value >= 0 && value <= 255;
}

bool ConfigWidget::validateIpInput(const QString& input)
{
    QRegularExpression ipRegex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    return ipRegex.match(input).hasMatch();
} 