#ifndef __CMD_UART_H__
#define __CMD_UART_H__
#include "stdint.h"

#define CMD_UART_RX_PKT 0xAA        // 保持原来的包头
#define CMD_UART_RX_NONE 0x00
#define CMD_UART_RX_LENGHT 0xC2     // 等待长度字节
#define CMD_UART_RX_EVENT 0xC3      // 等待事件字节
#define CMD_UART_RX_DATA 0xC4       // 等待数据字节
#define CMD_UART_RX_CRC 0xC5        // 等待CRC字节


#define CMD_EVENT_LCD_CTRL_SET 0x80
#define CMD_EVENT_LCD_CTRL_CLR 0x00


#define BT_PAIR_LED_SEG 0
#define BT_PAIR_LED_CMD 0x10

#define CAPS_LOCK_LED_SEG 0
#define CAPS_LOCK_LED_CMD 0x20

#define LOW_POWER_LED_SEG 0
#define LOW_POWER_LED_CMD 0x08
/**
 * @brief cmd uart event enum
 *
 */
enum
{
  CMD_UART_EVENT_NONE = 0,
  CMD_UART_EVENT_RX_DONE = 0x01,
  CMD_UART_EVENT_TX_DONE = 0x02,
  CMD_UART_LCD_MCU_UPDATE = 0x04,
  CMD_UART_EVENT_ERROR = 0x08
};

/**
 * @brief cmd uart event enum
 *
 */
typedef enum
{
  CMD_EVENT_NONE = 0,
  CMD_EVENT_JUMP_BOOT, // 跳转到boot
  CMD_EVENT_ACK,       // ack 回复，无数据包
  CMD_EVENT_POWER_ON,
  CMD_EVENT_PAIR_LED_CTRL,      // 1 on 0 off
  CMD_EVENT_BATLEVEL,           // 1byte 0-100
  CMD_EVENT_CAPS_LOCK_LED_CTRL, // 1 on 0 off
  CMD_EVENT_CONNECTED,
  CMD_EVENT_DISCONNECT,
  CMD_EVENT_POWER_DOWN, // 有效
  CMD_EVENT_WAKE_UP, // 有效
  CMD_EVENT_LCD_CTRL,     // byte 0 seg(bit7 ctrl set or clr), byte1 com value
  CMD_EVENT_LCD_CNS_CTRL, // byte 0 seg length(bit7 ctrl set or clr)  1 - 5 byte com1 - com5 ,seg set
  CMD_EVENT_CHGING,
  CMD_EVENT_CHG_FULL,
  CMD_EVENT_CLR_FLASH_LED, // 有效
} cmd_event_e;

typedef enum uart_int_status
{
    UART_RX_INT  = 0x10,
    UART_TX_INT  = 0x20,
    UART_ALL_INT = 0x30,

} uart_int_type;

typedef struct
{
  uint8_t pkt;      // 包头: 0xAA (保持原格式)
  uint8_t length;   // 数据包长度
  uint8_t evt;      // 事件类型
  uint8_t dat[16];  // 数据
  uint8_t crc;      // 校验码
} uart_t;

typedef struct 
{
  uint8_t last_bat;
  uint8_t current_bat;
}bat_info_t;

extern uint16_t cmd_uart_power_on_delay_send;
extern uint8_t enter_wft;
extern uint32_t _pin_wakeup;

void cmd_uart_pd_setting(void);
void cmd_uart_init(void);
void _uart_cb(void);

/**
 * @brief uart tick timer
 *
 */
void uart_timer(void);

/**
 * @brief check uart event
 *
 */
void uart_event_check(void);

/**
 * @brief get data crc
 *
 * @param buff data buffer
 * @param len data length
 * @return crc
 */
uint8_t get_crc(uint8_t* buff, uint16_t len);

/**
 * @brief cmd uart send pack
 *
 * @param evt event id
 * @param dat event data
 * @param length event data length
 */
void cmd_uart_send_pack(uint8_t evt, uint8_t* dat, uint8_t length);

/**
 * @brief cmd uart en queue
 *
 * @param evt event id
 * @param dat event data
 * @param len event data length
 */
void cmd_uart_en_queue(uint8_t evt, uint8_t* dat, uint8_t len);

uint8_t check_uart_status(void);

/**
 * @brief ctrl lcd seg
 *
 * @param s set or clr (0x80 set ,0 clr)
 * @param seg seg index
 * @param com com value
 */
void cmd_uart_send_lcd_ctrl(uint8_t s, uint8_t seg, uint8_t com);

/**
 * @brief cmd uart event run
 *
 */
void cmd_uart_event_run(void);

void cmd_uart_hw_init(void);

void cmd_uart_enter_pd(void);

uint8_t is_cmd_uart_queue_empty(void);

/**
 * @brief mcu update event
 *
 * @param l 0 clear update mode ,1 enter update mode
 */
void mcu_update_event_set(uint8_t l);

/**
 * @brief uart 查询电量
 */
void uart_query_battery_level(void);


/**
 * @brief 设置蓝牙连接状态指示灯
 *  
 * @param idx 0:蓝牙连接状态指示灯 1:大写锁定状态指示灯 2:低电指示灯
 * @param state 0:关 1:开
 */
void uart_led_ctrl(uint8_t idx, uint8_t state);

void cmd_uart_receive_cb(uart_int_type type, uint8_t data);


void uart_power_on(void);
void uart_power_down(void);
void uart_wake_up(void);

void cmd_uart_test(void);


#endif
