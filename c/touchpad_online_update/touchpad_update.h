//
// Created by hank on 2024/7/15.
//

#ifndef KB_06101_PIXART_TOUCHPAD_UPDATE_H
#define KB_06101_PIXART_TOUCHPAD_UPDATE_H
#include <stdint.h>

#define TOUCHPAD_KEY1_ADDR 0x2c   //写入0xaa 进入工程模式 
#define TOUCHPAD_KEY2_ADDR 0x2d   //写入0xcc 进入工程模式 写入 0xbb 离开工程模式

#define TOUCHPAD_FLASH_CONTR_ADDR 0x0d // 写入0x02 开启flash控制模式
#define TOUCH_SLAVE_ID 								0x33 // 触摸板从机地址
#define INT_GPIO            						22 // 中断GPIO

uint8_t PTP_read_reg(uint16_t addr, uint8_t * p_val, uint8_t sz);
uint8_t touch_write_reg(uint16_t addr, uint8_t val);
uint8_t touch_read_reg(uint16_t addr, uint8_t * p_val);
// Main Flow Chart函数封装
uint8_t main_flow_chart(const uint8_t *firmware_code, uint32_t firmware_size);

uint8_t start_update(void);
#endif //KB_06101_PIXART_TOUCHPAD_UPDATE_H
