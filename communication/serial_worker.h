#ifndef SERIAL_WORKER_H
#define SERIAL_WORKER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QMutex>
#include <QQueue>
#include <QTimer>

class SerialWorker : public QObject
{
    Q_OBJECT

public:
    // 串口配置参数
    struct SerialConfig {
        QString portName;           // 串口名称
        QSerialPort::BaudRate baudRate;      // 波特率
        QSerialPort::DataBits dataBits;      // 数据位
        QSerialPort::Parity parity;          // 校验位
        QSerialPort::StopBits stopBits;      // 停止位
        QSerialPort::FlowControl flowControl; // 流控制
        
        SerialConfig() {
            portName = "";
            baudRate = QSerialPort::Baud9600;
            dataBits = QSerialPort::Data8;
            parity = QSerialPort::NoParity;
            stopBits = QSerialPort::OneStop;
            flowControl = QSerialPort::NoFlowControl;
        }
    };

    explicit SerialWorker(QObject *parent = nullptr);
    ~SerialWorker();

    // 获取可用串口列表
    static QStringList getAvailablePorts();

public slots:
    // 串口控制槽函数
    void openSerial(const SerialConfig& config);
    void closeSerial();
    void sendData(const QByteArray& data);
    
    // 初始化和清理
    void initialize();
    void cleanup();

signals:
    // 数据接收信号
    void dataReceived(const QByteArray& data);
    
    // 连接状态改变信号
    void connectionStateChanged(bool connected);
    
    // 错误信号
    void errorOccurred(const QString& errorString);
    
    // 数据发送完成信号
    void dataSent(const QByteArray& data);
    
    // 操作结果信号
    void openResult(bool success, const QString& message);

private slots:
    void handleReadyRead();
    void handleErrorOccurred(QSerialPort::SerialPortError error);
    void processSendQueue();

private:
    QSerialPort* m_serialPort;
    SerialConfig m_config;
    bool m_connected;
    
    // 发送队列
    QQueue<QByteArray> m_sendQueue;
    QMutex m_sendMutex;
    
    // 发送处理定时器
    QTimer* m_sendTimer;
    
    // 内部方法
    void cleanupSerial();
};

#endif // SERIAL_WORKER_H 