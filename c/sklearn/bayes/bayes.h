#ifndef __ALGORITHMIC_FILTERING_H__
#define __ALGORITHMIC_FILTERING_H__

// #if TCFG_TOUCHPAD_ENABLE


#define M_PI  3.14159265358979323846   // pi

// #define N_FEATURES 2

// static const double size_soi_priors[2] = {0.4541198501872659, 0.545880149812734};
// static const double size_soi_sigmas[2][2] = {{272674.709710334, 19.96359795249337}, {372281.39321618195, 13.063431399365927}};
// static const double size_soi_thetas[2][2] = {{1686.4042268041258, 17.061855670103093}, {1094.9467409948543, 12.231560891938251}};


//#define N_FEATURES 2
//
//static const double size_soi_priors[2] = {0.5968882602545968, 0.4031117397454031};
//static const double size_soi_sigmas[2][N_FEATURES] = {{417.8506464181212, 4.33047054605938}, {8937.04299351051, 5.574701728415645}};
//static const double size_soi_thetas[2][N_FEATURES] = {{-24.46445497630332, 12.191943127962086}, {-231.16491228070174, 14.79298245614035}};

// one presure
#define N_FEATURES 2

static const double size_soi_priors[2] = {0.5033809166040571, 0.4966190833959429};
static const double size_soi_sigmas[2][2] = {{320147.1319334255, 116.66963001747328}, {87955.13463003254, 43.92591438395398}};
static const double size_soi_thetas[2][2] = {{741.7738805970149, 14.299253731343283}, {440.0839636913767, 12.695915279878971}};

double* compute(double features[N_FEATURES]);

int predict_bayes(double features[]);

double* predictProba(double features[]);

// #endif
#endif