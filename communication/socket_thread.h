#ifndef SOCKET_THREAD_H
#define SOCKET_THREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QTimer>

class SocketThread : public QThread
{
    Q_OBJECT

public:
    explicit SocketThread(QObject *parent = nullptr);
    ~SocketThread();

    // Socket配置参数
    struct SocketConfig {
        QString hostAddress;        // 目标IP地址
        quint16 port;              // 目标端口
        int connectTimeout;        // 连接超时时间(ms)
        int readTimeout;           // 读取超时时间(ms)
        bool autoReconnect;        // 是否自动重连
        int reconnectInterval;     // 重连间隔(ms)
        
        SocketConfig() {
            hostAddress = "192.168.1.100";
            port = 8080;
            connectTimeout = 5000;
            readTimeout = 3000;
            autoReconnect = true;
            reconnectInterval = 3000;
        }
    };

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

protected:
    void run() override;

private slots:
    void handleConnected();
    void handleDisconnected();
    void handleReadyRead();
    void handleErrorOccurred(QAbstractSocket::SocketError error);
    void attemptReconnect();

private:
    QTcpSocket* m_socket;
    SocketConfig m_config;
    bool m_connected;
    bool m_shouldReconnect;
    
    // 线程同步
    QMutex m_mutex;
    QWaitCondition m_condition;
    bool m_stopRequested;
    
    // 发送队列
    QQueue<QByteArray> m_sendQueue;
    QMutex m_sendMutex;
    
    // 重连定时器
    QTimer* m_reconnectTimer;
    
    // 内部方法
    void processSendQueue();
    void cleanupSocket();
    void setupSocket();
    QString socketErrorToString(QAbstractSocket::SocketError error);
};

#endif // SOCKET_THREAD_H 