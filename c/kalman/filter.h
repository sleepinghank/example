//
// Created by Hank on 2024/2/19.
//

#ifndef C_FILTER_H
#define C_FILTER_H

static const double R = 0.01;
void kalman_filter(double x, double p, double z, double* x2, double* p2);

#endif //C_FILTER_H
