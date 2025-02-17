//
// Created by hank on 2025/1/27.
//

#ifndef C_MCS_GESTURE_H
#define C_MCS_GESTURE_H

int mcs_test();
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

#endif //C_MCS_GESTURE_H
