#ifndef PROTOCOL_FRAME_H
#define PROTOCOL_FRAME_H

#include <QByteArray>
#include <QString>
#include <QHostAddress>
#include <cstdint>

extern "C" {
#include "../pc_protocol.h"
}

class ProtocolFrame
{
public:
    // 构造函数
    ProtocolFrame();
    ~ProtocolFrame();

    // 静态工厂方法 - 创建MAC地址设置帧
    static QByteArray buildMacSetFrame(uint8_t macHighByte);
    
    // 静态工厂方法 - 创建IP地址设置帧  
    static QByteArray buildIpSetFrame(const QString& ipAddress);
    // 静态工厂方法 - 创建子网掩码设置帧  
    static QByteArray buildMaskSetFrame(const QString& maskAddress);
    // 静态工厂方法 - 创建网关地址设置帧  
    static QByteArray buildGatewaySetFrame(const QString& gatewayAddress);

    // 静态工厂方法 - 创建VCU参数设置帧  
    static QByteArray buildVcuParamSetFrame(const QString& rearObstacleDistance, const QString& speedCorrectionFactor);

    
    // 解析接收到的帧数据
    struct ParsedData {
        uint8_t sourceAddr;
        uint8_t targetAddr;
        uint16_t functionCode;
        QByteArray data;
        bool isValid;
        QString errorMessage;
    };
    
    static ParsedData parseFrame(const QByteArray& frameData);
    
    // 验证帧格式和CRC
    static bool validateFrame(const QByteArray& frameData);
    
    // 工具方法 - IP字符串转字节数组
    static QByteArray ipStringToBytes(const QString& ipAddress);
    
    // 工具方法 - 字节数组转IP字符串
    static QString bytesToIpString(const QByteArray& ipBytes);
    
    // 工具方法 - MAC字节转字符串显示
    static QString macBytesToString(uint8_t macHighByte);

private:
    // 内部辅助方法
    static QByteArray buildFrame(uint16_t functionCode, const QByteArray& data);
    static uint16_t calculateCRC(const QByteArray& data);
};

#endif // PROTOCOL_FRAME_H 