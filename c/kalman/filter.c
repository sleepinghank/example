//
// Created by Hank on 2024/2/19.
//

#include "filter.h"
#include <stdio.h>

// 卡尔曼滤波 迭代计算
// 卡尔曼滤波 迭代计算
void kalman_filter(double x, double p, double z, double* x2, double* p2) {
    double x1 = x;
    double p1 = p;
    double k = p1 / (p1 + R);
    *x2 = x1 + k * (z - x1);
    *p2 = (1.0 - k) * p1;
}
