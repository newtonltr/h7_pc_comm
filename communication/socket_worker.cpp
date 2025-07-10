#include "socket_worker.h"
#include <QDebug>
#include <QHostAddress>

SocketWorker::SocketWorker(QObject *parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_connected(false)
    , m_shouldReconnect(false)
    , m_sendTimer(nullptr)
    , m_reconnectTimer(nullptr)
{
}

SocketWorker::~SocketWorker()
{
    cleanup();
}

void SocketWorker::initialize()
{
    // 创建发送处理定时器
    m_sendTimer = new QTimer(this);
    connect(m_sendTimer, &QTimer::timeout, this, &SocketWorker::processSendQueue);
    m_sendTimer->setSingleShot(false);
    m_sendTimer->setInterval(10); // 10ms 检查一次发送队列
    
    // 创建重连定时器
    m_reconnectTimer = new QTimer(this);
    connect(m_reconnectTimer, &QTimer::timeout, this, &SocketWorker::attemptReconnect);
    m_reconnectTimer->setSingleShot(true);
}

void SocketWorker::cleanup()
{
    m_shouldReconnect = false;
    disconnectFromHost();
    
    if (m_sendTimer) {
        m_sendTimer->stop();
        m_sendTimer->deleteLater();
        m_sendTimer = nullptr;
    }
    
    if (m_reconnectTimer) {
        m_reconnectTimer->stop();
        m_reconnectTimer->deleteLater();
        m_reconnectTimer = nullptr;
    }
}

void SocketWorker::connectToHost(const SocketConfig& config)
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
        
        // 启动发送定时器
        if (m_sendTimer) {
            m_sendTimer->start();
        }
        
        qDebug() << "Socket连接成功:" << getConnectionInfo();
        emit connectResult(true, QString("Socket连接成功: %1").arg(getConnectionInfo()));
    } else {
        QString errorMsg = QString("无法连接到 %1:%2 - %3")
                          .arg(config.hostAddress)
                          .arg(config.port)
                          .arg(m_socket->errorString());
        qWarning() << errorMsg;
        emit errorOccurred(errorMsg);
        emit connectResult(false, errorMsg);
        
        // 如果需要自动重连，启动重连定时器
        if (m_shouldReconnect) {
            m_reconnectTimer->start(config.reconnectInterval);
        }
    }
}

