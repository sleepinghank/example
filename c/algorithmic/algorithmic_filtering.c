#ifndef _ALGORITHMIC_FILTERING_H_
#define _ALGORITHMIC_FILTERING_H_


#include "math.h"
#include "algorithmic_filtering.h"
#include "string.h"
#include <stdint.h>
#include <stdio.h>

// 计算特征的数学值
double* compute(double features[N_FEATURES]){
    static double likelihoods[BASE_LENGTH];
    int i = 0;
    int j = 0;
    double nij;
    for (i = 0; i < BASE_LENGTH; ++i) {
        double sum = 0.;
        for (j = 0; j < N_FEATURES; ++j) {
            sum +=log(2. * M_PI * sigmas[i][j]);
        }
        nij = -0.5 * sum;
        sum = 0.;
        for (j = 0; j < N_FEATURES; ++j) {
            sum += pow(features[j] - thetas[i][j], 2.) / sigmas[i][j];
        }
        nij -= 0.5*sum;
        likelihoods[i] = log(priors[i]) + nij;
    }
    return likelihoods;
}

// 寻找明确的特征分类
static int findMax(double nums[]) {
    int idx = 0;
    int i = 0;
    for (i = 0; i < BASE_LENGTH; ++i) {
        idx = nums[i] > nums[idx] ? i : idx;
    }
    return idx;
}

static double logSumExp(double nums[]){
    double max = nums[findMax(nums)];
    double sum = 0.;
    int i = 0 , il = BASE_LENGTH;
    for (i = 0 ; i < il; i++) {
        sum += exp(nums[i] - max);
    }
    return max - log(sum);
}

// 直接预测分类
int predict(double features[]){
    return findMax((double *) compute(features));
}

// 预测每个分类的比值
double* predictProba(double features[]) {
    double* jll = compute(features);
    double sum = logSumExp(jll);
    int i = 0;
    for (i = 0; i < BASE_LENGTH; i++) {
        jll[i] = exp(jll[i] - sum);
        printf("jll[%d] = %f\n", i, jll[i]);
    }
    return jll;
}

int predict_secondary_data(double likelihoods[]){
    return findMax(likelihoods);
}

double* predictProba_secondary_data(double jll[]) {
    double sum = logSumExp(jll);
    int i = 0;
    for (i = 0; i < BASE_LENGTH; i++) {
        jll[i] = exp(jll[i] - sum);
    }
    return jll;
}

#endif
