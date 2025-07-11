//#include "ARMCM0.h"
//#include "debug.h"
//#include "ble.h"
//#include "timer.h"
//#include "pxi_par2860_ble_lib.h"
//#include "gpadc.h"
//#include "hid.h"
//#include "par_queue.h"
#include <string.h>
//#include "uart.h"
//#include "gpio.h"
//#include "ota.h"
//#include "keyboard.h"
//#include "keycode.h"
//#include "HID_OVER_GATT_KEYBOARD_OTA_Profile_PTP.h"
//#include "kb_para_profile.h"
#include "cmd_uart.h"
//#include "debug_log.h"
//#include "debug.h"
//#include "back_LED.h"
// #include "led.h"
// #include "pix_tp_update.h"
#include "stdint.h"
#include "par_queue.h"
#include "utils/util.h"

extern uint32_t _io_pin_wakeup;
static uint8_t cmd_uart_send_ack = 0;
//uint8_t enter_wft = 0;
queue_t cmd_uart_queue;
queue_t nus_queue;
bat_info_t bat_info = {
    .last_bat = 0,
    .current_bat = 100
};

uint16_t cmd_uart_rx_timeout = 1000;
uint8_t cmd_uart_rx_status;
uint8_t cmd_uart_rx_index;
uart_t uart_buffer;
uint8_t cmd_uart_event;
uint16_t cmd_uart_send_tick = 0;
uint16_t cmd_uart_tx_timeout = 0;
uint16_t cmd_uart_power_on_delay_send = 100;
uint8_t mcu_update_result = 0;
uint8_t mcu_update_result_cmd = 0;

// 优化缓冲区定义 - 大幅减小内存占用
#define UART_RX_BUFFER_SIZE 128  // 从2048减少到128字节
#define UART_RX_BUFFER_MASK (UART_RX_BUFFER_SIZE - 1)

typedef struct {
    uint8_t buffer[UART_RX_BUFFER_SIZE];
    volatile uint8_t write_index;    // 改为uint8_t节省内存
    volatile uint8_t read_index;     // 改为uint8_t节省内存
    volatile uint8_t overflow;
    volatile uint16_t total_received;   // 改为uint16_t节省内存
    volatile uint16_t total_processed;  // 改为uint16_t节省内存
} uart_rx_buffer_t;

static uart_rx_buffer_t uart_rx_buf = {0};

// 数据处理状态 - 增加包验证逻辑
typedef enum {
    UART_PARSE_IDLE = 0,
    UART_PARSE_SYNC1,
    UART_PARSE_SYNC2,
    UART_PARSE_LENGTH,     // 等待长度字节
    UART_PARSE_EVENT,      // 等待事件字节
    UART_PARSE_DATA,       // 等待数据字节
    UART_PARSE_CRC,        // 等待CRC字节
    UART_PARSE_VERIFY,     // 验证包完整性
    UART_PARSE_COMPLETE    // 解析完成
} uart_parse_state_t;

static uart_parse_state_t parse_state = UART_PARSE_IDLE;
static uint8_t parse_data_index = 0;
static uint16_t parse_timeout = 0;
static uint8_t candidate_packet[16]; // 候选包缓冲区用于验证

void _uart_cb(void){
  //----printf("uart_cb\r\n");
}


/// @brief 串口进入关机
void cmd_uart_enter_pd(void)
{
  //par_init_queue(&cmd_uart_queue);
//  _io_pin_wakeup &= ~U32BIT(CMD_UART_RX);
//  gpi_disable_int(CMD_UART_RX);
  // led_blink_stop();
  cmd_uart_en_queue(CMD_EVENT_POWER_DOWN, NULL, 0);

}

//uint8_t is_cmd_uart_queue_empty(void)
//{
//  return par_query_queue_empty(&cmd_uart_queue);
//}


/// @brief 检查串口状态 
/// @return 1：活动 0：空闲
//uint8_t check_uart_status(void)
//{
//  if (cmd_uart_rx_timeout || cmd_uart_event)
//  {
//    return 1;
//  }
//
//  return 0;
//}

/**
 * @brief cmd uart set event
 *
 * @param evt
 */
static void cmd_uart_set_evt(uint8_t evt)
{
  cmd_uart_event |= evt;
}

/**
 * @brief cmd uart clear event
 *
 * @param evt
 */
