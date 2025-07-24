//
// Created by hank on 2025/5/30.
//

#include "filtering.h"
#include <string.h>

// 中值滤波结构体
typedef struct {
    int16_t buffer[FILTER_MAX_WINDOW_SIZE];
    uint8_t window_size;
    uint8_t current_index;
    uint8_t is_initialized;
} median_filter_t;

// 低通滤波结构体
typedef struct {
    int16_t last_filtered_value[2];  // 各通道上次滤波值 [0]:左通道 [1]:右通道
    uint8_t alpha_numerator[2];      // 各通道滤波参数分子 (0-255)
    uint8_t alpha_denominator[2];    // 各通道滤波参数分母 (256)
    uint8_t is_initialized[2];       // 各通道初始化状态
} low_pass_filter_t;

// 全局滤波器实例
static median_filter_t median_filter_instance;
static low_pass_filter_t low_pass_filter_instance;

/**
 * @brief 初始化中值滤波器
 * @param window_size 窗口大小 (3-15)
 * @param initial_value 初始值
 * @return uint8_t 1成功，0失败
 */
uint8_t median_filter_init(uint8_t window_size, int16_t initial_value) {
    if (window_size > FILTER_MAX_WINDOW_SIZE || window_size < 1) {
        return 0;  // 窗口大小无效
    }

    median_filter_instance.window_size = window_size;
    median_filter_instance.current_index = 0;
    median_filter_instance.is_initialized = 1;

    // 用初始值填充缓冲区
    for (uint8_t i = 0; i < window_size; i++) {
        median_filter_instance.buffer[i] = initial_value;
    }

    return 1;
}

/**
 * @brief 中值滤波持续迭代处理
 * @param new_value 新的输入值
 * @return int16_t 滤波后的值
 */
int16_t median_filter_process(int16_t new_value) {
    if (!median_filter_instance.is_initialized) {
        return new_value;  // 未初始化时直接返回输入值
    }

    // 更新缓冲区
    median_filter_instance.buffer[median_filter_instance.current_index] = new_value;
    median_filter_instance.current_index = (median_filter_instance.current_index + 1) % median_filter_instance.window_size;

    // 计算中值
    int16_t temp_buffer[FILTER_MAX_WINDOW_SIZE];
    memcpy(temp_buffer, median_filter_instance.buffer, sizeof(int16_t) * median_filter_instance.window_size);

    // 简单的冒泡排序（避免浮点运算）
    for (uint8_t i = 0; i < median_filter_instance.window_size - 1; i++) {
        for (uint8_t j = 0; j < median_filter_instance.window_size - 1 - i; j++) {
            if (temp_buffer[j] > temp_buffer[j + 1]) {
                int16_t temp = temp_buffer[j];
                temp_buffer[j] = temp_buffer[j + 1];
                temp_buffer[j + 1] = temp;
            }
        }
    }

    // 返回中值
    return temp_buffer[median_filter_instance.window_size / 2];
}

/**
 * @brief 初始化低通滤波器
 * @param channel 通道号 (0: 左通道, 1: 右通道)
 * @param alpha_numerator 滤波参数分子 (0-255)
 * @param alpha_denominator 滤波参数分母 (256)
 * @param initial_value 初始值
 * @return uint8_t 1成功，0失败
 */
uint8_t low_pass_filter_init(uint8_t channel, uint8_t alpha_numerator, uint8_t alpha_denominator, int16_t initial_value) {
    if (channel > 1 || alpha_denominator == 0 || alpha_numerator > alpha_denominator) {
        return 0;  // 参数无效
    }

    low_pass_filter_instance.alpha_numerator[channel] = alpha_numerator;
    low_pass_filter_instance.alpha_denominator[channel] = alpha_denominator;
    low_pass_filter_instance.last_filtered_value[channel] = initial_value;
    low_pass_filter_instance.is_initialized[channel] = 1;

    return 1;
}

/**
 * @brief 低通滤波持续迭代处理
 * @param channel 通道号 (0: 左通道, 1: 右通道)
 * @param new_value 新的输入值
 * @return int16_t 滤波后的值
 */
int16_t low_pass_filter_process(uint8_t channel, int16_t new_value) {
    if (channel > 1 || !low_pass_filter_instance.is_initialized[channel]) {
        return new_value;  // 通道无效或未初始化时直接返回输入值
    }

    // 使用整数运算避免浮点数
    // filtered[i] = alpha * raw[i] + (1-alpha) * filtered[i-1]
    // 等价于: filtered[i] = (alpha * raw[i] + (denominator-alpha) * filtered[i-1]) / denominator

    int32_t result = (int32_t)low_pass_filter_instance.alpha_numerator[channel] * new_value +
                     (int32_t)(low_pass_filter_instance.alpha_denominator[channel] - low_pass_filter_instance.alpha_numerator[channel]) *
                     low_pass_filter_instance.last_filtered_value[channel];

    // 四舍五入除法
    result = (result + low_pass_filter_instance.alpha_denominator[channel] / 2) / low_pass_filter_instance.alpha_denominator[channel];

    // 限制在int16_t范围内
    if (result > 32767) result = 32767;
    if (result < -32768) result = -32768;

    low_pass_filter_instance.last_filtered_value[channel] = (int16_t)result;
    return (int16_t)result;
}

/**
 * @brief 重置中值滤波器
 */
void median_filter_reset(void) {
    median_filter_instance.is_initialized = 0;
}

/**
 * @brief 重置低通滤波器
 * @param channel 通道号 (0: 左通道, 1: 右通道)
 */
void low_pass_filter_reset(uint8_t channel) {
    if (channel <= 1) {
        low_pass_filter_instance.is_initialized[channel] = 0;
    }
}