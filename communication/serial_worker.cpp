#include "serial_worker.h"
#include <QDebug>
#include <QSerialPortInfo>

SerialWorker::SerialWorker(QObject *parent)
    : QObject(parent)
    , m_serialPort(nullptr)
    , m_connected(false)
    , m_sendTimer(nullptr)
{
}

SerialWorker::~SerialWorker()
{
    cleanup();
}

void SerialWorker::initialize()
{
    // 创建发送处理定时器
    m_sendTimer = new QTimer(this);
    connect(m_sendTimer, &QTimer::timeout, this, &SerialWorker::processSendQueue);
    m_sendTimer->setSingleShot(false);
    m_sendTimer->setInterval(10); // 10ms 检查一次发送队列
}

void SerialWorker::cleanup()
{
    closeSerial();
    
    if (m_sendTimer) {
        m_sendTimer->stop();
        m_sendTimer->deleteLater();
        m_sendTimer = nullptr;
    }
}

void SerialWorker::openSerial(const SerialConfig& config)
{
    if (m_connected) {
        closeSerial();
    }
    
    m_config = config;
    
    // 创建串口对象 - 无父对象，将在工作线程中运行
    m_serialPort = new QSerialPort();
    m_serialPort->setPortName(config.portName);
    m_serialPort->setBaudRate(config.baudRate);
    m_serialPort->setDataBits(config.dataBits);
    m_serialPort->setParity(config.parity);
    m_serialPort->setStopBits(config.stopBits);
    m_serialPort->setFlowControl(config.flowControl);
    
    // 连接信号槽
    connect(m_serialPort, &QSerialPort::readyRead, 
            this, &SerialWorker::handleReadyRead);
    connect(m_serialPort, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::errorOccurred),
            this, &SerialWorker::handleErrorOccurred);
    
    // 尝试打开串口
    if (m_serialPort->open(QIODevice::ReadWrite)) {
        m_connected = true;
        emit connectionStateChanged(true);
        
        // 启动发送定时器
        if (m_sendTimer) {
            m_sendTimer->start();
        }
        
        qDebug() << "串口打开成功:" << config.portName;
        emit openResult(true, QString("串口打开成功: %1").arg(config.portName));
    } else {
        QString errorMsg = QString("无法打开串口 %1: %2")
                          .arg(config.portName)
                          .arg(m_serialPort->errorString());
        qWarning() << errorMsg;
        emit errorOccurred(errorMsg);
        emit openResult(false, errorMsg);
        
        cleanupSerial();
    }
}

void SerialWorker::closeSerial()
{
    if (m_serialPort && m_connected) {
        m_connected = false;
        
        // 停止发送定时器
        if (m_sendTimer) {
            m_sendTimer->stop();
        }
        
        // 关闭串口
        m_serialPort->close();
        emit connectionStateChanged(false);
        
        qDebug() << "串口已关闭:" << m_config.portName;
    }
    
    cleanupSerial();
}

void SerialWorker::sendData(const QByteArray& data)
{
    if (!m_connected || !m_serialPort || !m_serialPort->isOpen()) {
        emit errorOccurred("串口未连接，无法发送数据");
        return;
    }
    
    // 将数据加入发送队列
    QMutexLocker locker(&m_sendMutex);
    m_sendQueue.enqueue(data);
}

QStringList SerialWorker::getAvailablePorts()
{
    QStringList portList;
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    
    for (const QSerialPortInfo &portInfo : serialPortInfos) {
        portList << portInfo.portName();
    }
    
    return portList;
}

void SerialWorker::handleReadyRead()
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

void SerialWorker::handleErrorOccurred(QSerialPort::SerialPortError error)
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

void SerialWorker::processSendQueue()
{
    if (!m_connected || !m_serialPort || !m_serialPort->isOpen()) {
        return;
    }
    
    QMutexLocker locker(&m_sendMutex);
    
    while (!m_sendQueue.isEmpty()) {
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

void SerialWorker::cleanupSerial()
{
    if (m_serialPort) {
        m_serialPort->deleteLater();
        m_serialPort = nullptr;
    }
    
    // 清空发送队列
    QMutexLocker locker(&m_sendMutex);
    m_sendQueue.clear();
} 