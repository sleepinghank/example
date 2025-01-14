//
// Created by hank on 2025/1/13.
//

#ifndef C_PRESET_GESTURE_H
#define C_PRESET_GESTURE_H

#include <stdint.h>

#define COORDINATE_MATRIX_X_RESOLUTION 24
#define COORDINATE_MATRIX_Y_RESOLUTION 24

#define IMPACT_FACTOR 3
#define DECREASING_PARAM 1

/**
 * @brief 初始化预设手势，将所有模版数据转换为对应分辨率的坐标
 */
void preset_gesture_init(void);
/**
 * @brief 根据平移不动性，将所有坐标减去 平均值，
 * @param array 原始数组
 * @param len 数据长度 
 * @return uint8 成功与否
 */
uint8_t subtract_mean_from_coordinates(int16_t *array, uint16_t len);

/**
 * @brief 按照固定分辨率，构建坐标矩阵，降低分辨率，
 * @param array 原始数组
 * @param x_len x轴分辨率
 * @param y_len y轴分辨率
 * @return uint8 成功与否
 */
uint8_t build_coordinate_matrix(int16_t *array, uint16_t len, int16_t *coordinate_matrix);
/**
 * @brief 对当前坐标点进行扩展渲染
 * @param array 坐标矩阵数组
 * @param x_len x轴分辨率
 * @param y_len y轴分辨率
 * @return uint8_t 成功与否
 */
uint8_t expand_coordinate_point(int16_t *array, uint16_t x_len, uint16_t y_len);

/**
 * @brief 将坐标矩阵分为四个区块，对每个区块进行k最近邻计算
 * @param array 坐标矩阵数组
 * @param x_len x轴分辨率
 * @param y_len y轴分辨率
 * @param k 最近邻的k值
 * @param distances 返回的四个区块的距离值数组
 * @return uint8_t 成功与否
 */
uint8_t calculate_quadrant_knn_distances(int16_t *array,uint8_t k, float *distances);

/**
 * @brief 计算单个区块的k最近邻距离
 * @param array 坐标矩阵数组
 * @param start_x 区块起始x坐标
 * @param start_y 区块起始y坐标
 * @param width 区块宽度
 * @param height 区块高度
 * @return uint8_t 最近邻模块的下标
 */
uint8_t calculate_block_knn_distance(int16_t *array, uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height);



#endif //C_PRESET_GESTURE_H
