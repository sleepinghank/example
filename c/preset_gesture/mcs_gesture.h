//
// Created by hank on 2025/1/27.
//

#ifndef C_MCS_GESTURE_H
#define C_MCS_GESTURE_H

#include <stdint.h>

#define MAX_FRAMES 1000
#define FIXED_LENGTH 8

/* 结构体定义 */
typedef struct {
    double x;
    double y;
} Point;

typedef struct {
    Point* points;      // 原始坐标序列
    int length;         // 原始长度
    Point inv_points[MAX_FRAMES]; // 平移不变处理后的坐标
    Point resampled[FIXED_LENGTH]; // 固定长度坐标
    double feature[2*FIXED_LENGTH]; // 最终特征向量
} GestureData;

// 手势类型枚举
typedef enum {
    GESTURE_S = 1,
    GESTURE_Z = 2,
    GESTURE_N = 3,
    GESTURE_W = 4,
    GESTURE_UNKNOWN = 100
} GestureType;

/**
 * 识别手势类型
 * @param input_points 输入的手势坐标点数组
 * @param point_count 坐标点数量
 * @return 识别出的手势类型
 */
GestureType recognize_gesture(Point* input_points, uint8_t point_count);

#endif //C_MCS_GESTURE_H
