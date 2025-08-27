#ifndef __ALGORITHMIC_FILTERING_H__
#define __ALGORITHMIC_FILTERING_H__

// #if TCFG_TOUCHPAD_ENABLE
#include <stdint.h>

#define M_PI  3.14159265358979323846   // pi

#define BASE_LENGTH 2
#define N_FEATURES 3

// static const double key_priors[2] = {0.6246973365617433, 0.37530266343825663};
// static const double key_sigmas[2][2] = {{275639.56749429455, 157.77509262221355}, {173290.39599879808, 13.323058510375029}};
// static const double key_thetas[2][2] = {{1517.777519379845, 17.347286821705428}, {1021.4296774193548, 14.187096774193549}};

static const double priors[2] =  {0.47711511789181693, 0.5228848821081831};
static const double sigmas[2][N_FEATURES] = {{1.9192167413777041, 1620.8947863060075, 1214.736956059929}, {6.373906361833612, 140311.57924798806, 211342.8302101422}};
static const double thetas[2][N_FEATURES] = {{8.700581395348838, 43.20348837209303, 40.93313953488372}, {9.594164456233422, 233.0424403183024, 421.9867374005305}};

double* compute(double features[N_FEATURES]);


int predict(double features[]);

double* predictProba(double features[]);

// #endif
#endif