static void cmd_uart_clr_evt(uint8_t evt)
{
  cmd_uart_event &= ~evt;
}

/**
 * @brief 获取缓冲区中可读数据长度
 */
static uint8_t uart_buffer_available(void)  // 返回类型改为uint8_t
{
    return (uart_rx_buf.write_index - uart_rx_buf.read_index) & UART_RX_BUFFER_MASK;
}
/**
 * @brief uart tick timer
 *
 */
void uart_timer(void)
{
  if (cmd_uart_rx_timeout > 0)
  {
    if (--cmd_uart_rx_timeout == 0)
    {
      cmd_uart_rx_status = CMD_UART_RX_NONE;
    }
  }

  if (cmd_uart_tx_timeout > 0)
  {
    if (--cmd_uart_tx_timeout == 0)
    {
      cmd_uart_send_ack = 0;
    }
  }

  if (cmd_uart_power_on_delay_send > 0)
  {
    cmd_uart_power_on_delay_send --;
  }
  
  // 解析超时处理
  if (parse_timeout > 0) {
    if (--parse_timeout == 0) {
      if (parse_state != UART_PARSE_IDLE) {
        printf("UART: Parse timeout, reset parser. State was: %d\r\n", parse_state);
        parse_state = UART_PARSE_IDLE;
        parse_data_index = 0;
      }
    }
  }
  
  // 减少调试输出频率以节省资源
  static uint16_t debug_timer = 0;
  if (++debug_timer >= 5000) { // 减少调试频率，从1000增加到5000
    debug_timer = 0;
    
    if (uart_rx_buf.overflow || uart_buffer_available() > 0) {
      printf("UART: Avail=%d RX=%u Proc=%u OF=%d St=%d\r\n", 
               uart_buffer_available(),
               uart_rx_buf.total_received, 
               uart_rx_buf.total_processed,
               uart_rx_buf.overflow,
               parse_state);
      
      if (uart_rx_buf.overflow) {
        uart_rx_buf.overflow = 0; // 清除溢出标志
      }
    }
  }
}

uint8_t debug_rx_buffer[2048];
uint16_t debug_rx_index = 0;
/**
 * @brief 串口接收中断回调 - 最小化内存和处理时间
 */
void cmd_uart_receive_cb(uart_int_type type, uint8_t data)
{
    if (type == UART_RX_INT)
    {
        // MCU更新模式特殊处理
        if (cmd_uart_event & CMD_UART_LCD_MCU_UPDATE)
        {
            mcu_update_result = 1;
            mcu_update_result_cmd = data;
            return;
        }

        // 检查缓冲区是否满 - 使用uint8_t运算
        uint8_t next_write = (uart_rx_buf.write_index + 1) & UART_RX_BUFFER_MASK;
        if (next_write == uart_rx_buf.read_index) {
            // 缓冲区满，设置溢出标志
            uart_rx_buf.overflow = 1;
            return;
        }

        // 存储数据到环形缓冲区
        uart_rx_buf.buffer[uart_rx_buf.write_index] = data;
        uart_rx_buf.write_index = next_write;
        uart_rx_buf.total_received++;
        
        // 重置解析超时
        parse_timeout = 300;
    }
}

/**
 * @brief 从缓冲区读取一个字节
 */
static uint8_t uart_buffer_read_byte(uint8_t *data)
{
    if (uart_rx_buf.read_index == uart_rx_buf.write_index) {
        return 0; // 缓冲区为空
    }
    
    *data = uart_rx_buf.buffer[uart_rx_buf.read_index];
    uart_rx_buf.read_index = (uart_rx_buf.read_index + 1) & UART_RX_BUFFER_MASK;
    uart_rx_buf.total_processed++;
    
    return 1; // 成功读取
}



/**
 * @brief 清空接收缓冲区
 */
static void uart_buffer_clear(void)
{
    uart_rx_buf.read_index = uart_rx_buf.write_index;
    uart_rx_buf.overflow = 0;
}

/**
 * @brief 验证数据包是否有效
 * @param pkt_start 包数据起始指针 (从包头开始)
 * @param total_len 包总长度
 * @param is_dual_header 是否为双字节包头
 * @return 1:有效 0:无效
 */
