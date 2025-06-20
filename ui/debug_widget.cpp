#include "debug_widget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QScrollBar>
#include <QDebug>

DebugWidget::DebugWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
    , m_displayFormat(HexFormat)
    , m_showTimestamp(true)
    , m_autoScroll(true)
    , m_sentBytesCount(0)
    , m_receivedBytesCount(0)
    , m_sentPacketsCount(0)
    , m_receivedPacketsCount(0)
    , m_startTime(QDateTime::currentDateTime())
    , m_statsTimer(nullptr)
{
    initializeUI();
    setupConnections();
    
    // 启动统计更新定时器
    m_statsTimer = new QTimer(this);
    connect(m_statsTimer, &QTimer::timeout, this, &DebugWidget::updateStatistics);
    m_statsTimer->start(1000); // 每秒更新一次统计信息
}

DebugWidget::~DebugWidget()
{
    if (m_statsTimer) {
        m_statsTimer->stop();
    }
}

void DebugWidget::initializeUI()
{
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    
    // 初始化控制面板
    initializeControlPanel();
    m_mainLayout->addWidget(m_controlGroupBox);
    
    // 创建分割器
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_mainLayout->addWidget(m_splitter);
    
    // 初始化数据显示区域
    initializeDataDisplays();
    
    // 设置分割器比例
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 1);
}

void DebugWidget::initializeControlPanel()
{
    m_controlGroupBox = new QGroupBox("显示控制", this);
    QHBoxLayout* layout = new QHBoxLayout(m_controlGroupBox);
    
    // 显示格式选择
    layout->addWidget(new QLabel("数据格式:"));
    m_formatCombo = new QComboBox(m_controlGroupBox);
    m_formatCombo->addItems({"十六进制", "ASCII", "混合显示"});
    m_formatCombo->setCurrentIndex(static_cast<int>(m_displayFormat));
    layout->addWidget(m_formatCombo);
    
    // 时间戳选项
    m_timestampCheckBox = new QCheckBox("显示时间戳", m_controlGroupBox);
    m_timestampCheckBox->setChecked(m_showTimestamp);
    layout->addWidget(m_timestampCheckBox);
    
    // 自动滚动选项
    m_autoScrollCheckBox = new QCheckBox("自动滚动", m_controlGroupBox);
    m_autoScrollCheckBox->setChecked(m_autoScroll);
    layout->addWidget(m_autoScrollCheckBox);
    
    layout->addWidget(new QLabel("|")); // 分隔符
    
    // 清除按钮
    m_clearSentBtn = new QPushButton("清除发送", m_controlGroupBox);
    layout->addWidget(m_clearSentBtn);
    
    m_clearReceivedBtn = new QPushButton("清除接收", m_controlGroupBox);
    layout->addWidget(m_clearReceivedBtn);
    
    m_clearAllBtn = new QPushButton("清除全部", m_controlGroupBox);
    layout->addWidget(m_clearAllBtn);
    
    layout->addWidget(new QLabel("|")); // 分隔符
    
    // 保存日志按钮
    m_saveLogBtn = new QPushButton("保存日志", m_controlGroupBox);
    layout->addWidget(m_saveLogBtn);
    
    layout->addStretch();
}

void DebugWidget::initializeDataDisplays()
{
    // 创建左侧发送数据显示区
    QWidget* leftWidget = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
    
    // 发送数据组
    m_sentGroupBox = new QGroupBox("发送数据", leftWidget);
    QVBoxLayout* sentLayout = new QVBoxLayout(m_sentGroupBox);
    
    m_sentTextEdit = new QPlainTextEdit(m_sentGroupBox);
    m_sentTextEdit->setReadOnly(true);
    m_sentTextEdit->setMaximumBlockCount(1000); // 限制最大行数
    m_sentTextEdit->setFont(QFont("Courier", 9)); // 等宽字体
    sentLayout->addWidget(m_sentTextEdit);
    
    m_sentStatsLabel = new QLabel("发送: 0 包, 0 字节", m_sentGroupBox);
    m_sentStatsLabel->setStyleSheet("QLabel { color: blue; font-weight: bold; }");
    sentLayout->addWidget(m_sentStatsLabel);
    
    // 状态信息组
    m_statusGroupBox = new QGroupBox("状态信息", leftWidget);
    QVBoxLayout* statusLayout = new QVBoxLayout(m_statusGroupBox);
    
    m_statusTextEdit = new QPlainTextEdit(m_statusGroupBox);
    m_statusTextEdit->setReadOnly(true);
    m_statusTextEdit->setMaximumBlockCount(500);
    m_statusTextEdit->setFont(QFont("Courier", 9));
    statusLayout->addWidget(m_statusTextEdit);
    
    leftLayout->addWidget(m_sentGroupBox, 2);
    leftLayout->addWidget(m_statusGroupBox, 1);
    
    // 创建右侧接收数据显示区
    m_receivedGroupBox = new QGroupBox("接收数据", this);
    QVBoxLayout* receivedLayout = new QVBoxLayout(m_receivedGroupBox);
    
    m_receivedTextEdit = new QPlainTextEdit(m_receivedGroupBox);
    m_receivedTextEdit->setReadOnly(true);
    m_receivedTextEdit->setMaximumBlockCount(1000);
    m_receivedTextEdit->setFont(QFont("Courier", 9));
    receivedLayout->addWidget(m_receivedTextEdit);
    
    m_receivedStatsLabel = new QLabel("接收: 0 包, 0 字节", m_receivedGroupBox);
    m_receivedStatsLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
    receivedLayout->addWidget(m_receivedStatsLabel);
    
    // 添加到分割器
    m_splitter->addWidget(leftWidget);
    m_splitter->addWidget(m_receivedGroupBox);
}

