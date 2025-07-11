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
    PC_VCU_INFO_GET               = 0x0063,  /// 自定义功能码，获取vcu信息，数据长度为0
    PC_MAC_ADDR_SET               = 0x01F1,  /// 自定义功能码，设置mac地址，数据长度为6字节
    PC_IP_ADDR_SET                = 0x01F2,  /// 自定义功能码，设置ip地址，数据长度为4字节
    PC_MASK_ADDR_SET              = 0x01F3,  /// 自定义功能码，设置子网掩码，数据长度为4字节
    PC_GATEWAY_ADDR_SET           = 0x01F4,  /// 自定义功能码，设置网关地址，数据长度为4字节
    PC_HARDFAULT_INFO_GET         = 0x01F5,  /// 自定义功能码，获取HardFault故障信息，数据长度为0

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

#pragma pack(1)
// HardFault故障信息数据结构
typedef struct {
    uint32_t magic_number;    // 魔数标识 0xDEADBEEF
    uint32_t timestamp;       // 时间戳（运行时间）
    uint32_t sp_value;        // 堆栈指针值  
    uint32_t r0_value;        // r0寄存器值
    uint32_t r1_value;        // r1寄存器值
    uint32_t r2_value;        // r2寄存器值
    uint32_t r3_value;        // r3寄存器值
    uint32_t r12_value;       // r12寄存器值
    uint32_t lr_value;        // 链接寄存器值
    uint32_t pc_value;        // 程序计数器值
    uint32_t xpsr_value;       // xpsr寄存器值
    uint32_t fault_count;     // 故障计数器
    uint32_t reserved[2];     // 保留字段，保持32字节对齐
} hardfault_info_t;
#pragma pack()

#pragma pack(1)
//vcu综合信息
struct state_def_t{
	char software_version[16];
	char hardware_version[16];
	uint8_t electric;
	float voltage;
	float current;
	float wireless_voltage;
	float wireless_current;
	float temperature;
	float humidity;
	uint8_t ip[4];
	uint16_t port;
	uint8_t crash_head;
	uint8_t crash_rear;
	uint8_t proximity;	//接近开关
	uint8_t emergency_stop;
	uint8_t ctrl_mode;
	uint8_t clear_mode;
	float joy_vc;					
	float joy_vw;
	float twist_vc;	//线速度反馈
	float twist_vw;	//角速度反馈
	float bat_temperature;
	float air_h2s;
	float air_co;
	float air_o2;
	float air_ex;
	float drv0_current_ch0;
	float drv0_current_ch1;
	float drv1_current_ch0;
	float drv1_current_ch1;	
	float cmd_vc;			//指令线速度
	float cmd_vw;			//指令角速度
	float joy_ch0;			//遥控器通道值0
	float joy_ch1;
	float joy_ch2;
	float joy_ch3;
	char boot_version[16];
	uint32_t serial_number[3];
	char dev_lock_sta;		//设备锁
	uint8_t fire_sensor;	//火焰传感
	uint8_t fall_sensor;
	float air_edc;
	float air_c2h4;
	float air_hcl;
	float air_cl2;
	float air_c3h6;
	float air_h2;
	float air_temp;
	float air_hum;
	float air_sf6;
	float cocl2;
	float c2h6o;
//新增
	float ch4;
	uint32_t  sts_bms;
	uint8_t   flag_air_invail;
	uint8_t   ultrasonic_f;//前超声波避障触发
	uint8_t   ultrasonic_r;//后超声波避障触发
	uint8_t   ultrasonic_tl;//左转超声波避障触发
	uint8_t   ultrasonic_tr;//右转超声波避障触发
	float lf_motor_current;
	float rf_motor_current;
	float rr_motor_current;
	float lr_motor_current;	
	uint8_t  lifter_h;
};
#pragma pack()


unsigned int CRC16(uint8_t *puchMsg,unsigned int usDataLen);
uint8_t checkRxCRC(uint8_t *rxBuf, uint8_t rxLen, uint16_t rxCRC);

#endif

