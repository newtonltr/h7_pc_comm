#include "status_widget.h"
#include <QMessageBox>
#include <QDebug>

StatusWidget::StatusWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_controlGroupBox(nullptr)
    , m_hardFaultReadBtn(nullptr)
    , m_vcuReadBtn(nullptr)
    , m_statusLabel(nullptr)
    , m_progressBar(nullptr)
    , m_displayTabWidget(nullptr)
    , m_hardFaultTab(nullptr)
    , m_hardFaultScrollArea(nullptr)
    , m_vcuTab(nullptr)
    , m_vcuScrollArea(nullptr)
    , m_isReading(false)
    , m_statusTimer(nullptr)
{
    initializeUI();
    setupConnections();
}

StatusWidget::~StatusWidget()
{
    if (m_statusTimer) {
        m_statusTimer->stop();
    }
}

void StatusWidget::initializeUI()
{
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    
    // 初始化各个功能组
    initializeControlGroup();
    initializeDisplayTabs();
    
    // 添加到主布局
    m_mainLayout->addWidget(m_controlGroupBox);
    m_mainLayout->addWidget(m_displayTabWidget);
    
    // 设置布局比例
    m_mainLayout->setStretchFactor(m_controlGroupBox, 0);
    m_mainLayout->setStretchFactor(m_displayTabWidget, 1);
}