void DebugWidget::setupConnections()
{
    // 控制面板信号连接
    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DebugWidget::onDisplayFormatChanged);
    connect(m_timestampCheckBox, &QCheckBox::toggled, this, &DebugWidget::onTimestampToggled);
    connect(m_autoScrollCheckBox, &QCheckBox::toggled, this, &DebugWidget::onAutoScrollToggled);
    
    // 清除按钮连接
    connect(m_clearSentBtn, &QPushButton::clicked, this, &DebugWidget::clearSentData);
    connect(m_clearReceivedBtn, &QPushButton::clicked, this, &DebugWidget::clearReceivedData);
    connect(m_clearAllBtn, &QPushButton::clicked, this, &DebugWidget::clearAllData);
    
    // 保存日志按钮连接
    connect(m_saveLogBtn, &QPushButton::clicked, this, &DebugWidget::saveLogToFile);
}

void DebugWidget::addSentData(const QByteArray& data)
{
    QString formattedData = formatData(data);
    QString timestamp = m_showTimestamp ? getCurrentTimestamp() + " " : "";
    QString message = QString("%1[发送] %2").arg(timestamp).arg(formattedData);
    
    appendToTextEdit(m_sentTextEdit, message);
    
    // 更新统计
    m_sentPacketsCount++;
    m_sentBytesCount += data.size();
    updateSentStatistics();
}

void DebugWidget::addReceivedData(const QByteArray& data)
{
    QString formattedData = formatData(data);
    QString timestamp = m_showTimestamp ? getCurrentTimestamp() + " " : "";
    QString message = QString("%1[接收] %2").arg(timestamp).arg(formattedData);
    
    appendToTextEdit(m_receivedTextEdit, message);
    
    // 更新统计
    m_receivedPacketsCount++;
    m_receivedBytesCount += data.size();
    updateReceivedStatistics();
}

void DebugWidget::addErrorMessage(const QString& message)
{
    QString timestamp = m_showTimestamp ? getCurrentTimestamp() + " " : "";
    QString errorMsg = QString("%1[错误] %2").arg(timestamp).arg(message);
    
    appendToTextEdit(m_statusTextEdit, errorMsg);
}

void DebugWidget::addStatusMessage(const QString& message)
{
    QString timestamp = m_showTimestamp ? getCurrentTimestamp() + " " : "";
    QString statusMsg = QString("%1[状态] %2").arg(timestamp).arg(message);
    
    appendToTextEdit(m_statusTextEdit, statusMsg);
}

void DebugWidget::clearAllData()
{
    clearSentData();
    clearReceivedData();
    m_statusTextEdit->clear();
    
    // 重置统计
    m_sentBytesCount = 0;
    m_receivedBytesCount = 0;
    m_sentPacketsCount = 0;
    m_receivedPacketsCount = 0;
    m_startTime = QDateTime::currentDateTime();
    
    updateStatistics();
    addStatusMessage("所有数据已清除");
}

void DebugWidget::setShowTimestamp(bool show)
{
    m_showTimestamp = show;
    m_timestampCheckBox->setChecked(show);
}

void DebugWidget::setDisplayFormat(DisplayFormat format)
{
    m_displayFormat = format;
    m_formatCombo->setCurrentIndex(static_cast<int>(format));
}

void DebugWidget::clearSentData()
{
    m_sentTextEdit->clear();
    m_sentBytesCount = 0;
    m_sentPacketsCount = 0;
    updateSentStatistics();
    addStatusMessage("发送数据已清除");
}

void DebugWidget::clearReceivedData()
{
    m_receivedTextEdit->clear();
    m_receivedBytesCount = 0;
    m_receivedPacketsCount = 0;
    updateReceivedStatistics();
    addStatusMessage("接收数据已清除");
}

