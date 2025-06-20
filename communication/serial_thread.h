#ifndef SERIAL_THREAD_H
#define SERIAL_THREAD_H

#include <QThread>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>

class SerialThread : public QThread
{
    Q_OBJECT

public:
    explicit SerialThread(QObject *parent = nullptr);
    ~SerialThread();

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

    // 控制方法
    bool openSerial(const SerialConfig& config);
    void closeSerial();
    bool isConnected() const;
    
    // 数据发送
    void sendData(const QByteArray& data);
    
    // 获取可用串口列表
    static QStringList getAvailablePorts();
    
    // 获取当前配置
    SerialConfig getCurrentConfig() const;

signals:
    // 数据接收信号
    void dataReceived(const QByteArray& data);
    
    // 连接状态改变信号
    void connectionStateChanged(bool connected);
    
    // 错误信号
    void errorOccurred(const QString& errorString);
    
    // 数据发送完成信号
    void dataSent(const QByteArray& data);

protected:
    void run() override;

private slots:
    void handleReadyRead();
    void handleErrorOccurred(QSerialPort::SerialPortError error);

private:
    QSerialPort* m_serialPort;
    SerialConfig m_config;
    bool m_connected;
    
    // 线程同步
    QMutex m_mutex;
    QWaitCondition m_condition;
    bool m_stopRequested;
    
    // 发送队列
    QQueue<QByteArray> m_sendQueue;
    QMutex m_sendMutex;
    
    // 内部方法
    void processSendQueue();
    void cleanupSerial();
};

#endif // SERIAL_THREAD_H 