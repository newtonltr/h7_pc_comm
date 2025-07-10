#include "socket_thread.h"
#include <QDebug>

SocketThread::SocketThread(QObject *parent)
    : QObject(parent)
    , m_workerThread(nullptr)
    , m_worker(nullptr)
    , m_connected(false)
{
    setupWorker();
}

SocketThread::~SocketThread()
{
    cleanupWorker();
}

bool SocketThread::connectToHost(const SocketConfig& config)
{
    if (!m_worker) {
        qWarning() << "Socket Worker not initialized";
        return false;
    }
    
    m_config = config;
    
    // 通过信号槽调用 Worker 的连接方法
    QMetaObject::invokeMethod(m_worker, "connectToHost", 
                             Qt::QueuedConnection,
                             Q_ARG(SocketWorker::SocketConfig, config));
    
    return true; // 实际结果通过 onWorkerConnectResult 信号返回
}

void SocketThread::disconnectFromHost()
{
    if (m_worker) {
        QMetaObject::invokeMethod(m_worker, "disconnectFromHost", Qt::QueuedConnection);
    }
    m_connected = false;
}

bool SocketThread::isConnected() const
{
    return m_connected;
}

void SocketThread::sendData(const QByteArray& data)
{
    if (!m_worker) {
        emit errorOccurred("Socket Worker not initialized");
        return;
    }
    
    QMetaObject::invokeMethod(m_worker, "sendData", 
                             Qt::QueuedConnection,
                             Q_ARG(QByteArray, data));
}

SocketThread::SocketConfig SocketThread::getCurrentConfig() const
{
    return m_config;
}

QString SocketThread::getConnectionInfo() const
{
    // 由于 Worker 在另一个线程，暂时返回配置信息
    if (m_connected) {
        return QString("连接到 %1:%2").arg(m_config.hostAddress).arg(m_config.port);
    }
    return "未连接";
}

void SocketThread::setupWorker()
{
    // 创建工作线程
    m_workerThread = new QThread(this);
    
    // 创建 Worker 对象（无父对象）
    m_worker = new SocketWorker();
    
    // 将 Worker 移动到工作线程
    m_worker->moveToThread(m_workerThread);
    
    // 连接 Worker 信号到本对象的信号（转发）
    connect(m_worker, &SocketWorker::dataReceived,
            this, &SocketThread::dataReceived);
    connect(m_worker, &SocketWorker::connectionStateChanged,
            this, &SocketThread::connectionStateChanged);
    connect(m_worker, &SocketWorker::errorOccurred,
            this, &SocketThread::errorOccurred);
    connect(m_worker, &SocketWorker::dataSent,
            this, &SocketThread::dataSent);
    connect(m_worker, &SocketWorker::connected,
            this, &SocketThread::connected);
    connect(m_worker, &SocketWorker::disconnected,
            this, &SocketThread::disconnected);
    connect(m_worker, &SocketWorker::connectResult,
            this, &SocketThread::onWorkerConnectResult);
    
    // 连接线程启动信号到 Worker 初始化
    connect(m_workerThread, &QThread::started,
            m_worker, &SocketWorker::initialize);
    
    // 连接线程结束信号到 Worker 清理
    connect(m_workerThread, &QThread::finished,
            m_worker, &SocketWorker::cleanup);
    connect(m_workerThread, &QThread::finished,
            m_worker, &SocketWorker::deleteLater);
    
    // 启动工作线程
    m_workerThread->start();
    
    qDebug() << "Socket线程系统已初始化";
}

void SocketThread::cleanupWorker()
{
    if (m_workerThread) {
        // 停止工作线程
        m_workerThread->quit();
        if (!m_workerThread->wait(3000)) {
            qWarning() << "Socket工作线程未能正常退出，强制终止";
            m_workerThread->terminate();
            m_workerThread->wait(1000);
        }
        
        m_workerThread->deleteLater();
        m_workerThread = nullptr;
    }
    
    // Worker 对象会通过 deleteLater 自动清理
    m_worker = nullptr;
    
    qDebug() << "Socket线程系统已清理";
}

void SocketThread::onWorkerConnectResult(bool success, const QString& message)
{
    m_connected = success;
    if (!success) {
        emit errorOccurred(message);
    }
    qDebug() << "Socket连接结果:" << (success ? "成功" : "失败") << message;
} 