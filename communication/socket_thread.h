#ifndef SOCKET_THREAD_H
#define SOCKET_THREAD_H

#include <QObject>
#include <QThread>
#include <QByteArray>
#include "socket_worker.h"

class SocketThread : public QObject
{
    Q_OBJECT

public:
    // 使用 SocketWorker 的配置结构
    using SocketConfig = SocketWorker::SocketConfig;

    explicit SocketThread(QObject *parent = nullptr);
    ~SocketThread();

    // 控制方法
    bool connectToHost(const SocketConfig& config);
    void disconnectFromHost();
    bool isConnected() const;
    
    // 数据发送
    void sendData(const QByteArray& data);
    
    // 获取当前配置
    SocketConfig getCurrentConfig() const;
    
    // 获取连接状态信息
    QString getConnectionInfo() const;

signals:
    // 数据接收信号
    void dataReceived(const QByteArray& data);
    
    // 连接状态改变信号
    void connectionStateChanged(bool connected);
    
    // 错误信号
    void errorOccurred(const QString& errorString);
    
    // 数据发送完成信号
    void dataSent(const QByteArray& data);
    
    // 连接成功信号
    void connected();
    
    // 断开连接信号
    void disconnected();

private slots:
    // 处理 Worker 信号
    void onWorkerConnectResult(bool success, const QString& message);

private:
    QThread* m_workerThread;
    SocketWorker* m_worker;
    SocketConfig m_config;
    bool m_connected;
    
    // 内部方法
    void setupWorker();
    void cleanupWorker();
};

#endif // SOCKET_THREAD_H 