void SocketWorker::disconnectFromHost()
{
    m_shouldReconnect = false;
    
    if (m_reconnectTimer) {
        m_reconnectTimer->stop();
    }
    
    if (m_socket && m_connected) {
        m_connected = false;
        
        // 停止发送定时器
        if (m_sendTimer) {
            m_sendTimer->stop();
        }
        
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

void SocketWorker::sendData(const QByteArray& data)
{
    if (!m_connected || !m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        emit errorOccurred("Socket未连接，无法发送数据");
        return;
    }
    
    // 将数据加入发送队列
    QMutexLocker locker(&m_sendMutex);
    m_sendQueue.enqueue(data);
}

void SocketWorker::attemptReconnect()
{
    if (m_shouldReconnect && !m_connected) {
        qDebug() << "尝试重新连接...";
        
        // 清理旧的socket
        cleanupSocket();
        
        // 重新连接
        setupSocket();
        m_socket->connectToHost(QHostAddress(m_config.hostAddress), m_config.port);
        
        if (m_socket->waitForConnected(m_config.connectTimeout)) {
            m_connected = true;
            emit connectionStateChanged(true);
            emit connected();
            
            if (m_sendTimer) {
                m_sendTimer->start();
            }
            
            qDebug() << "重连成功:" << getConnectionInfo();
        } else {
            qDebug() << "重连失败，将在" << m_config.reconnectInterval << "ms后再次尝试";
            m_reconnectTimer->start(m_config.reconnectInterval);
        }
    }
}

void SocketWorker::handleConnected()
{
    m_connected = true;
    emit connectionStateChanged(true);
    emit connected();
    qDebug() << "Socket连接建立:" << getConnectionInfo();
}

void SocketWorker::handleDisconnected()
{
    bool wasConnected = m_connected;
    m_connected = false;
    
    if (m_sendTimer) {
        m_sendTimer->stop();
    }
    
    if (wasConnected) {
        emit connectionStateChanged(false);
        emit disconnected();
        qDebug() << "Socket连接断开";
        
        // 如果需要自动重连且不是主动断开
        if (m_shouldReconnect) {
            qDebug() << "启动自动重连，间隔:" << m_config.reconnectInterval << "ms";
            m_reconnectTimer->start(m_config.reconnectInterval);
        }
    }
}

void SocketWorker::handleReadyRead()
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

void SocketWorker::handleErrorOccurred(QAbstractSocket::SocketError error)
{
    QString errorString = socketErrorToString(error);
    qWarning() << "Socket错误:" << errorString;
    emit errorOccurred(errorString);
}

void SocketWorker::processSendQueue()
{
    if (!m_connected || !m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        return;
    }
    
    QMutexLocker locker(&m_sendMutex);
    
    while (!m_sendQueue.isEmpty()) {
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

void SocketWorker::cleanupSocket()
{
    if (m_socket) {
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    
    // 清空发送队列
    QMutexLocker locker(&m_sendMutex);
    m_sendQueue.clear();
}

void SocketWorker::setupSocket()
{
    cleanupSocket();
    
    m_socket = new QTcpSocket();  // 无父对象，在工作线程中运行
    
    // 连接信号槽
    connect(m_socket, &QTcpSocket::connected, this, &SocketWorker::handleConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &SocketWorker::handleDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &SocketWorker::handleReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &SocketWorker::handleErrorOccurred);
}

QString SocketWorker::socketErrorToString(QAbstractSocket::SocketError error)
{
    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
        return "连接被拒绝";
    case QAbstractSocket::RemoteHostClosedError:
        return "远程主机关闭连接";
    case QAbstractSocket::HostNotFoundError:
        return "主机未找到";
    case QAbstractSocket::SocketAccessError:
        return "Socket访问错误";
    case QAbstractSocket::SocketResourceError:
        return "Socket资源错误";
    case QAbstractSocket::SocketTimeoutError:
        return "Socket超时";
    case QAbstractSocket::DatagramTooLargeError:
        return "数据报过大";
    case QAbstractSocket::NetworkError:
        return "网络错误";
    case QAbstractSocket::AddressInUseError:
        return "地址已被使用";
    case QAbstractSocket::SocketAddressNotAvailableError:
        return "Socket地址不可用";
    case QAbstractSocket::UnsupportedSocketOperationError:
        return "不支持的Socket操作";
    case QAbstractSocket::ProxyAuthenticationRequiredError:
        return "代理认证失败";
    case QAbstractSocket::SslHandshakeFailedError:
        return "SSL握手失败";
    case QAbstractSocket::UnfinishedSocketOperationError:
        return "未完成的Socket操作";
    case QAbstractSocket::ProxyConnectionRefusedError:
        return "代理连接被拒绝";
    case QAbstractSocket::ProxyConnectionClosedError:
        return "代理连接关闭";
    case QAbstractSocket::ProxyConnectionTimeoutError:
        return "代理连接超时";
    case QAbstractSocket::ProxyNotFoundError:
        return "代理未找到";
    case QAbstractSocket::ProxyProtocolError:
        return "代理协议错误";
    case QAbstractSocket::OperationError:
        return "操作错误";
    case QAbstractSocket::SslInternalError:
        return "SSL内部错误";
    case QAbstractSocket::SslInvalidUserDataError:
        return "SSL无效用户数据";
    case QAbstractSocket::TemporaryError:
        return "临时错误";
    default:
        return "未知错误";
    }
}

QString SocketWorker::getConnectionInfo() const
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