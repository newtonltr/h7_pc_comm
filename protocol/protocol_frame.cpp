#include "protocol_frame.h"
#include <QStringList>
#include <QRegularExpression>
#include <QDebug>

ProtocolFrame::ProtocolFrame()
{
    // 默认构造函数
}

ProtocolFrame::~ProtocolFrame()
{
    // 析构函数
}

QByteArray ProtocolFrame::buildMacSetFrame(uint8_t macHighByte)
{
    // MAC地址数据：{0x02, 0x00, 0x00, 0x00, 0x00, macHighByte}
    // 只有mac_addr[5]是可变的，其他字节固定
    QByteArray macData;
    macData.append(static_cast<char>(0x02));  // mac_addr[0] = 2 (固定)
    macData.append(static_cast<char>(0x00));  // mac_addr[1] = 0 (固定)
    macData.append(static_cast<char>(0x00));  // mac_addr[2] = 0 (固定)
    macData.append(static_cast<char>(0x00));  // mac_addr[3] = 0 (固定)
    macData.append(static_cast<char>(0x00));  // mac_addr[4] = 0 (固定)
    macData.append(static_cast<char>(macHighByte)); // mac_addr[5] = 用户输入 (可变)
    
    return buildFrame(PC_MAC_ADDR_SET, macData);
}

QByteArray ProtocolFrame::buildIpSetFrame(const QString& ipAddress)
{
    QByteArray ipData = ipStringToBytes(ipAddress);
    if (ipData.isEmpty()) {
        qWarning() << "Invalid IP address format:" << ipAddress;
        return QByteArray();
    }
    
    return buildFrame(PC_IP_ADDR_SET, ipData);
}

QByteArray ProtocolFrame::buildMaskSetFrame(const QString& maskAddress)
{
    QByteArray maskData = ipStringToBytes(maskAddress);
    if (maskData.isEmpty()) {
        qWarning() << "Invalid mask address format:" << maskAddress;
        return QByteArray();
    }

    return buildFrame(PC_MASK_ADDR_SET, maskData);
}

QByteArray ProtocolFrame::buildGatewaySetFrame(const QString& gatewayAddress)
{
    QByteArray gatewayData = ipStringToBytes(gatewayAddress);
    if (gatewayData.isEmpty()) {
        qWarning() << "Invalid gateway address format:" << gatewayAddress;
        return QByteArray();
    }

    return buildFrame(PC_GATEWAY_ADDR_SET, gatewayData);
}

QByteArray ProtocolFrame::buildVcuParamSetFrame(const QString& rearObstacleDistance, const QString& speedCorrectionFactor)
{
    QByteArray vcuParamData;
    float rearObstacleDistanceFloat = rearObstacleDistance.toFloat();   
    float speedCorrectionFactorFloat = speedCorrectionFactor.toFloat();
    vcuParamData.append(reinterpret_cast<const char*>(&rearObstacleDistanceFloat), sizeof(float));
    vcuParamData.append(reinterpret_cast<const char*>(&speedCorrectionFactorFloat), sizeof(float));
    if (vcuParamData.isEmpty()) {
        qWarning() << "Invalid vcu param format:" << rearObstacleDistance << "or" << speedCorrectionFactor;
        return QByteArray();
    }

    return buildFrame(PC_VCU_PARAM_SET, vcuParamData);
}

QByteArray ProtocolFrame::buildHardFaultInfoGetFrame()
{
    // HardFault信息获取请求，数据长度为0
    QByteArray emptyData;
    return buildFrame(PC_HARDFAULT_INFO_GET, emptyData);
}

QByteArray ProtocolFrame::buildVcuInfoGetFrame()
{
    // VCU信息获取请求，数据长度为0
    QByteArray emptyData;
    return buildFrame(PC_VCU_INFO_GET, emptyData);
}