void StatusWidget::initializeControlGroup()
{
    m_controlGroupBox = new QGroupBox("状态读取控制", this);
    QVBoxLayout* controlLayout = new QVBoxLayout(m_controlGroupBox);
    
    // 按键区域
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_hardFaultReadBtn = new QPushButton("HardFault故障信息读取", this);
    m_vcuReadBtn = new QPushButton("VCU综合信息读取", this);
    
    m_hardFaultReadBtn->setMinimumHeight(40);
    m_vcuReadBtn->setMinimumHeight(40);
    
    buttonLayout->addWidget(m_hardFaultReadBtn);
    buttonLayout->addWidget(m_vcuReadBtn);
    buttonLayout->addStretch();
    
    // 状态显示区域
    QHBoxLayout* statusLayout = new QHBoxLayout();
    m_statusLabel = new QLabel("就绪", this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_progressBar->setRange(0, 0); // 无限进度条
    
    statusLayout->addWidget(new QLabel("状态:", this));
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addWidget(m_progressBar);
    statusLayout->addStretch();
    
    controlLayout->addLayout(buttonLayout);
    controlLayout->addLayout(statusLayout);
}

void StatusWidget::initializeDisplayTabs()
{
    m_displayTabWidget = new QTabWidget(this);
    
    initializeHardFaultTab();
    initializeVcuTab();
    
    m_displayTabWidget->addTab(m_hardFaultTab, "HardFault故障信息");
    m_displayTabWidget->addTab(m_vcuTab, "VCU综合信息");
}

void StatusWidget::initializeHardFaultTab()
{
    m_hardFaultTab = new QWidget();
    m_hardFaultScrollArea = new QScrollArea(m_hardFaultTab);
    m_hardFaultScrollArea->setWidgetResizable(true);
    
    // 创建内容widget
    QWidget* contentWidget = new QWidget();
    QGridLayout* gridLayout = new QGridLayout(contentWidget);
    
    int row = 0;
    
    // 魔数标识
    gridLayout->addWidget(new QLabel("魔数标识:", contentWidget), row, 0);
    m_magicNumberEdit = new QLineEdit(contentWidget);
    m_magicNumberEdit->setReadOnly(true);
    gridLayout->addWidget(m_magicNumberEdit, row++, 1);
    
    // 时间戳
    gridLayout->addWidget(new QLabel("时间戳(运行时间):", contentWidget), row, 0);
    m_timestampEdit = new QLineEdit(contentWidget);
    m_timestampEdit->setReadOnly(true);
    gridLayout->addWidget(m_timestampEdit, row++, 1);
    
    // 程序计数器值
    gridLayout->addWidget(new QLabel("程序计数器值(PC):", contentWidget), row, 0);
    m_pcValueEdit = new QLineEdit(contentWidget);
    m_pcValueEdit->setReadOnly(true);
    gridLayout->addWidget(m_pcValueEdit, row++, 1);
    
    // 堆栈指针值
    gridLayout->addWidget(new QLabel("堆栈指针值(SP):", contentWidget), row, 0);
    m_spValueEdit = new QLineEdit(contentWidget);
    m_spValueEdit->setReadOnly(true);
    gridLayout->addWidget(m_spValueEdit, row++, 1);
    
    // 链接寄存器值
    gridLayout->addWidget(new QLabel("链接寄存器值(LR):", contentWidget), row, 0);
    m_lrValueEdit = new QLineEdit(contentWidget);
    m_lrValueEdit->setReadOnly(true);
    gridLayout->addWidget(m_lrValueEdit, row++, 1);
    
    // 故障计数器
    gridLayout->addWidget(new QLabel("故障计数器:", contentWidget), row, 0);
    m_faultCountEdit = new QLineEdit(contentWidget);
    m_faultCountEdit->setReadOnly(true);
    gridLayout->addWidget(m_faultCountEdit, row++, 1);
    
    // 最后更新时间
    m_hardFaultLastUpdateLabel = new QLabel("暂无数据", contentWidget);
    m_hardFaultLastUpdateLabel->setStyleSheet("color: gray; font-style: italic;");
    gridLayout->addWidget(new QLabel("最后更新:", contentWidget), row, 0);
    gridLayout->addWidget(m_hardFaultLastUpdateLabel, row++, 1);
    
    // 添加弹性空间
    gridLayout->setRowStretch(row, 1);
    
    m_hardFaultScrollArea->setWidget(contentWidget);
    
    // 设置tab布局
    QVBoxLayout* tabLayout = new QVBoxLayout(m_hardFaultTab);
    tabLayout->addWidget(m_hardFaultScrollArea);
}

void StatusWidget::initializeVcuTab()
{
    m_vcuTab = new QWidget();
    m_vcuScrollArea = new QScrollArea(m_vcuTab);
    m_vcuScrollArea->setWidgetResizable(true);
    
    // 创建内容widget
    QWidget* contentWidget = new QWidget();
    QGridLayout* gridLayout = new QGridLayout(contentWidget);
    
    int row = 0;
    
    // 版本信息组
    QLabel* versionGroupLabel = new QLabel("版本信息", contentWidget);
    versionGroupLabel->setStyleSheet("font-weight: bold; color: blue; margin-top: 10px;");
    gridLayout->addWidget(versionGroupLabel, row++, 0, 1, 2);
    
    gridLayout->addWidget(new QLabel("软件版本:", contentWidget), row, 0);
    m_softwareVersionEdit = new QLineEdit(contentWidget);
    m_softwareVersionEdit->setReadOnly(true);
    gridLayout->addWidget(m_softwareVersionEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("硬件版本:", contentWidget), row, 0);
    m_hardwareVersionEdit = new QLineEdit(contentWidget);
    m_hardwareVersionEdit->setReadOnly(true);
    gridLayout->addWidget(m_hardwareVersionEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("Boot版本:", contentWidget), row, 0);
    m_bootVersionEdit = new QLineEdit(contentWidget);
    m_bootVersionEdit->setReadOnly(true);
    gridLayout->addWidget(m_bootVersionEdit, row++, 1);
    
    // 电源信息组
    QLabel* powerGroupLabel = new QLabel("电源信息", contentWidget);
    powerGroupLabel->setStyleSheet("font-weight: bold; color: blue; margin-top: 10px;");
    gridLayout->addWidget(powerGroupLabel, row++, 0, 1, 2);
    
    gridLayout->addWidget(new QLabel("电量(%):", contentWidget), row, 0);
    m_electricEdit = new QLineEdit(contentWidget);
    m_electricEdit->setReadOnly(true);
    gridLayout->addWidget(m_electricEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("电压(V):", contentWidget), row, 0);
    m_voltageEdit = new QLineEdit(contentWidget);
    m_voltageEdit->setReadOnly(true);
    gridLayout->addWidget(m_voltageEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("电流(A):", contentWidget), row, 0);
    m_currentEdit = new QLineEdit(contentWidget);
    m_currentEdit->setReadOnly(true);
    gridLayout->addWidget(m_currentEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("无线充电电压(V):", contentWidget), row, 0);
    m_wirelessVoltageEdit = new QLineEdit(contentWidget);
    m_wirelessVoltageEdit->setReadOnly(true);
    gridLayout->addWidget(m_wirelessVoltageEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("无线充电电流(A):", contentWidget), row, 0);
    m_wirelessCurrentEdit = new QLineEdit(contentWidget);
    m_wirelessCurrentEdit->setReadOnly(true);
    gridLayout->addWidget(m_wirelessCurrentEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("电池温度(℃):", contentWidget), row, 0);
    m_batTemperatureEdit = new QLineEdit(contentWidget);
    m_batTemperatureEdit->setReadOnly(true);
    gridLayout->addWidget(m_batTemperatureEdit, row++, 1);
    
    // 环境信息组
    QLabel* envGroupLabel = new QLabel("环境信息", contentWidget);
    envGroupLabel->setStyleSheet("font-weight: bold; color: blue; margin-top: 10px;");
    gridLayout->addWidget(envGroupLabel, row++, 0, 1, 2);
    
    gridLayout->addWidget(new QLabel("温度(℃):", contentWidget), row, 0);
    m_temperatureEdit = new QLineEdit(contentWidget);
    m_temperatureEdit->setReadOnly(true);
    gridLayout->addWidget(m_temperatureEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("湿度(%):", contentWidget), row, 0);
    m_humidityEdit = new QLineEdit(contentWidget);
    m_humidityEdit->setReadOnly(true);
    gridLayout->addWidget(m_humidityEdit, row++, 1);
    
    // 网络信息组
    QLabel* netGroupLabel = new QLabel("网络信息", contentWidget);
    netGroupLabel->setStyleSheet("font-weight: bold; color: blue; margin-top: 10px;");
    gridLayout->addWidget(netGroupLabel, row++, 0, 1, 2);
    
    gridLayout->addWidget(new QLabel("IP地址:", contentWidget), row, 0);
    m_ipAddressEdit = new QLineEdit(contentWidget);
    m_ipAddressEdit->setReadOnly(true);
    gridLayout->addWidget(m_ipAddressEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("端口:", contentWidget), row, 0);
    m_portEdit = new QLineEdit(contentWidget);
    m_portEdit->setReadOnly(true);
    gridLayout->addWidget(m_portEdit, row++, 1);
    
    // 传感器状态组
    QLabel* sensorGroupLabel = new QLabel("传感器状态", contentWidget);
    sensorGroupLabel->setStyleSheet("font-weight: bold; color: blue; margin-top: 10px;");
    gridLayout->addWidget(sensorGroupLabel, row++, 0, 1, 2);
    
    gridLayout->addWidget(new QLabel("前碰撞:", contentWidget), row, 0);
    m_crashHeadEdit = new QLineEdit(contentWidget);
    m_crashHeadEdit->setReadOnly(true);
    gridLayout->addWidget(m_crashHeadEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("后碰撞:", contentWidget), row, 0);
    m_crashRearEdit = new QLineEdit(contentWidget);
    m_crashRearEdit->setReadOnly(true);
    gridLayout->addWidget(m_crashRearEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("接近开关:", contentWidget), row, 0);
    m_proximityEdit = new QLineEdit(contentWidget);
    m_proximityEdit->setReadOnly(true);
    gridLayout->addWidget(m_proximityEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("急停:", contentWidget), row, 0);
    m_emergencyStopEdit = new QLineEdit(contentWidget);
    m_emergencyStopEdit->setReadOnly(true);
    gridLayout->addWidget(m_emergencyStopEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("火焰传感:", contentWidget), row, 0);
    m_fireSensorEdit = new QLineEdit(contentWidget);
    m_fireSensorEdit->setReadOnly(true);
    gridLayout->addWidget(m_fireSensorEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("跌落传感:", contentWidget), row, 0);
    m_fallSensorEdit = new QLineEdit(contentWidget);
    m_fallSensorEdit->setReadOnly(true);
    gridLayout->addWidget(m_fallSensorEdit, row++, 1);
    
    // 超声波传感器组
    QLabel* ultrasonicGroupLabel = new QLabel("超声波传感器", contentWidget);
    ultrasonicGroupLabel->setStyleSheet("font-weight: bold; color: blue; margin-top: 10px;");
    gridLayout->addWidget(ultrasonicGroupLabel, row++, 0, 1, 2);
    
    gridLayout->addWidget(new QLabel("前超声波避障:", contentWidget), row, 0);
    m_ultrasonicFEdit = new QLineEdit(contentWidget);
    m_ultrasonicFEdit->setReadOnly(true);
    gridLayout->addWidget(m_ultrasonicFEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("后超声波避障:", contentWidget), row, 0);
    m_ultrasonicREdit = new QLineEdit(contentWidget);
    m_ultrasonicREdit->setReadOnly(true);
    gridLayout->addWidget(m_ultrasonicREdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("左转超声波避障:", contentWidget), row, 0);
    m_ultrasonicTlEdit = new QLineEdit(contentWidget);
    m_ultrasonicTlEdit->setReadOnly(true);
    gridLayout->addWidget(m_ultrasonicTlEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("右转超声波避障:", contentWidget), row, 0);
    m_ultrasonicTrEdit = new QLineEdit(contentWidget);
    m_ultrasonicTrEdit->setReadOnly(true);
    gridLayout->addWidget(m_ultrasonicTrEdit, row++, 1);
    
    // 控制信息组
    QLabel* controlGroupLabel = new QLabel("控制信息", contentWidget);
    controlGroupLabel->setStyleSheet("font-weight: bold; color: blue; margin-top: 10px;");
    gridLayout->addWidget(controlGroupLabel, row++, 0, 1, 2);
    
    gridLayout->addWidget(new QLabel("控制模式:", contentWidget), row, 0);
    m_ctrlModeEdit = new QLineEdit(contentWidget);
    m_ctrlModeEdit->setReadOnly(true);
    gridLayout->addWidget(m_ctrlModeEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("清除模式:", contentWidget), row, 0);
    m_clearModeEdit = new QLineEdit(contentWidget);
    m_clearModeEdit->setReadOnly(true);
    gridLayout->addWidget(m_clearModeEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("遥控线速度:", contentWidget), row, 0);
    m_joyVcEdit = new QLineEdit(contentWidget);
    m_joyVcEdit->setReadOnly(true);
    gridLayout->addWidget(m_joyVcEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("遥控角速度:", contentWidget), row, 0);
    m_joyVwEdit = new QLineEdit(contentWidget);
    m_joyVwEdit->setReadOnly(true);
    gridLayout->addWidget(m_joyVwEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("反馈线速度:", contentWidget), row, 0);
    m_twistVcEdit = new QLineEdit(contentWidget);
    m_twistVcEdit->setReadOnly(true);
    gridLayout->addWidget(m_twistVcEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("反馈角速度:", contentWidget), row, 0);
    m_twistVwEdit = new QLineEdit(contentWidget);
    m_twistVwEdit->setReadOnly(true);
    gridLayout->addWidget(m_twistVwEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("指令线速度:", contentWidget), row, 0);
    m_cmdVcEdit = new QLineEdit(contentWidget);
    m_cmdVcEdit->setReadOnly(true);
    gridLayout->addWidget(m_cmdVcEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("指令角速度:", contentWidget), row, 0);
    m_cmdVwEdit = new QLineEdit(contentWidget);
    m_cmdVwEdit->setReadOnly(true);
    gridLayout->addWidget(m_cmdVwEdit, row++, 1);
    
    // 设备状态组
    QLabel* deviceGroupLabel = new QLabel("设备状态", contentWidget);
    deviceGroupLabel->setStyleSheet("font-weight: bold; color: blue; margin-top: 10px;");
    gridLayout->addWidget(deviceGroupLabel, row++, 0, 1, 2);
    
    gridLayout->addWidget(new QLabel("设备锁状态:", contentWidget), row, 0);
    m_devLockStaEdit = new QLineEdit(contentWidget);
    m_devLockStaEdit->setReadOnly(true);
    gridLayout->addWidget(m_devLockStaEdit, row++, 1);
    
    gridLayout->addWidget(new QLabel("升降机高度:", contentWidget), row, 0);
    m_lifterHEdit = new QLineEdit(contentWidget);
    m_lifterHEdit->setReadOnly(true);
    gridLayout->addWidget(m_lifterHEdit, row++, 1);
    
    // 最后更新时间
    m_vcuLastUpdateLabel = new QLabel("暂无数据", contentWidget);
    m_vcuLastUpdateLabel->setStyleSheet("color: gray; font-style: italic;");
    gridLayout->addWidget(new QLabel("最后更新:", contentWidget), row, 0);
    gridLayout->addWidget(m_vcuLastUpdateLabel, row++, 1);
    
    // 添加弹性空间
    gridLayout->setRowStretch(row, 1);
    
    m_vcuScrollArea->setWidget(contentWidget);
    
    // 设置tab布局
    QVBoxLayout* tabLayout = new QVBoxLayout(m_vcuTab);
    tabLayout->addWidget(m_vcuScrollArea);
}

void StatusWidget::setupConnections()
{
    // 按键信号连接
    connect(m_hardFaultReadBtn, &QPushButton::clicked, this, &StatusWidget::onHardFaultReadClicked);
    connect(m_vcuReadBtn, &QPushButton::clicked, this, &StatusWidget::onVcuReadClicked);
    
    // 状态更新定时器
    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &StatusWidget::updateStatusDisplay);
    m_statusTimer->start(1000); // 每秒更新一次
}

