#ifndef DEBUG_WIDGET_H
#define DEBUG_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QTimer>

class DebugWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DebugWidget(QWidget *parent = nullptr);
    ~DebugWidget();

    // 数据显示格式枚举
    enum DisplayFormat {
        HexFormat = 0,
        AsciiFormat = 1,
        MixedFormat = 2
    };

    // 添加发送数据日志
    void addSentData(const QByteArray& data);
    
    // 添加接收数据日志
    void addReceivedData(const QByteArray& data);
    
    // 添加错误信息
    void addErrorMessage(const QString& message);
    
    // 添加状态信息
    void addStatusMessage(const QString& message);
    
    // 清除所有数据
    void clearAllData();
    
    // 设置是否显示时间戳
    void setShowTimestamp(bool show);
    
    // 设置数据显示格式
    void setDisplayFormat(DisplayFormat format);

public slots:
    // 清除发送数据
    void clearSentData();
    
    // 清除接收数据
    void clearReceivedData();
    
    // 保存日志到文件
    void saveLogToFile();

private slots:
    void onDisplayFormatChanged();
    void onTimestampToggled(bool enabled);
    void onAutoScrollToggled(bool enabled);
    void updateStatistics();

private:
    // 主布局
    QVBoxLayout* m_mainLayout;
    QSplitter* m_splitter;
    
    // 控制面板
    QGroupBox* m_controlGroupBox;
    QComboBox* m_formatCombo;
    QCheckBox* m_timestampCheckBox;
    QCheckBox* m_autoScrollCheckBox;
    QPushButton* m_clearSentBtn;
    QPushButton* m_clearReceivedBtn;
    QPushButton* m_clearAllBtn;
    QPushButton* m_saveLogBtn;
    
    // 发送数据显示区
    QGroupBox* m_sentGroupBox;
    QPlainTextEdit* m_sentTextEdit;
    QLabel* m_sentStatsLabel;
    
    // 接收数据显示区
    QGroupBox* m_receivedGroupBox;
    QPlainTextEdit* m_receivedTextEdit;
    QLabel* m_receivedStatsLabel;
    
    // 状态信息显示区
    QGroupBox* m_statusGroupBox;
    QPlainTextEdit* m_statusTextEdit;
    
    // 当前设置
    DisplayFormat m_displayFormat;
    bool m_showTimestamp;
    bool m_autoScroll;
    
    // 统计信息
    int m_sentBytesCount;
    int m_receivedBytesCount;
    int m_sentPacketsCount;
    int m_receivedPacketsCount;
    QDateTime m_startTime;
    
    // 统计更新定时器
    QTimer* m_statsTimer;
    
    // 初始化方法
    void initializeUI();
    void initializeControlPanel();
    void initializeDataDisplays();
    void setupConnections();
    
    // 工具方法
    QString formatData(const QByteArray& data) const;
    QString getCurrentTimestamp() const;
    void appendToTextEdit(QPlainTextEdit* textEdit, const QString& text);
    void updateSentStatistics();
    void updateReceivedStatistics();
    QString formatBytes(int bytes) const;
    QString formatDuration(qint64 seconds) const;
};

#endif // DEBUG_WIDGET_H 