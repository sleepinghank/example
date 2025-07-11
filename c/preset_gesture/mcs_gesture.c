#include "mcs_gesture.h"
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
//
// Created by hank on 2025/1/27.
//


// 设置相似度阈值（可以根据实际需求调整）
static double threshold = 0.7;

// 特征向量
// 0：s 1：z 2：n 3：w
//double feature[4][2*FIXED_LENGTH] = {
//    {0.209695,-0.410035,0.115704,-0.413453,-0.077406,-0.317753,-0.120130,-0.059703,0.084943,0.109482,0.098614,0.324808,-0.062026,0.406837,-0.313239,0.276957},
//    {-0.290850,-0.294864,-0.134953,-0.307812,0.150944,-0.246955,0.173215,-0.237374,-0.007024,0.021591,-0.215750,0.302568,0.041920,0.407190,0.294670,0.390357},
//    {-0.237456,0.462379,-0.149550,0.090468,-0.117430,-0.293277,-0.112359,-0.279753,0.036406,0.318686,0.117550,0.325448,0.229123,-0.291587,0.230814,-0.308492},
//    {-0.354507,-0.297767,-0.303648,-0.152327,-0.160529,0.334493,-0.026332,-0.036333,0.040053,-0.197297,0.090198,0.332708,0.353773,-0.069882,0.412484,-0.266537},
//};
double feature[4][2*FIXED_LENGTH] = {
        {0.256014,-0.275782,-0.011832,-0.424360,-0.272602,-0.273086,-0.081910,-0.051398,0.211878,0.041927,0.236810,0.332346,-0.053946,0.406130,-0.302925,0.217122,},
        {-0.222236,-0.368792,0.007756,-0.368792,0.235138,-0.333119,0.061991,-0.101096,-0.124497,0.130926,-0.201064,0.340326,0.030958,0.345837,0.262981,0.345837},
        { 0.311732,-0.339699,0.311732,-0.036778,0.303829,0.266144,0.091126,0.118305,-0.110712,-0.184616,-0.296745,-0.233347,-0.296745,0.069574,-0.296745,0.365581,},
        {-0.421641,-0.319710,-0.322317,0.048672,-0.180474,0.238816,-0.078429,-0.131947,0.080421,-0.141472,0.177704,0.229292,0.321928,0.051053,0.426013,-0.319710},
};

/* 函数声明 */
uint8_t load_gesture_data(Point* input, uint16_t len, GestureData* data);
void translation_invariance(GestureData* data);
void temporal_invariance(GestureData* data);
void signal_fusion_normalization(GestureData* data);
double cosine_similarity(const double* vec1, const double* vec2, int dim);

// 添加手势大小阈值的宏定义（您可以根据实际需求调整这个值）
#define MIN_GESTURE_SIZE 100  // 像素或其他单位

// 返回值表示手势是否有效
uint8_t load_gesture_data(Point* input, uint16_t len, GestureData* data) {
    uint8_t i = 0;
    double min_x = input[0].x, max_x = input[0].x;
    double min_y = input[0].y, max_y = input[0].y;
    
    // 找出x和y方向的最大最小值
    for(i = 0; i < len; i++) {
        if(input[i].x < min_x) min_x = input[i].x;
        if(input[i].x > max_x) max_x = input[i].x;
        if(input[i].y < min_y) min_y = input[i].y;
        if(input[i].y > max_y) max_y = input[i].y;
    }
    printf("length: %d\n", len);
    printf("max_x: %f, min_x: %f\n", max_x, min_x);
    // 检查手势大小
    double width = max_x - min_x;
    double height = max_y - min_y;
    printf("width: %f, height: %f\n", width, height);
    // 如果手势太小，返回false
    if(width < MIN_GESTURE_SIZE && height < MIN_GESTURE_SIZE) {
        return 0;
    }
    
    data->points = (Point*)malloc(len * sizeof(Point));
    data->length = len;
    for(i = 0; i < len; i++) {
        data->points[i] = input[i];
    }
    
    return 1;
}

/* 平移不变性处理 */
void translation_invariance(GestureData* data) {
    double sum_x = 0, sum_y = 0;
    uint8_t i = 0;
    // 计算均值
    for(i=0; i<data->length; i++) {
        sum_x += data->points[i].x;
        sum_y += data->points[i].y;
    }
    double mean_x = sum_x / data->length;
    double mean_y = sum_y / data->length;

    // 减去均值
    for(i=0; i<data->length; i++) {
        data->inv_points[i].x = data->points[i].x - mean_x;
        data->inv_points[i].y = data->points[i].y - mean_y;
    }
}