void StatusWidget::onHardFaultReadClicked()
{
    emit hardFaultInfoReadRequested();
}

void StatusWidget::onVcuReadClicked()
{
    emit vcuInfoReadRequested();
}

void StatusWidget::displayHardFaultInfo(const hardfault_info_t& hardFaultData)
{
    // 更新HardFault信息显示
    m_magicNumberEdit->setText(formatHexValue(hardFaultData.magic_number));
    m_timestampEdit->setText(formatTimestamp(hardFaultData.timestamp));
    m_pcValueEdit->setText(formatHexValue(hardFaultData.pc_value));
    m_spValueEdit->setText(formatHexValue(hardFaultData.sp_value));
    m_lrValueEdit->setText(formatHexValue(hardFaultData.lr_value));
    m_faultCountEdit->setText(QString::number(hardFaultData.fault_count));
    
    // 更新时间戳
    m_lastHardFaultUpdate = QDateTime::currentDateTime();
    m_hardFaultLastUpdateLabel->setText(m_lastHardFaultUpdate.toString("yyyy-MM-dd hh:mm:ss"));
    
    // 切换到HardFault页面
    m_displayTabWidget->setCurrentIndex(0);
}

void StatusWidget::displayVcuInfo(const state_def_t& vcuData)
{
    // 版本信息
    m_softwareVersionEdit->setText(formatVersion(vcuData.software_version, 16));
    m_hardwareVersionEdit->setText(formatVersion(vcuData.hardware_version, 16));
    m_bootVersionEdit->setText(formatVersion(vcuData.boot_version, 16));
    
    // 电源信息
    m_electricEdit->setText(QString::number(vcuData.electric) + "%");
    m_voltageEdit->setText(formatFloatValue(vcuData.voltage, 2) + "V");
    m_currentEdit->setText(formatFloatValue(vcuData.current, 2) + "A");
    m_wirelessVoltageEdit->setText(formatFloatValue(vcuData.wireless_voltage, 2) + "V");
    m_wirelessCurrentEdit->setText(formatFloatValue(vcuData.wireless_current, 2) + "A");
    m_batTemperatureEdit->setText(formatFloatValue(vcuData.bat_temperature, 1) + "℃");
    
    // 环境信息
    m_temperatureEdit->setText(formatFloatValue(vcuData.temperature, 1) + "℃");
    m_humidityEdit->setText(formatFloatValue(vcuData.humidity, 1) + "%");
    
    // 网络信息
    m_ipAddressEdit->setText(formatIpAddress(vcuData.ip));
    m_portEdit->setText(QString::number(vcuData.port));
    
    // 传感器状态
    m_crashHeadEdit->setText(formatBoolValue(vcuData.crash_head));
    m_crashRearEdit->setText(formatBoolValue(vcuData.crash_rear));
    m_proximityEdit->setText(formatBoolValue(vcuData.proximity));
    m_emergencyStopEdit->setText(formatBoolValue(vcuData.emergency_stop));
    m_fireSensorEdit->setText(formatBoolValue(vcuData.fire_sensor));
    m_fallSensorEdit->setText(formatBoolValue(vcuData.fall_sensor));
    
    // 超声波传感器
    m_ultrasonicFEdit->setText(formatBoolValue(vcuData.ultrasonic_f));
    m_ultrasonicREdit->setText(formatBoolValue(vcuData.ultrasonic_r));
    m_ultrasonicTlEdit->setText(formatBoolValue(vcuData.ultrasonic_tl));
    m_ultrasonicTrEdit->setText(formatBoolValue(vcuData.ultrasonic_tr));
    
    // 控制信息
    m_ctrlModeEdit->setText(QString::number(vcuData.ctrl_mode));
    m_clearModeEdit->setText(QString::number(vcuData.clear_mode));
    m_joyVcEdit->setText(formatFloatValue(vcuData.joy_vc, 3));
    m_joyVwEdit->setText(formatFloatValue(vcuData.joy_vw, 3));
    m_twistVcEdit->setText(formatFloatValue(vcuData.twist_vc, 3));
    m_twistVwEdit->setText(formatFloatValue(vcuData.twist_vw, 3));
    m_cmdVcEdit->setText(formatFloatValue(vcuData.cmd_vc, 3));
    m_cmdVwEdit->setText(formatFloatValue(vcuData.cmd_vw, 3));
    
    // 设备状态
    m_devLockStaEdit->setText(formatBoolValue(vcuData.dev_lock_sta));
    m_lifterHEdit->setText(QString::number(vcuData.lifter_h));
    
    // 更新时间戳
    m_lastVcuUpdate = QDateTime::currentDateTime();
    m_vcuLastUpdateLabel->setText(m_lastVcuUpdate.toString("yyyy-MM-dd hh:mm:ss"));
    
    // 切换到VCU页面
    m_displayTabWidget->setCurrentIndex(1);
}

