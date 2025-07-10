#include "serial_thread.h"
#include <QDebug>

SerialThread::SerialThread(QObject *parent)
    : QObject(parent)
    , m_workerThread(nullptr)
    , m_worker(nullptr)
    , m_connected(false)
{
    setupWorker();
}

SerialThread::~SerialThread()
{
    cleanupWorker();
}

bool SerialThread::openSerial(const SerialConfig& config)
{
    if (!m_worker) {
        qWarning() << "Worker not initialized";
        return false;
    }
    
    m_config = config;
    
    // 通过信号槽调用 Worker 的打开串口方法
    // 使用 QueuedConnection 确保在工作线程中执行
    QMetaObject::invokeMethod(m_worker, "openSerial", 
                             Qt::QueuedConnection,
                             Q_ARG(SerialWorker::SerialConfig, config));
    
    return true; // 实际结果通过 onWorkerOpenResult 信号返回
}

void SerialThread::closeSerial()
{
    if (m_worker) {
        QMetaObject::invokeMethod(m_worker, "closeSerial", Qt::QueuedConnection);
    }
    m_connected = false;
}

bool SerialThread::isConnected() const
{
    return m_connected;
}

void SerialThread::sendData(const QByteArray& data)
{
    if (!m_worker) {
        emit errorOccurred("Worker not initialized");
        return;
    }
    
    QMetaObject::invokeMethod(m_worker, "sendData", 
                             Qt::QueuedConnection,
                             Q_ARG(QByteArray, data));
}

QStringList SerialThread::getAvailablePorts()
{
    return SerialWorker::getAvailablePorts();
}

SerialThread::SerialConfig SerialThread::getCurrentConfig() const
{
    return m_config;
}

void SerialThread::setupWorker()
{
    // 创建工作线程
    m_workerThread = new QThread(this);
    
    // 创建 Worker 对象（无父对象）
    m_worker = new SerialWorker();
    
    // 将 Worker 移动到工作线程
    m_worker->moveToThread(m_workerThread);
    
    // 连接 Worker 信号到本对象的信号（转发）
    connect(m_worker, &SerialWorker::dataReceived,
            this, &SerialThread::dataReceived);
    connect(m_worker, &SerialWorker::connectionStateChanged,
            this, &SerialThread::connectionStateChanged);
    connect(m_worker, &SerialWorker::errorOccurred,
            this, &SerialThread::errorOccurred);
    connect(m_worker, &SerialWorker::dataSent,
            this, &SerialThread::dataSent);
    connect(m_worker, &SerialWorker::openResult,
            this, &SerialThread::onWorkerOpenResult);
    
    // 连接线程启动信号到 Worker 初始化
    connect(m_workerThread, &QThread::started,
            m_worker, &SerialWorker::initialize);
    
    // 连接线程结束信号到 Worker 清理
    connect(m_workerThread, &QThread::finished,
            m_worker, &SerialWorker::cleanup);
    connect(m_workerThread, &QThread::finished,
            m_worker, &SerialWorker::deleteLater);
    
    // 启动工作线程
    m_workerThread->start();
    
    qDebug() << "串口线程系统已初始化";
}

void SerialThread::cleanupWorker()
{
    if (m_workerThread) {
        // 停止工作线程
        m_workerThread->quit();
        if (!m_workerThread->wait(3000)) {
            qWarning() << "串口工作线程未能正常退出，强制终止";
            m_workerThread->terminate();
            m_workerThread->wait(1000);
        }
        
        m_workerThread->deleteLater();
        m_workerThread = nullptr;
    }
    
    // Worker 对象会通过 deleteLater 自动清理
    m_worker = nullptr;
    
    qDebug() << "串口线程系统已清理";
}

void SerialThread::onWorkerOpenResult(bool success, const QString& message)
{
    m_connected = success;
    if (!success) {
        emit errorOccurred(message);
    }
    qDebug() << "串口打开结果:" << (success ? "成功" : "失败") << message;
} 