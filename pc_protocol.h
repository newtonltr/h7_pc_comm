#ifndef __PC_PROTOCOL_H__
#define __PC_PROTOCOL_H__

#include <stdint.h>


/*
    pc通信协议简介：
    1. 帧头：0xff
    2. 源地址：pc发给mcu, 源地址为0x03，mcu发给pc, 源地址为0x01
    3. 目标地址：pc发给mcu, 目标地址为0x01，mcu发给pc, 目标地址为0x03
    4. 功能码：定义在protocol_function_code_e枚举中
    5. 数据长度：看protocol_function_code_e后的注释
    6. 数据：根据功能码不同，数据不同，长度由数据长度字段指定
    7. 校验码：校验码为pc_comm_protocol__head_t长度+数据长度，使用modbus，lsb计算
 */

#define pc_protocol_head 0xff

#define pc_addr 0x03    // pc地址
#define mcu_addr 0x01   // mcu地址
/// @brief pc通信功能码枚举
enum protocol_function_code_e
{
    PC_MAC_ADDR_SET               = 0x01F1,  /// 自定义功能码，设置mac地址，数据长度为6字节
    PC_IP_ADDR_SET                = 0x01F2,  /// 自定义功能码，设置ip地址，数据长度为4字节
    PC_MASK_ADDR_SET              = 0x01F3,  /// 自定义功能码，设置子网掩码，数据长度为4字节
    PC_GATEWAY_ADDR_SET           = 0x01F4,  /// 自定义功能码，设置网关地址，数据长度为4字节
    PC_VCU_PARAM_SET              = 0xFFFD,  /// 自定义功能码，设置VCU参数：后避障距离和速度校正系数，数据长度为8字节        
    
};

#pragma pack(1)
struct pc_comm_protocol__head_t
{
	uint8_t head;   // 帧头
	uint8_t source_addr; // 源地址
	uint8_t target_addr; // 目标地址
	uint16_t function_code; // 功能码
	uint16_t data_length; // 数据长度
};
#pragma pack()

unsigned int CRC16(uint8_t *puchMsg,unsigned int usDataLen);
uint8_t checkRxCRC(uint8_t *rxBuf, uint8_t rxLen, uint16_t rxCRC);

#endif

