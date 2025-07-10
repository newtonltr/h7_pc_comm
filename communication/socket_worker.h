#ifndef SOCKET_WORKER_H
#define SOCKET_WORKER_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QMutex>
#include <QQueue>
#include <QTimer>

class SocketWorker : public QObject
{
    Q_OBJECT

public:
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

    explicit SocketWorker(QObject *parent = nullptr);
    ~SocketWorker();

public slots:
    // Socket控制槽函数
    void connectToHost(const SocketConfig& config);
    void disconnectFromHost();
    void sendData(const QByteArray& data);
    
    // 初始化和清理
    void initialize();
    void cleanup();
    
    // 重连尝试
    void attemptReconnect();

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
    
    // 操作结果信号
    void connectResult(bool success, const QString& message);

private slots:
    void handleConnected();
    void handleDisconnected();
    void handleReadyRead();
    void handleErrorOccurred(QAbstractSocket::SocketError error);
    void processSendQueue();

private:
    QTcpSocket* m_socket;
    SocketConfig m_config;
    bool m_connected;
    bool m_shouldReconnect;
    
    // 发送队列
    QQueue<QByteArray> m_sendQueue;
    QMutex m_sendMutex;
    
    // 发送处理定时器
    QTimer* m_sendTimer;
    
    // 重连定时器
    QTimer* m_reconnectTimer;
    
    // 内部方法
    void cleanupSocket();
    void setupSocket();
    QString socketErrorToString(QAbstractSocket::SocketError error);
    QString getConnectionInfo() const;
};

#endif // SOCKET_WORKER_H 