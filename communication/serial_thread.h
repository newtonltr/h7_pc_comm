#ifndef SERIAL_THREAD_H
#define SERIAL_THREAD_H

#include <QObject>
#include <QThread>
#include <QByteArray>
#include "serial_worker.h"

class SerialThread : public QObject
{
    Q_OBJECT

public:
    // 使用 SerialWorker 的配置结构
    using SerialConfig = SerialWorker::SerialConfig;

    explicit SerialThread(QObject *parent = nullptr);
    ~SerialThread();

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

private slots:
    // 处理 Worker 信号
    void onWorkerOpenResult(bool success, const QString& message);

private:
    QThread* m_workerThread;
    SerialWorker* m_worker;
    SerialConfig m_config;
    bool m_connected;
    
    // 内部方法
    void setupWorker();
    void cleanupWorker();
};

#endif // SERIAL_THREAD_H 