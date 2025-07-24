//
// Created by hank on 2025/7/16.
//

#ifndef C_FILTERING_H
#define C_FILTERING_H

#include <stdint.h>

// 滤波器配置常量
#define FILTER_MAX_WINDOW_SIZE 15

// 中值滤波器函数声明
/**
 * @brief 初始化中值滤波器
 * @param window_size 窗口大小 (3-15)
 * @param initial_value 初始值
 * @return uint8_t 1成功，0失败
 */
uint8_t median_filter_init(uint8_t window_size, int16_t initial_value);

/**
 * @brief 中值滤波持续迭代处理
 * @param new_value 新的输入值
 * @return int16_t 滤波后的值
 */
int16_t median_filter_process(int16_t new_value);

/**
 * @brief 重置中值滤波器
 */
void median_filter_reset(void);

// 低通滤波器函数声明
/**
 * @brief 初始化低通滤波器
 * @param channel 通道号 (0: 左通道, 1: 右通道)
 * @param alpha_numerator 滤波参数分子 (0-255)
 * @param alpha_denominator 滤波参数分母 (256)
 * @param initial_value 初始值
 * @return uint8_t 1成功，0失败
 */
uint8_t low_pass_filter_init(uint8_t channel, uint8_t alpha_numerator, uint8_t alpha_denominator, int16_t initial_value);

/**
 * @brief 低通滤波持续迭代处理
 * @param channel 通道号 (0: 左通道, 1: 右通道)
 * @param new_value 新的输入值
 * @return int16_t 滤波后的值
 */
int16_t low_pass_filter_process(uint8_t channel, int16_t new_value);

/**
 * @brief 重置低通滤波器
 * @param channel 通道号 (0: 左通道, 1: 右通道)
 */
void low_pass_filter_reset(uint8_t channel);


#endif //C_FILTERING_H