ProtocolFrame::ParsedData ProtocolFrame::parseFrame(const QByteArray& frameData)
{
    ParsedData result;
    result.isValid = false;
    
    // 检查最小长度
    if (frameData.size() < static_cast<int>(sizeof(pc_comm_protocol__head_t) + 2)) {
        result.errorMessage = "帧长度不足";
        return result;
    }
    
    // 验证帧头
    if (static_cast<uint8_t>(frameData[0]) != pc_protocol_head) {
        result.errorMessage = "帧头错误";
        return result;
    }
    
    // 解析协议头
    const pc_comm_protocol__head_t* header = 
        reinterpret_cast<const pc_comm_protocol__head_t*>(frameData.constData());
    
    result.sourceAddr = header->source_addr;
    result.targetAddr = header->target_addr;
    result.functionCode = header->function_code;
    
    // 提取数据部分
    int headerSize = sizeof(pc_comm_protocol__head_t);
    int dataLength = header->data_length;
    
    if (frameData.size() < headerSize + dataLength + 2) {
        result.errorMessage = "数据长度不匹配";
        return result;
    }
    
    result.data = frameData.mid(headerSize, dataLength);
    
    // 验证CRC
    if (!validateFrame(frameData)) {
        result.errorMessage = "CRC校验失败";
        return result;
    }
    
    result.isValid = true;
    return result;
}

bool ProtocolFrame::validateFrame(const QByteArray& frameData)
{
    if (frameData.size() < static_cast<int>(sizeof(pc_comm_protocol__head_t) + 2)) {
        return false;
    }
    
    const pc_comm_protocol__head_t* header = 
        reinterpret_cast<const pc_comm_protocol__head_t*>(frameData.constData());
    
    int headerSize = sizeof(pc_comm_protocol__head_t);
    int dataLength = header->data_length;
    int expectedSize = headerSize + dataLength + 2;
    
    if (frameData.size() != expectedSize) {
        return false;
    }
    
    // 提取接收到的CRC
    uint16_t receivedCRC;
    memcpy(&receivedCRC, frameData.constData() + headerSize + dataLength, 2);
    
    // 计算CRC
    QByteArray dataForCRC = frameData.left(headerSize + dataLength);
    uint16_t calculatedCRC = calculateCRC(dataForCRC);
    
    return receivedCRC == calculatedCRC;
}

QByteArray ProtocolFrame::ipStringToBytes(const QString& ipAddress)
{
    QStringList parts = ipAddress.split('.');
    if (parts.size() != 4) {
        return QByteArray();
    }
    
    // 先按正常顺序转换
    QByteArray temp;
    for (const QString& part : parts) {
        bool ok;
        int value = part.toInt(&ok);
        if (!ok || value < 0 || value > 255) {
            return QByteArray();
        }
        temp.append(static_cast<char>(value));
    }
    
    // 将字节顺序反转以符合协议要求
    // 192.168.110.111 -> {0xC0, 0xA8, 0x6E, 0x6F} -> {0x6F, 0x6E, 0xA8, 0xC0}
    QByteArray result;
    for (int i = temp.size() - 1; i >= 0; --i) {
        result.append(temp[i]);
    }
    
    return result;
}

QString ProtocolFrame::bytesToIpString(const QByteArray& ipBytes)
{
    if (ipBytes.size() != 4) {
        return QString();
    }
    
    // 由于发送时字节序是倒序的，接收时需要反转回来
    // {0x6F, 0x6E, 0xA8, 0xC0} -> 192.168.110.111
    return QString("%1.%2.%3.%4")
            .arg(static_cast<uint8_t>(ipBytes[3]))  // 倒序：ipBytes[3]
            .arg(static_cast<uint8_t>(ipBytes[2]))  // 倒序：ipBytes[2]
            .arg(static_cast<uint8_t>(ipBytes[1]))  // 倒序：ipBytes[1]
            .arg(static_cast<uint8_t>(ipBytes[0])); // 倒序：ipBytes[0]
}

QString ProtocolFrame::macBytesToString(uint8_t macHighByte)
{
    return QString("02:00:00:00:00:%1").arg(macHighByte, 2, 16, QChar('0')).toUpper();
}

QByteArray ProtocolFrame::buildFrame(uint16_t functionCode, const QByteArray& data)
{
    // 构建协议头
    pc_comm_protocol__head_t header;
    header.head = pc_protocol_head;
    header.source_addr = pc_addr;
    header.target_addr = mcu_addr;
    header.function_code = functionCode;
    header.data_length = static_cast<uint16_t>(data.size());
    
    // 构建完整帧
    QByteArray frame;
    frame.append(reinterpret_cast<const char*>(&header), sizeof(header));
    frame.append(data);
    
    // 计算并添加CRC
    uint16_t crc = calculateCRC(frame);
    frame.append(reinterpret_cast<const char*>(&crc), 2);
    
    return frame;
}

uint16_t ProtocolFrame::calculateCRC(const QByteArray& data)
{
    return CRC16(reinterpret_cast<uint8_t*>(const_cast<char*>(data.constData())), 
                 static_cast<unsigned int>(data.size()));
} 