void DebugWidget::saveLogToFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "保存通信日志", 
        QString("communication_log_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "文本文件 (*.txt);;所有文件 (*.*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "保存错误", "无法创建文件: " + file.errorString());
        return;
    }
    
    QTextStream out(&file);
    // Qt 6中QTextStream默认使用UTF-8编码，无需手动设置
    
    // 写入文件头信息
    out << "通信日志文件" << Qt::endl;
    out << "生成时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << Qt::endl;
    out << "会话开始时间: " << m_startTime.toString("yyyy-MM-dd hh:mm:ss") << Qt::endl;
    out << "统计信息: 发送 " << m_sentPacketsCount << " 包 " << m_sentBytesCount << " 字节, "
        << "接收 " << m_receivedPacketsCount << " 包 " << m_receivedBytesCount << " 字节" << Qt::endl;
    out << QString("=").repeated(80) << Qt::endl << Qt::endl;
    
    // 写入发送数据
    if (!m_sentTextEdit->toPlainText().isEmpty()) {
        out << "===== 发送数据 =====" << Qt::endl;
        out << m_sentTextEdit->toPlainText() << Qt::endl << Qt::endl;
    }
    
    // 写入接收数据
    if (!m_receivedTextEdit->toPlainText().isEmpty()) {
        out << "===== 接收数据 =====" << Qt::endl;
        out << m_receivedTextEdit->toPlainText() << Qt::endl << Qt::endl;
    }
    
    // 写入状态信息
    if (!m_statusTextEdit->toPlainText().isEmpty()) {
        out << "===== 状态信息 =====" << Qt::endl;
        out << m_statusTextEdit->toPlainText() << Qt::endl;
    }
    
    file.close();
    addStatusMessage(QString("日志已保存到: %1").arg(fileName));
}

void DebugWidget::onDisplayFormatChanged()
{
    m_displayFormat = static_cast<DisplayFormat>(m_formatCombo->currentIndex());
    addStatusMessage(QString("数据显示格式已切换为: %1").arg(m_formatCombo->currentText()));
}

void DebugWidget::onTimestampToggled(bool enabled)
{
    m_showTimestamp = enabled;
    addStatusMessage(QString("时间戳显示: %1").arg(enabled ? "开启" : "关闭"));
}

void DebugWidget::onAutoScrollToggled(bool enabled)
{
    m_autoScroll = enabled;
    addStatusMessage(QString("自动滚动: %1").arg(enabled ? "开启" : "关闭"));
}

void DebugWidget::updateStatistics()
{
    updateSentStatistics();
    updateReceivedStatistics();
}

QString DebugWidget::formatData(const QByteArray& data) const
{
    switch (m_displayFormat) {
    case HexFormat:
        return data.toHex(' ').toUpper();
        
    case AsciiFormat: {
        QString result;
        for (char c : data) {
            if (c >= 32 && c <= 126) {
                result += c;
            } else {
                result += QString("[%1]").arg(static_cast<uint8_t>(c), 2, 16, QChar('0')).toUpper();
            }
        }
        return result;
    }
    
    case MixedFormat: {
        QString hex = data.toHex(' ').toUpper();
        QString ascii;
        for (char c : data) {
            if (c >= 32 && c <= 126) {
                ascii += c;
            } else {
                ascii += '.';
            }
        }
        return QString("HEX: %1 | ASCII: %2").arg(hex).arg(ascii);
    }
    
    default:
        return data.toHex(' ').toUpper();
    }
}

QString DebugWidget::getCurrentTimestamp() const
{
    return QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
}

void DebugWidget::appendToTextEdit(QPlainTextEdit* textEdit, const QString& text)
{
    if (!textEdit) {
        return;
    }
    
    textEdit->appendPlainText(text);
    
    // 自动滚动到底部
    if (m_autoScroll) {
        QScrollBar* scrollBar = textEdit->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    }
}

void DebugWidget::updateSentStatistics()
{
    QString stats = QString("发送: %1 包, %2").arg(m_sentPacketsCount).arg(formatBytes(m_sentBytesCount));
    m_sentStatsLabel->setText(stats);
}

void DebugWidget::updateReceivedStatistics()
{
    QString stats = QString("接收: %1 包, %2").arg(m_receivedPacketsCount).arg(formatBytes(m_receivedBytesCount));
    m_receivedStatsLabel->setText(stats);
}

QString DebugWidget::formatBytes(int bytes) const
{
    if (bytes < 1024) {
        return QString("%1 字节").arg(bytes);
    } else if (bytes < 1024 * 1024) {
        return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    } else {
        return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
    }
}

QString DebugWidget::formatDuration(qint64 seconds) const
{
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    if (hours > 0) {
        return QString("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
    }
} 