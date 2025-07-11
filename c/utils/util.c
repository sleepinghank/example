//
// Created by hank on 2025/5/30.
//

#include <stdint.h>
#include <stdio.h>
#include "util.h"

/**
 * @brief 使用标准库打印十六进制数组
 * @param prefix 前缀字符串
 * @param data 数据指针
 * @param length 数据长度
 */
void print_hex_array(const char* prefix, uint8_t* data, uint8_t length)
{
    printf("%s", prefix);
    for (uint8_t i = 0; i < length; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}