static uint8_t validate_packet(uint8_t* pkt_start, uint8_t total_len, uint8_t is_dual_header)
{
    uint8_t header_len = is_dual_header ? 3 : 2; // 包头+长度字段的总长度
    uint8_t length_offset = is_dual_header ? 2 : 1; // 长度字段在包中的偏移
    printf("is_dual_header:%d\r\n", is_dual_header);
    print_hex_array("validate_packet\r\n", pkt_start, total_len);
    if (total_len < (header_len + 1)) return 0; // 最小包长度检查
    
    // 检查包头
    if (is_dual_header) {
        if (pkt_start[0] != 0x00 || pkt_start[1] != CMD_UART_RX_PKT) {
            return 0;
        }
    } else {
        if (pkt_start[0] != CMD_UART_RX_PKT) {
            return 0;
        }
    }
    
    uint8_t length = pkt_start[length_offset];
    
    // 长度有效性检查 (length包含事件+数据+CRC)
    if (length < 2 || length > 9) {
        return 0;
    }
    
    // 包总长度检查: 包头长度 + 长度字段(1) + 长度值
    if (total_len != (header_len + length)) {
        return 0;
    }
    
    uint8_t evt = pkt_start[length_offset + 1];
    
    // 事件类型有效性检查 (根据实际协议调整)
    if (evt == 0x00 || evt == 0xFF) {
        return 0;
    }
    
    // CRC验证 (从长度字节开始计算)
    uint8_t calculated_crc = get_crc(&pkt_start[length_offset - 1], length  + 1);
    uint8_t received_crc = pkt_start[total_len - 1];
    printf("calculated_crc:%d, received_crc:%d\r\n", calculated_crc, received_crc);
    return (calculated_crc == received_crc) ? 1 : 0;
}

/**
 * @brief 双字节包头解析状态机
 */