void StatusWidget::setReadingStatus(bool isReading, const QString& message)
{
    m_isReading = isReading;
    
    if (!message.isEmpty()) {
        m_statusLabel->setText(message);
    }
}

void StatusWidget::showErrorMessage(const QString& error)
{
    m_statusLabel->setText(QString("错误: %1").arg(error));
    m_statusLabel->setStyleSheet("color: red;");
    
    // 3秒后恢复正常状态
    QTimer::singleShot(3000, [this]() {
        m_statusLabel->setText("就绪");
        m_statusLabel->setStyleSheet("");
    });
}

void StatusWidget::updateStatusDisplay()
{
}

// 数据格式化工具函数实现
QString StatusWidget::formatTimestamp(uint32_t timestamp)
{
    uint32_t seconds = timestamp / 1000;
    uint32_t ms = timestamp % 1000;
    uint32_t hours = seconds / 3600;
    uint32_t minutes = (seconds % 3600) / 60;
    uint32_t secs = seconds % 60;
    
    return QString("%1:%2:%3.%4")
           .arg(hours, 2, 10, QChar('0'))
           .arg(minutes, 2, 10, QChar('0'))
           .arg(secs, 2, 10, QChar('0'))
           .arg(ms, 3, 10, QChar('0'));
}

QString StatusWidget::formatHexValue(uint32_t value)
{
    return QString("0x%1").arg(value, 8, 16, QChar('0')).toUpper();
}

QString StatusWidget::formatFloatValue(float value, int precision)
{
    return QString::number(value, 'f', precision);
}

QString StatusWidget::formatBoolValue(uint8_t value)
{
    return value ? "是" : "否";
}

QString StatusWidget::formatIpAddress(const uint8_t ip[4])
{
    return QString("%1.%2.%3.%4")
           .arg(ip[0])
           .arg(ip[1])
           .arg(ip[2])
           .arg(ip[3]);
}

QString StatusWidget::formatVersion(const char* version, int maxLength)
{
    QByteArray versionArray(version, maxLength);
    // 查找第一个'\0'或到达最大长度
    int nullIndex = versionArray.indexOf('\0');
    if (nullIndex >= 0) {
        versionArray.truncate(nullIndex);
    }
    return QString::fromUtf8(versionArray);
} 