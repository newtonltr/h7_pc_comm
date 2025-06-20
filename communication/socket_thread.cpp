#include "socket_thread.h"
#include <QDebug>
#include <QHostAddress>
#include <QTimer>

SocketThread::SocketThread(QObject *parent)
    : QThread(parent)
    , m_socket(nullptr)
    , m_connected(false)
    , m_shouldReconnect(false)
    , m_stopRequested(false)
    , m_reconnectTimer(nullptr)
{
}

SocketThread::~SocketThread()
{
    disconnectFromHost();
    
    // 停止线程
    m_stopRequested = true;
    m_condition.wakeAll();
    
    if (isRunning()) {
        quit();
        wait(3000); // 等待最多3秒
    }
}

bool SocketThread::connectToHost(const SocketConfig& config)
{
    if (m_connected) {
        disconnectFromHost();
    }
    
    m_config = config;
    m_shouldReconnect = config.autoReconnect;
    
    // 设置Socket
    setupSocket();
    
    // 尝试连接
    qDebug() << "尝试连接到" << config.hostAddress << ":" << config.port;
    m_socket->connectToHost(QHostAddress(config.hostAddress), config.port);
    
    // 等待连接结果
    if (m_socket->waitForConnected(config.connectTimeout)) {
        m_connected = true;
        emit connectionStateChanged(true);
        emit connected();
        
        // 启动线程
        if (!isRunning()) {
            start();
        }
        
        qDebug() << "Socket连接成功:" << getConnectionInfo();
        return true;
    } else {
        QString errorMsg = QString("无法连接到 %1:%2 - %3")
                          .arg(config.hostAddress)
                          .arg(config.port)
                          .arg(m_socket->errorString());
        qWarning() << errorMsg;
        emit errorOccurred(errorMsg);
        return false;
    }
}

void SocketThread::disconnectFromHost()
{
    m_shouldReconnect = false;
    
    if (m_socket && m_connected) {
        m_connected = false;
        m_socket->disconnectFromHost();
        
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->waitForDisconnected(3000);
        }
        
        emit connectionStateChanged(false);
        emit disconnected();
        qDebug() << "Socket已断开连接";
    }
    
    cleanupSocket();
}

bool SocketThread::isConnected() const
{
    return m_connected && m_socket && m_socket->state() == QAbstractSocket::ConnectedState;
}

void SocketThread::sendData(const QByteArray& data)
{
    if (!isConnected()) {
        emit errorOccurred("Socket未连接，无法发送数据");
        return;
    }
    
    // 将数据加入发送队列
    QMutexLocker locker(&m_sendMutex);
    m_sendQueue.enqueue(data);
    
    // 唤醒工作线程
    m_condition.wakeAll();
}

SocketThread::SocketConfig SocketThread::getCurrentConfig() const
{
    return m_config;
}

QString SocketThread::getConnectionInfo() const
{
    if (m_socket && m_connected) {
        return QString("%1:%2 -> %3:%4")
                .arg(m_socket->localAddress().toString())
                .arg(m_socket->localPort())
                .arg(m_socket->peerAddress().toString())
                .arg(m_socket->peerPort());
    }
    return "未连接";
}

void SocketThread::run()
{
    qDebug() << "Socket线程开始运行";
    
    while (!m_stopRequested) {
        // 处理发送队列
        processSendQueue();
        
        // 等待新的发送请求或退出信号
        QMutexLocker locker(&m_mutex);
        if (m_sendQueue.isEmpty() && !m_stopRequested) {
            m_condition.wait(&m_mutex, 100); // 最多等待100ms
        }
    }
    
    qDebug() << "Socket线程结束运行";
}

void SocketThread::handleConnected()
{
    m_connected = true;
    emit connectionStateChanged(true);
    emit connected();
    qDebug() << "Socket连接建立:" << getConnectionInfo();
}

void SocketThread::handleDisconnected()
{
    bool wasConnected = m_connected;
    m_connected = false;
    
    if (wasConnected) {
        emit connectionStateChanged(false);
        emit disconnected();
        qDebug() << "Socket连接断开";
    }
}

void SocketThread::handleReadyRead()
{
    if (!m_socket) {
        return;
    }
    
    QByteArray data = m_socket->readAll();
    if (!data.isEmpty()) {
        qDebug() << "Socket接收数据:" << data.toHex(' ');
        emit dataReceived(data);
    }
}

void SocketThread::handleErrorOccurred(QAbstractSocket::SocketError error)
{
    QString errorString = socketErrorToString(error);
    qWarning() << "Socket错误:" << errorString;
    emit errorOccurred(errorString);
}

void SocketThread::attemptReconnect()
{
    if (m_shouldReconnect && !m_connected) {
        qDebug() << "尝试重新连接...";
        
        if (m_socket) {
            m_socket->connectToHost(QHostAddress(m_config.hostAddress), m_config.port);
        }
    }
}

void SocketThread::processSendQueue()
{
    QMutexLocker locker(&m_sendMutex);
    
    while (!m_sendQueue.isEmpty() && isConnected()) {
        QByteArray data = m_sendQueue.dequeue();
        locker.unlock();
        
        // 发送数据
        qint64 bytesWritten = m_socket->write(data);
        if (bytesWritten == data.size()) {
            if (m_socket->waitForBytesWritten(3000)) {
                qDebug() << "Socket发送数据成功:" << data.toHex(' ');
                emit dataSent(data);
            } else {
                qWarning() << "Socket发送数据超时";
                emit errorOccurred("数据发送超时");
            }
        } else {
            qWarning() << "Socket数据发送不完整";
            emit errorOccurred("数据发送不完整");
        }
        
        locker.relock();
    }
}

void SocketThread::cleanupSocket()
{
    if (m_socket) {
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    
    // 清空发送队列
    QMutexLocker locker(&m_sendMutex);
    m_sendQueue.clear();
}

void SocketThread::setupSocket()
{
    cleanupSocket();
    
    m_socket = new QTcpSocket(this);
    
    // 连接信号槽
    connect(m_socket, &QTcpSocket::connected, this, &SocketThread::handleConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &SocketThread::handleDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &SocketThread::handleReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &SocketThread::handleErrorOccurred);
}

QString SocketThread::socketErrorToString(QAbstractSocket::SocketError error)
{
    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
        return "连接被拒绝";
    case QAbstractSocket::RemoteHostClosedError:
        return "远程主机关闭连接";
    case QAbstractSocket::HostNotFoundError:
        return "主机未找到";
    case QAbstractSocket::SocketTimeoutError:
        return "Socket超时";
    case QAbstractSocket::NetworkError:
        return "网络错误";
    default:
        return "未知错误";
    }
} 