static void uart_parse_packet(void)
{
    uint8_t data;
    static uint8_t temp_buffer[18];
    static uint8_t temp_index = 0;
    
    while (uart_buffer_read_byte(&data)) {
        switch (parse_state) {
            case UART_PARSE_IDLE:
            case UART_PARSE_SYNC1:
                if (data == 0x00) {
                    parse_state = UART_PARSE_SYNC2;
                    temp_buffer[0] = data;
                    temp_index = 1;
                    parse_data_index = 0;
                    printf("UART: Sync1 detected: 0x%02x\r\n", data);
                } else {
                    // 继续等待0x00
                    parse_state = UART_PARSE_SYNC1;
                }
                break;
                
            case UART_PARSE_SYNC2:
                if (data == CMD_UART_RX_PKT) { // 0xAA
                    parse_state = UART_PARSE_LENGTH;
                    temp_buffer[temp_index++] = data;
                    printf("UART: Sync2 detected: 0x%02x, dual header complete\r\n", data);
                } else if (data == 0x00) {
                    // 可能是新包的开始，重新开始
                    temp_buffer[0] = data;
                    temp_index = 1;
                    printf("UART: New sync1 while waiting sync2\r\n");
                } else {
                    // 不是有效的第二个同步字节，回到SYNC1状态
                    parse_state = UART_PARSE_SYNC1;
                    temp_index = 0;
                    printf("UART: Invalid sync2, reset to sync1\r\n");
                    
                    // 检查当前字节是否为新的sync1
                    if (data == 0x00) {
                        parse_state = UART_PARSE_SYNC2;
                        temp_buffer[0] = data;
                        temp_index = 1;
                    }
                }
                break;
                
            case UART_PARSE_LENGTH:
                temp_buffer[temp_index++] = data;
                uart_buffer.length = data;
                printf("UART: Length: %d (includes event+data+CRC)\r\n", data);
                
                // 检查长度有效性
                if (uart_buffer.length < 2 || uart_buffer.length > 9) {
                    printf("UART: Invalid length, reset parser\r\n");
                    parse_state = UART_PARSE_SYNC1;
                    temp_index = 0;
                } else {
                    parse_state = UART_PARSE_EVENT;
                }
                break;
                
            case UART_PARSE_EVENT:
                temp_buffer[temp_index++] = data;
                uart_buffer.evt = data;
                parse_data_index = 0;
                printf("UART: Event: 0x%02x\r\n", data);
                
                if (uart_buffer.length == 2) {
                    // 长度为2: 事件(1) + CRC(1)，没有数据部分
                    parse_state = UART_PARSE_CRC;
                } else {
                    // 有数据部分
                    parse_state = UART_PARSE_DATA;
                }
                break;
                
            case UART_PARSE_DATA:
                if (parse_data_index < sizeof(uart_buffer.dat)) {
                    temp_buffer[temp_index++] = data;
                    uart_buffer.dat[parse_data_index] = data;
                    printf("UART: Data[%d]: 0x%02x\r\n", parse_data_index, data);
                    parse_data_index++;
                    
                    // 检查是否接收完所有数据: 长度 - 事件(1) - CRC(1)
                    if (parse_data_index >= (uart_buffer.length - 2)) {
                        parse_state = UART_PARSE_CRC;
                    }
                } else {
                    printf("UART: Data buffer overflow\r\n");
                    parse_state = UART_PARSE_SYNC1;
                    temp_index = 0;
                }
                break;
                
            case UART_PARSE_CRC:
                temp_buffer[temp_index++] = data;
                uart_buffer.crc = data;
                parse_state = UART_PARSE_VERIFY;
                printf("UART: CRC received: 0x%02x\r\n", data);
                break;
                
            case UART_PARSE_VERIFY:
                // 验证整个包 (假设为双字节包头)
                if (validate_packet(temp_buffer, temp_index, 1)) {
                    // 包有效，数据已在前面步骤中填充
                    parse_state = UART_PARSE_COMPLETE;
                    printf("UART: Valid dual-header packet verified\r\n");
                    printf("UART: Header=0x%02x%02x, Len=%d, Evt=0x%02x, CRC=0x%02x\r\n", 
                             temp_buffer[0], temp_buffer[1], uart_buffer.length, 
                             uart_buffer.evt, uart_buffer.crc);
                    print_hex_array("UART RX: \r\n", temp_buffer, temp_index);
                    
                    // 设置事件标志
                    cmd_uart_set_evt(CMD_UART_EVENT_RX_DONE);
                    temp_index = 0;
                    return;
                } else {
                    printf("UART: Invalid packet, searching for new header\r\n");
                    
                    // 包无效，在临时缓冲区中寻找下一个可能的包头
                    uint8_t found_header = 0;
                    for (uint8_t i = 1; i < temp_index - 1; i++) {
                        if (temp_buffer[i] == 0x00 && i + 1 < temp_index && temp_buffer[i+1] == CMD_UART_RX_PKT) {
                            // 找到新的包头候选
                            uint8_t remaining = temp_index - i;
                            memmove(temp_buffer, &temp_buffer[i], remaining);
                            temp_index = remaining;
                            found_header = 1;
                            
                            if (remaining == 2) {
                                parse_state = UART_PARSE_LENGTH;
                            } else if (remaining > 2) {
                                // 继续处理已有数据
                                uart_buffer.length = temp_buffer[2];
                                if (uart_buffer.length >= 2 && uart_buffer.length <= 9) {
                                    parse_state = UART_PARSE_EVENT;
                                    temp_index = 3; // 重新设置索引
                                } else {
                                    found_header = 0;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    
                    if (!found_header) {
                        parse_state = UART_PARSE_SYNC1;
                        temp_index = 0;
                    }
                }
                break;
                
            default:
                parse_state = UART_PARSE_SYNC1;
                temp_index = 0;
                break;
        }
    }
}

void cmd_uart_hw_init(void)
{
//  gpi_config(CMD_UART_RX, GPI_PULL_UP, GPI_INVERTED);
//  gpi_config(CMD_UART_TX, GPI_PULL_UP, GPI_INVERTED);
//
//  gpi_enable_int(CMD_UART_RX, LEVEL_TRIGGER, POL_FALLING_LOW, _uart_cb);
//  _io_pin_wakeup |= U32BIT(CMD_UART_RX);
//
//  pad_mux_write(CMD_UART_RX, PIN_SEL_UART_RXD1); // uart
//  pad_mux_write(CMD_UART_TX, PIN_SEL_UART_TXD1); // uart
//
//  uart_1_init(UART_RTS_CTS_DISABLE, UART_BAUD_9600);
//
//  CS_IN();
//  uart_1_en_int(UART_RX_INT, cmd_uart_receive_cb);
//  CS_OUT();


}

void cmd_uart_init(void)
{
  cmd_uart_hw_init();

  cmd_uart_rx_status = CMD_UART_RX_NONE;
  cmd_uart_rx_index = 0;
  cmd_uart_tx_timeout = 0;
  cmd_uart_send_ack = 0;
  cmd_uart_event = CMD_UART_EVENT_NONE;

  // cmd_uart_en_queue(CMD_EVENT_WAKE_UP, NULL, NULL);
}

void cmd_uart_pd_setting(void)
{
//  gpi_enable_int(CMD_UART_RX, LEVEL_TRIGGER, POL_FALLING_LOW, _uart_cb);
//  _io_pin_wakeup |= U32BIT(CMD_UART_RX);

  //  gpi_config(CMD_UART_RX, GPI_PULL_UP, GPI_INVERTED);
  //  gpi_config(CMD_UART_TX, GPI_PULL_UP, GPI_INVERTED);
  //  gpo_config(CMD_UART_TX, GPO_HIGH);
}

/**
 * @brief get data crc
 *
 * @param buff data buffer
 * @param len data length
 * @return crc
 */
uint8_t get_crc(uint8_t* buff, uint16_t len)
{
  uint8_t crc = 0, i;
  print_hex_array("get_crc\r\n", buff, len);
  for (i = 0; i < len; i++)
  {
    crc ^= buff[i];
  }

  return crc;
}

/**
 * @brief cmd uart en queue
 *
 * @param evt event id
 * @param dat event data
 * @param len event data length
 */
// extern uint8_t LCD_CLOSE_FLAG;
void cmd_uart_en_queue(uint8_t evt, uint8_t* dat, uint8_t len)
{
  uint8_t buf[16];

  if (cmd_uart_event & CMD_UART_LCD_MCU_UPDATE)
  {
    return ;
  }

//  enter_wft = 0;
  buf[0] = evt;
  memcpy(&buf[1], dat, len);

  par_en_queue(&cmd_uart_queue, buf, (len + 1));
}

/**
 * @brief cmd uart send pack - 保持原格式发送
 */
void cmd_uart_send_pack(uint8_t evt, uint8_t* dat, uint8_t length)
{
    uint8_t buf[20];
    uint8_t index = 0;
    uint8_t crc;

    memset(buf, 0, 20);

    buf[index++] = CMD_UART_RX_PKT;    // 原格式包头 0xAA
    buf[index++] = length + 2;         // 长度
    buf[index++] = evt;                // 事件

    if (length > 0)
    {
        memcpy(&buf[index], dat, length);
        index += length;
    }

    crc = get_crc(buf, index);
    buf[index++] = crc;

    // printf("UART TX: evt=0x%02x, len=%d, crc=0x%02x\r\n", evt, length, crc);
    // print_hex_array("UART TX: \r\n", buf, index);

//    enter_wft = 0;
    uint8_t wak = 0;
//    uart_1_write(&wak, 1);
//    uart_1_write(buf, index);

    if (CMD_EVENT_ACK != evt)
    {
        cmd_uart_send_ack = 1;
        cmd_uart_tx_timeout = 200 / 5;
    }
    else
    {
//        enter_wft = 1;
    }
}

/**
 * @brief cmd uart handler event
 *
 */
// extern uint8_t backlight_off_flag;
uint8_t backlight_should_off = 0;
void cmd_uart_event_handler(void)
{
  //  uint32_t t = 0;
  switch (uart_buffer.evt)
  {
    case CMD_EVENT_ACK:
      cmd_uart_send_ack = 0;
      cmd_uart_tx_timeout = 0;
      par_de_queue(&cmd_uart_queue);
//      enter_wft = 1;

      if (uart_buffer.dat[0] == CMD_EVENT_BATLEVEL)
      {
        printf("CMD_EVENT_BATLEVEL:%d\r\n",uart_buffer.dat[1]);
//        if (ble_is_connected() && ble_check_notify_enable(BATTERY_LEVEL_IDX))
//        {
//          // ble_tx_data(BATTERY_LEVEL_IDX, 1, &uart_buffer.dat[1]);
//          bat_info.current_bat = uart_buffer.dat[1];
//          if (bat_info.current_bat != bat_info.last_bat)
//          {
//            bat_info.last_bat = bat_info.current_bat;
//            ble_tx_data(BATTERY_LEVEL_IDX, 1, &uart_buffer.dat[1]);
//          }
//        }
      }

      // if (uart_buffer.dat[1] <= 10 || uart_buffer.dat[1] == 0xff)
      // {
      //   printf("LOW POWER!!!!!!!!!!\r\n");
      //   // backlight_off();
      //   backlight_should_off = 1;
      // }
      // else
      // {
      //   backlight_should_off = 0;
      // }

      //      PAR_LOG_INFO("CMD_EVENT_ACK-------------------uart_buffer.dat[1]--%d\n\r\n", uart_buffer.dat[1]);
      break;

    case CMD_EVENT_BATLEVEL:
      cmd_uart_send_pack(CMD_EVENT_ACK, NULL, 0);
      printf("-----------CMD_EVENT_BATLEVEL\r\n");
//      if (ble_is_connected() && ble_check_notify_enable(BATTERY_LEVEL_IDX))
//      {
//        ble_tx_data(BATTERY_LEVEL_IDX, 1, &uart_buffer.dat[0]);
//      }
      break;

    default:
       printf("unknown:%d\r\n",uart_buffer.dat[2]);
      break;
  }
}

/**
 * @brief cmd uart queue loop send
 *
 */
static void cmd_uart_queue_loop(void)
{
  cmd_uart_send_tick++;

  // 检查是否在开机延迟期间
  if (cmd_uart_power_on_delay_send > 0) {
      return;
  }

  if (cmd_uart_send_tick < 20)
  {
    return;
  }

  cmd_uart_send_tick = 0;

  if (cmd_uart_send_ack)
  {
    return;
  }

  if (cmd_uart_event & CMD_UART_LCD_MCU_UPDATE)
  {
    return ;
  }

  if (!par_query_queue_empty(&cmd_uart_queue))
  {
    uint8_t buf[20];
    uint8_t len = par_query_queue_head(&cmd_uart_queue, buf);

    //  NRF_LOG_INFO("QUEUE: \n\r\n");
    //   NRF_LOG_HEXDUMP_INFO(buf, len);

    cmd_uart_send_pack(buf[0], &buf[1], len - 1);
  }
}

/**
 * @brief cmd uart event run
 *
 */
void cmd_uart_event_run(void)
{
    // 处理缓冲区中的数据
    uart_parse_packet();
    
    // 处理解析完成的数据包
    if (cmd_uart_event & CMD_UART_EVENT_RX_DONE)
    {
        cmd_uart_clr_evt(CMD_UART_EVENT_RX_DONE);
        printf("UART: Processing packet - Evt=0x%02x, DataLen=%d\r\n", 
                 uart_buffer.evt, uart_buffer.length - 2);
        cmd_uart_event_handler();
        
        // 重置解析器准备下一个数据包
        parse_state = UART_PARSE_SYNC1;
        parse_data_index = 0;
    }

    cmd_uart_queue_loop();
}

/**
 * @brief ctrl lcd seg
 *
 * @param s set or clr (0x80 set ,0 clr)
 * @param seg seg index
 * @param com com value
 */
// extern uint8_t LCD_CLOSE_FLAG;
void cmd_uart_send_lcd_ctrl(uint8_t s, uint8_t seg, uint8_t com)
{
  // if (LCD_CLOSE_FLAG != 1)
  // {
    uint8_t buf[2];
    buf[0] = seg | s;
    buf[1] = com;

    cmd_uart_en_queue(CMD_EVENT_LCD_CTRL, buf, 2);
  // }
}



/**
 * @brief mcu update event
 *
 * @param l 0 clear update mode ,1 enter update mode
 */
void mcu_update_event_set(uint8_t l)
{
  if (l)
  {
    cmd_uart_event |= CMD_UART_LCD_MCU_UPDATE;
  }
  else
  {
    cmd_uart_event &= ~CMD_UART_LCD_MCU_UPDATE;
  }
}

// 增加强制发送函数用于紧急命令
void cmd_uart_send_immediate(uint8_t evt, uint8_t* dat, uint8_t len)
{
    printf("Immediate send cmd: 0x%02x\r\n", evt);
    
    // 如果不在开机延迟期间且没有等待ACK，立即发送
    if (cmd_uart_power_on_delay_send == 0 && !cmd_uart_send_ack) {
        cmd_uart_send_pack(evt, dat, len);
    } else {
        // 否则加入队列头部（高优先级）
        cmd_uart_en_queue(evt, dat, len);
        printf("Added to queue due to delay/ack\r\n");
    }
}

/**
 * @brief 发送电量查询命令
 */
void uart_query_battery_level(void)
{
    //发送电量查询命令到队列
//    if(ble_is_connected())
//    {
      // cmd_uart_send_pack(CMD_EVENT_BATLEVEL, NULL, 0);
      cmd_uart_en_queue(CMD_EVENT_BATLEVEL, NULL, 0);                                
      printf("SEND CMD_EVENT_BATLEVEL\r\n");
//    }
}


void uart_led_ctrl(uint8_t idx, uint8_t state)
{

    uint8_t seg = 0;
    uint8_t com = 0;
    uint8_t s = state ? 0x80 : 0;

    switch (idx)
    {
        case 0:
            seg = BT_PAIR_LED_SEG;
            com = BT_PAIR_LED_CMD;
            break;
        case 1:
            seg = CAPS_LOCK_LED_SEG;
            com = CAPS_LOCK_LED_CMD;
            break;
        case 2:
            seg = LOW_POWER_LED_SEG;
            com = LOW_POWER_LED_CMD;
            break;
        default:
          //----printf("cmd_uart_led_ctrl: idx error:0x%02x\r\n",idx);
          return;
    }
    cmd_uart_send_lcd_ctrl(s,seg,com);
}

void uart_power_on(void){
  cmd_uart_en_queue(CMD_EVENT_POWER_ON, NULL, 0);
  // cmd_uart_send_pack(CMD_EVENT_POWER_ON, NULL, 0);
  //----printf("SEND CMD_EVENT_POWER_ON\r\n");
}


void uart_power_down(void){
  cmd_uart_en_queue(CMD_EVENT_POWER_ON, NULL, 0);
  // cmd_uart_send_pack(CMD_EVENT_POWER_DOWN, NULL, 0);
  //----printf("SEND CMD_EVENT_POWER_ON\r\n");
}

void uart_wake_up(void){
  cmd_uart_en_queue(CMD_EVENT_POWER_ON, NULL, 0);
  // cmd_uart_send_pack(CMD_EVENT_WAKE_UP, NULL, 0);
  //----printf("SEND CMD_EVENT_WAKE_UP\r\n");
}

// 简化调试函数以节省代码空间
void cmd_uart_get_buffer_status(void)
{
    printf("UART Buf: W=%d R=%d Avail=%d RX=%u Proc=%u OF=%d St=%d\r\n", 
             uart_rx_buf.write_index, uart_rx_buf.read_index,
             uart_buffer_available(),
             uart_rx_buf.total_received, uart_rx_buf.total_processed,
             uart_rx_buf.overflow, parse_state);
}

void cmd_uart_reset_parser(void)
{
    parse_state = UART_PARSE_IDLE;
    parse_data_index = 0;
    parse_timeout = 0;
    uart_buffer_clear();
    printf("UART: Parser and buffer reset\r\n");
}

void cmd_uart_test(void)
{
  // 测试串口解析
  // 测试数据 包1：0x00 0xAA 0x02 0x02 0xAA 包2：0x00 0xAA 0x04 0x02 0x05 0x10 0xB9
  // 数据包组成为 包头(2) + 长度(1) + 事件(1) + 数据(n) + CRC(1)
//  uint8_t data1[] = {0x00, 0xAA, 0x02, 0x02, 0xAA,0x00, 0xAA, 0x04, 0x02, 0x05, 0x10, 0xB9};
  uint8_t data1[] = { 0x00,0xAA,0x03,0x05,0x0F,0xA3,0x00,0xAA,0x02,0x02,0xAA,0x00,0xAA,0x03,0x05,0x0F,0xA3,0x00,0xAA,0x02,0x02,0xAA,0x00,0xAA,0x02,0x02,0x00,0x02,0x02,0x00,0x02,0x02};

  // 调用cmd_uart_receive_cb 依次入栈
  for (uint8_t i = 0; i < sizeof(data1); i++)
  {
    cmd_uart_receive_cb(UART_RX_INT, data1[i]);
  }
  // 调用cmd_uart_event_run 解析
  while (1)
  {
    cmd_uart_event_run();
    //  延迟操作
//    for (uint8_t i = 0; i < 1000; i++)
//    {
//      __asm("nop\r\n");
//    }
  }
  
}