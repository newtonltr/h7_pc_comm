#include "serial_thread.h"
#include <QDebug>
#include <QSerialPortInfo>
#include <QThread>

SerialThread::SerialThread(QObject *parent)
    : QThread(parent)
    , m_serialPort(nullptr)
    , m_connected(false)
    , m_stopRequested(false)
{
}

SerialThread::~SerialThread()
{
    closeSerial();
    
    // 停止线程
    m_stopRequested = true;
    m_condition.wakeAll();
    
    if (isRunning()) {
        quit();
        wait(3000); // 等待最多3秒
    }
}

bool SerialThread::openSerial(const SerialConfig& config)
{
    if (m_connected) {
        closeSerial();
    }
    
    m_config = config;
    
    // 创建串口对象
    m_serialPort = new QSerialPort(this);
    m_serialPort->setPortName(config.portName);
    m_serialPort->setBaudRate(config.baudRate);
    m_serialPort->setDataBits(config.dataBits);
    m_serialPort->setParity(config.parity);
    m_serialPort->setStopBits(config.stopBits);
    m_serialPort->setFlowControl(config.flowControl);
    
    // 连接信号槽
    connect(m_serialPort, &QSerialPort::readyRead, 
            this, &SerialThread::handleReadyRead);
    connect(m_serialPort, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::errorOccurred),
            this, &SerialThread::handleErrorOccurred);
    
    // 尝试打开串口
    if (m_serialPort->open(QIODevice::ReadWrite)) {
        m_connected = true;
        emit connectionStateChanged(true);
        
        // 启动线程
        if (!isRunning()) {
            start();
        }
        
        qDebug() << "串口打开成功:" << config.portName;
        return true;
    } else {
        QString errorMsg = QString("无法打开串口 %1: %2")
                          .arg(config.portName)
                          .arg(m_serialPort->errorString());
        qWarning() << errorMsg;
        emit errorOccurred(errorMsg);
        
        cleanupSerial();
        return false;
    }
}

void SerialThread::closeSerial()
{
    if (m_serialPort && m_connected) {
        m_connected = false;
        
        // 关闭串口
        m_serialPort->close();
        emit connectionStateChanged(false);
        
        qDebug() << "串口已关闭:" << m_config.portName;
    }
    
    cleanupSerial();
}

bool SerialThread::isConnected() const
{
    return m_connected && m_serialPort && m_serialPort->isOpen();
}

void SerialThread::sendData(const QByteArray& data)
{
    if (!isConnected()) {
        emit errorOccurred("串口未连接，无法发送数据");
        return;
    }
    
    // 将数据加入发送队列
    QMutexLocker locker(&m_sendMutex);
    m_sendQueue.enqueue(data);
    
    // 唤醒工作线程
    m_condition.wakeAll();
}

QStringList SerialThread::getAvailablePorts()
{
    QStringList portList;
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    
    for (const QSerialPortInfo &portInfo : serialPortInfos) {
        portList << portInfo.portName();
    }
    
    return portList;
}

SerialThread::SerialConfig SerialThread::getCurrentConfig() const
{
    return m_config;
}

void SerialThread::run()
{
    qDebug() << "串口线程开始运行";
    
    while (!m_stopRequested) {
        // 处理发送队列
        processSendQueue();
        
        // 等待新的发送请求或退出信号
        QMutexLocker locker(&m_mutex);
        if (m_sendQueue.isEmpty() && !m_stopRequested) {
            m_condition.wait(&m_mutex, 100); // 最多等待100ms
        }
    }
    
    qDebug() << "串口线程结束运行";
}

void SerialThread::handleReadyRead()
{
    if (!m_serialPort) {
        return;
    }
    
    QByteArray data = m_serialPort->readAll();
    if (!data.isEmpty()) {
        qDebug() << "串口接收数据:" << data.toHex(' ');
        emit dataReceived(data);
    }
}

void SerialThread::handleErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) {
        return;
    }
    
    QString errorString;
    switch (error) {
    case QSerialPort::DeviceNotFoundError:
        errorString = "设备未找到";
        break;
    case QSerialPort::PermissionError:
        errorString = "权限错误";
        break;
    case QSerialPort::OpenError:
        errorString = "打开错误";
        break;
    case QSerialPort::WriteError:
        errorString = "写入错误";
        break;
    case QSerialPort::ReadError:
        errorString = "读取错误";
        break;
    case QSerialPort::ResourceError:
        errorString = "资源错误";
        break;
    case QSerialPort::UnsupportedOperationError:
        errorString = "不支持的操作";
        break;
    case QSerialPort::TimeoutError:
        errorString = "超时错误";
        break;
    default:
        errorString = "未知错误";
        break;
    }
    
    qWarning() << "串口错误:" << errorString;
    emit errorOccurred(errorString);
    
    // 严重错误时断开连接
    if (error == QSerialPort::ResourceError || 
        error == QSerialPort::DeviceNotFoundError) {
        closeSerial();
    }
}

void SerialThread::processSendQueue()
{
    QMutexLocker locker(&m_sendMutex);
    
    while (!m_sendQueue.isEmpty() && isConnected()) {
        QByteArray data = m_sendQueue.dequeue();
        locker.unlock();
        
        // 发送数据
        qint64 bytesWritten = m_serialPort->write(data);
        if (bytesWritten == data.size()) {
            if (m_serialPort->waitForBytesWritten(1000)) {
                qDebug() << "串口发送数据成功:" << data.toHex(' ');
                emit dataSent(data);
            } else {
                qWarning() << "串口发送数据超时";
                emit errorOccurred("数据发送超时");
            }
        } else {
            qWarning() << "串口数据发送不完整";
            emit errorOccurred("数据发送不完整");
        }
        
        locker.relock();
    }
}

void SerialThread::cleanupSerial()
{
    if (m_serialPort) {
        m_serialPort->deleteLater();
        m_serialPort = nullptr;
    }
    
    // 清空发送队列
    QMutexLocker locker(&m_sendMutex);
    m_sendQueue.clear();
} 