/* 时间长度不变性处理（线性插值） */
void temporal_invariance(GestureData* data) {
    double step = (double)(data->length - 1) / (FIXED_LENGTH - 1);
    int i=0;
    for(i=0; i<FIXED_LENGTH; i++) {
        double pos = i * step;
        int idx = (int)pos;
        double frac = pos - idx; // 插值系数

        if(idx == data->length - 1) {
            data->resampled[i] = data->inv_points[idx];
        } else {
            data->resampled[i].x = data->inv_points[idx].x * (1-frac) + data->inv_points[idx+1].x * frac;
            data->resampled[i].y = data->inv_points[idx].y * (1-frac) + data->inv_points[idx+1].y * frac;
        }
    }
}

/* 信号融合与归一化 */
void signal_fusion_normalization(GestureData* data) {
    double sum_sq = 0.0;
    int i=0;
    double norm = 0.0;
    // 交替存储x,y坐标
    for(i=0; i<FIXED_LENGTH; i++) {
        data->feature[2*i] = data->resampled[i].x;
        data->feature[2*i+1] = data->resampled[i].y;
    }

    // 计算L2范数
    for(i=0; i<2*FIXED_LENGTH; i++) {
        sum_sq += data->feature[i] * data->feature[i];
    }
    norm = sqrt(sum_sq);
    // 归一化
    if(norm > 1e-6) {
        for(i=0; i<2*FIXED_LENGTH; i++) {
            data->feature[i] /= norm;
        }
    }
//    针对data->feature，分别输出x和y坐标
//    printf("x坐标：\n");
//    for(i=0; i<FIXED_LENGTH; i++) {
//        printf("%f,",data->feature[2*i]);
//    }
//    printf("\ny坐标：\n");
//    for(i=0; i<FIXED_LENGTH; i++) {
//        printf("%f,",data->feature[2*i+1]);
//    }
//    printf("\n");
}

/* 余弦相似度计算 */
double cosine_similarity(const double* vec1, const double* vec2, int dim) {
    double dot = 0.0, norm1 = 0.0, norm2 = 0.0;
    int i=0;
    for(i=0; i<dim; i++) {
        dot += vec1[i] * vec2[i];
        norm1 += vec1[i] * vec1[i];
        norm2 += vec2[i] * vec2[i];
    }
    return dot / (sqrt(norm1) * sqrt(norm2));
}

// DTW快速计算（需预存模板）
float dtw_distance(const double* vec1, const double* vec2, int dim) {
    // 实现DTW算法（此处简化为示例）
    float cost = 0.0f;
    for (int i = 0; i < dim; i++) {
        cost += fabs(vec1[i] - vec2[i]);
    }
    return cost;
}


/**
 * 识别手势类型
 * @param input_points 输入的手势坐标点数组
 * @param point_count 坐标点数量
 * @return 识别出的手势类型
 */
GestureType recognize_gesture(Point* input_points, uint16_t point_count) {
    // 处理输入的手势数据
    GestureData input_data;
    // 找出最匹配的手势
    double max_similarity = -1.0;
    double similarity = 0.0;
    GestureType best_match = GESTURE_UNKNOWN;
    int i = 0;
//    printf("1\r\n");
      // 检查手势大小是否有效
    if(load_gesture_data(input_points, point_count, &input_data) == 0) {
        return GESTURE_UNKNOWN;
    }
//    printf("2\r\n");
    // 标准化处理
    translation_invariance(&input_data);
    temporal_invariance(&input_data);
    signal_fusion_normalization(&input_data);

//    输出input_data.feature
    printf("input_data.feature:\n");
    for(i=0; i<2*FIXED_LENGTH; i++) {
        printf("%f,",input_data.feature[i]);
    }
    printf("\n");

    // 与每个模板计算相似度
    for(i = 0; i < 4; i++) {
        similarity = cosine_similarity(input_data.feature, feature[i], 2*FIXED_LENGTH);

        float distance = dtw_distance(input_data.feature, feature[i], 2*FIXED_LENGTH);
        printf("i:%d,similarity: %f,distance:%f\n",i, similarity,distance);
        if(similarity > max_similarity && distance < 2) {
            max_similarity = similarity;
            best_match = (GestureType)i;
        }
    }
    
    // 如果最大相似度低于阈值，返回未知手势
    if(max_similarity < threshold) {
        return GESTURE_UNKNOWN;
    }
    
    return best_match+1;
}


// // 使用示例
// void test_gesture_recognition() {
//     // 示例：使用一组测试数据

//     int point_count = sizeof(arr_w_5) / sizeof(Point);
    
//     // 设置相似度阈值（可以根据实际需求调整）
//     double threshold = 0.7;
    
//     // 识别手势
//     GestureType result = recognize_gesture(arr_w_5, point_count, threshold);
    
//     // 输出结果
//     printf("Recognized gesture: %s\n", get_gesture_name(result));
// }

