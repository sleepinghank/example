//
// Created by Hank on 2024/1/10.
//
/*
This file is generated by https://github.com/nok/sklearn-porter/

Estimator:
    SVC
    https://scikit-learn.org/stable/modules/generated/sklearn.svm.SVC.html

Environment:
    scikit-learn v0.21.3
    sklearn-porter v1.0.0

Usage:
    1. Compile the generated source code:
        $ gcc SVC.c -std=c99 -lm -o SVC
    2. Execute a prediction:
        $ ./SVC feature_1 ... feature_4
*/

#include "svc.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define N_FEATURES 4
#define N_CLASSES 2
#define N_VECTORS 542
#define N_WEIGHTS 2
#define N_COEFFS 1
#define N_INTERS 1
#define KERNEL_TYPE 'r'
#define KERNEL_GAMMA 0.25
#define KERNEL_COEF 0.0
#define KERNEL_DEGREE 3

double vectors[542][4] = {{-32.0, 154.0, 349.0, 19.0}, {-57.0, 162.0, 680.0, 17.0}, {-26.0, 154.0, 349.0, 20.0}, {-36.0, 154.0, 349.0, 20.0}, {-58.0, 1076.0, 691.0, 13.0}, {-45.0, 1317.0, 701.0, 13.0}, {-33.0, 162.0, 680.0, 15.0}, {-13.0, 726.0, 51.0, 10.0}, {-62.0, 765.0, 183.0, 15.0}, {-37.0, 162.0, 680.0, 16.0}, {-14.0, 920.0, 90.0, 13.0}, {4.0, 124.0, 58.0, 9.0}, {-28.0, 1395.0, 165.0, 14.0}, {-67.0, 1228.0, 537.0, 13.0}, {-24.0, 727.0, 51.0, 10.0}, {-18.0, 545.0, 144.0, 14.0}, {-51.0, 154.0, 349.0, 20.0}, {-49.0, 331.0, 673.0, 15.0}, {-41.0, 755.0, 187.0, 13.0}, {-45.0, 1079.0, 182.0, 13.0}, {-50.0, 438.0, 517.0, 12.0}, {-65.0, 823.0, 677.0, 11.0}, {-18.0, 469.0, 63.0, 10.0}, {-32.0, 369.0, 180.0, 12.0}, {-72.0, 1413.0, 353.0, 19.0}, {-49.0, 160.0, 519.0, 20.0}, {-45.0, 355.0, 670.0, 11.0}, {-41.0, 921.0, 90.0, 13.0}, {-15.0, 1114.0, 97.0, 12.0}, {-44.0, 921.0, 90.0, 13.0}, {-55.0, 1398.0, 611.0, 15.0}, {-35.0, 1175.0, 350.0, 14.0}, {-40.0, 496.0, 313.0, 16.0}, {-67.0, 554.0, 140.0, 14.0}, {-47.0, 1398.0, 611.0, 15.0}, {-50.0, 154.0, 349.0, 20.0}, {-44.0, 537.0, 141.0, 14.0}, {-7.0, 156.0, 58.0, 20.0}, {-31.0, 162.0, 680.0, 16.0}, {-63.0, 497.0, 312.0, 18.0}, {-30.0, 1110.0, 100.0, 14.0}, {-33.0, 154.0, 349.0, 21.0}, {-61.0, 1398.0, 611.0, 15.0}, {-49.0, 1175.0, 350.0, 15.0}, {-25.0, 1115.0, 98.0, 12.0}, {-45.0, 1175.0, 350.0, 14.0}, {-22.0, 156.0, 58.0, 20.0}, {-62.0, 162.0, 680.0, 17.0}, {-52.0, 162.0, 680.0, 14.0}, {-23.0, 1388.0, 167.0, 14.0}, {-33.0, 1110.0, 100.0, 12.0}, {-29.0, 921.0, 90.0, 13.0}, {1.0, 156.0, 58.0, 19.0}, {-17.0, 726.0, 51.0, 10.0}, {-22.0, 470.0, 60.0, 11.0}, {-58.0, 368.0, 172.0, 15.0}, {-27.0, 744.0, 182.0, 14.0}, {-77.0, 1298.0, 708.0, 12.0}, {-47.0, 154.0, 349.0, 20.0}, {-61.0, 162.0, 680.0, 17.0}, {-53.0, 154.0, 349.0, 20.0}, {-51.0, 154.0, 349.0, 20.0}, {-39.0, 1413.0, 353.0, 19.0}, {2.0, 152.0, 57.0, 11.0}, {-10.0, 469.0, 63.0, 11.0}, {-67.0, 1295.0, 708.0, 12.0}, {-35.0, 1119.0, 98.0, 14.0}, {1.0, 156.0, 60.0, 12.0}, {-78.0, 823.0, 677.0, 12.0}, {-57.0, 556.0, 678.0, 11.0}, {-22.0, 1110.0, 100.0, 13.0}, {-18.0, 1403.0, 82.0, 11.0}, {-72.0, 1175.0, 350.0, 16.0}, {0.0, 156.0, 60.0, 12.0}, {-61.0, 985.0, 550.0, 13.0}, {-56.0, 1413.0, 353.0, 18.0}, {-64.0, 1398.0, 611.0, 15.0}, {-62.0, 1175.0, 350.0, 16.0}, {-28.0, 749.0, 187.0, 13.0}, {1.0, 156.0, 60.0, 19.0}, {-35.0, 747.0, 182.0, 14.0}, {-47.0, 162.0, 680.0, 17.0}, {-56.0, 160.0, 519.0, 19.0}, {-68.0, 943.0, 307.0, 17.0}, {-39.0, 1110.0, 100.0, 13.0}, {-8.0, 726.0, 49.0, 10.0}, {-50.0, 543.0, 140.0, 14.0}, {-21.0, 1399.0, 83.0, 11.0}, {-37.0, 1393.0, 166.0, 14.0}, {-26.0, 921.0, 90.0, 13.0}, {-45.0, 1290.0, 709.0, 12.0}, {-35.0, 181.0, 162.0, 23.0}, {-21.0, 181.0, 162.0, 22.0}, {-45.0, 160.0, 519.0, 19.0}, {-55.0, 1175.0, 350.0, 16.0}, {-63.0, 556.0, 678.0, 12.0}, {-70.0, 1256.0, 542.0, 14.0}, {-48.0, 984.0, 550.0, 13.0}, {-42.0, 432.0, 517.0, 12.0}, {-58.0, 548.0, 140.0, 14.0}, {-60.0, 433.0, 522.0, 15.0}, {-66.0, 533.0, 681.0, 13.0}, {-76.0, 736.0, 545.0, 14.0}, {-49.0, 733.0, 310.0, 18.0}, {-73.0, 1067.0, 693.0, 12.0}, {-27.0, 1395.0, 83.0, 11.0}, {-44.0, 162.0, 680.0, 18.0}, {-22.0, 1070.0, 182.0, 15.0}, {-41.0, 722.0, 315.0, 14.0}, {-32.0, 921.0, 90.0, 13.0}, {-54.0, 722.0, 315.0, 15.0}, {-20.0, 920.0, 90.0, 13.0}, {-18.0, 1401.0, 82.0, 11.0}, {2.0, 156.0, 60.0, 19.0}, {-37.0, 1396.0, 163.0, 13.0}, {-23.0, 920.0, 90.0, 13.0}, {-33.0, 160.0, 519.0, 18.0}, {-43.0, 924.0, 90.0, 13.0}, {-50.0, 154.0, 349.0, 20.0}, {-48.0, 757.0, 186.0, 14.0}, {-29.0, 156.0, 170.0, 22.0}, {-65.0, 492.0, 315.0, 17.0}, {-65.0, 372.0, 172.0, 14.0}, {-44.0, 1370.0, 354.0, 19.0}, {-55.0, 355.0, 670.0, 11.0}, {-41.0, 1175.0, 350.0, 14.0}, {-53.0, 157.0, 519.0, 18.0}, {-88.0, 1255.0, 542.0, 18.0}, {-16.0, 149.0, 170.0, 21.0}, {-41.0, 1121.0, 98.0, 15.0}, {-73.0, 1076.0, 691.0, 11.0}, {-25.0, 162.0, 680.0, 15.0}, {4.0, 137.0, 53.0, 11.0}, {-27.0, 1110.0, 100.0, 12.0}, {-36.0, 924.0, 89.0, 12.0}, {-56.0, 1226.0, 537.0, 12.0}, {-73.0, 823.0, 677.0, 13.0}, {-61.0, 162.0, 680.0, 17.0}, {-24.0, 1072.0, 183.0, 13.0}, {-1.0, 156.0, 60.0, 19.0}, {-10.0, 923.0, 87.0, 12.0}, {-12.0, 181.0, 162.0, 23.0}, {-49.0, 1317.0, 701.0, 12.0}, {-64.0, 162.0, 680.0, 18.0}, {-45.0, 492.0, 315.0, 16.0}, {-45.0, 529.0, 681.0, 13.0}, {-58.0, 159.0, 519.0, 18.0}, {-37.0, 162.0, 680.0, 18.0}, {-42.0, 162.0, 680.0, 17.0}, {-77.0, 743.0, 544.0, 14.0}, {-66.0, 1091.0, 174.0, 16.0}, {-38.0, 355.0, 670.0, 11.0}, {-56.0, 162.0, 680.0, 18.0}, {-35.0, 555.0, 143.0, 14.0}, {-42.0, 1289.0, 710.0, 12.0}, {-24.0, 1110.0, 100.0, 13.0}, {-53.0, 440.0, 517.0, 12.0}, {-27.0, 1074.0, 183.0, 13.0}, {-42.0, 1397.0, 162.0, 14.0}, {-63.0, 442.0, 517.0, 14.0}, {-20.0, 1111.0, 100.0, 12.0}, {-74.0, 371.0, 184.0, 14.0}, {-71.0, 375.0, 172.0, 14.0}, {-71.0, 1398.0, 611.0, 15.0}, {-79.0, 1413.0, 353.0, 19.0}, {-44.0, 156.0, 519.0, 17.0}, {-87.0, 1175.0, 350.0, 16.0}, {-42.0, 1398.0, 611.0, 15.0}, {-64.0, 1413.0, 353.0, 19.0}, {-5.0, 727.0, 50.0, 10.0}, {-22.0, 154.0, 349.0, 20.0}, {-70.0, 1370.0, 354.0, 18.0}, {-75.0, 558.0, 139.0, 14.0}, {-60.0, 160.0, 519.0, 21.0}, {-69.0, 337.0, 672.0, 16.0}, {-60.0, 723.0, 315.0, 15.0}, {-19.0, 1395.0, 83.0, 11.0}, {-26.0, 1397.0, 83.0, 11.0}, {-28.0, 156.0, 58.0, 20.0}, {-56.0, 1370.0, 354.0, 18.0}, {-36.0, 328.0, 674.0, 14.0}, {-22.0, 742.0, 182.0, 14.0}, {-22.0, 368.0, 177.0, 13.0}, {-46.0, 925.0, 90.0, 13.0}, {-26.0, 726.0, 51.0, 11.0}, {-6.0, 726.0, 47.0, 10.0}, {-63.0, 743.0, 544.0, 13.0}, {-68.0, 987.0, 549.0, 13.0}, {-60.0, 1180.0, 349.0, 15.0}, {-65.0, 370.0, 185.0, 13.0}, {-39.0, 941.0, 307.0, 15.0}, {-45.0, 364.0, 172.0, 14.0}, {-58.0, 731.0, 546.0, 14.0}, {-27.0, 368.0, 178.0, 14.0}, {-41.0, 162.0, 680.0, 17.0}, {-53.0, 730.0, 546.0, 13.0}, {-33.0, 162.0, 680.0, 16.0}, {-68.0, 995.0, 549.0, 15.0}, {-36.0, 1398.0, 611.0, 14.0}, {-16.0, 469.0, 59.0, 11.0}, {-39.0, 1121.0, 98.0, 13.0}, {-24.0, 1395.0, 83.0, 11.0}, {-36.0, 1077.0, 183.0, 14.0}, {-77.0, 492.0, 315.0, 17.0}, {-63.0, 160.0, 519.0, 22.0}, {-33.0, 1393.0, 166.0, 14.0}, {-21.0, 1395.0, 83.0, 11.0}, {-53.0, 994.0, 549.0, 14.0}, {-50.0, 154.0, 349.0, 20.0}, {-55.0, 160.0, 519.0, 20.0}, {-9.0, 726.0, 51.0, 11.0}, {-69.0, 162.0, 680.0, 18.0}, {-68.0, 436.0, 522.0, 14.0}, {-50.0, 1292.0, 709.0, 12.0}, {-51.0, 497.0, 313.0, 18.0}, {-24.0, 924.0, 88.0, 12.0}, {-21.0, 1403.0, 82.0, 11.0}, {-70.0, 370.0, 185.0, 15.0}, {-29.0, 552.0, 143.0, 14.0}, {-7.0, 156.0, 60.0, 20.0}, {-55.0, 531.0, 681.0, 13.0}, {-44.0, 160.0, 519.0, 18.0}, {-22.0, 1395.0, 83.0, 11.0}, {-4.0, 156.0, 58.0, 19.0}, {-32.0, 1413.0, 353.0, 17.0}, {-28.0, 156.0, 58.0, 19.0}, {-52.0, 370.0, 185.0, 13.0}, {-56.0, 751.0, 181.0, 16.0}, {-30.0, 1117.0, 98.0, 12.0}, {-27.0, 181.0, 162.0, 23.0}, {-70.0, 556.0, 678.0, 13.0}, {-39.0, 160.0, 170.0, 22.0}, {-75.0, 440.0, 521.0, 15.0}, {1.0, 156.0, 60.0, 18.0}, {-75.0, 1230.0, 536.0, 13.0}, {-28.0, 357.0, 172.0, 15.0}, {0.0, 154.0, 59.0, 11.0}, {-4.0, 727.0, 46.0, 10.0}, {-25.0, 745.0, 188.0, 14.0}, {-70.0, 1317.0, 701.0, 13.0}, {-42.0, 559.0, 141.0, 14.0}, {-77.0, 989.0, 548.0, 13.0}, {-48.0, 430.0, 523.0, 14.0}, {-46.0, 162.0, 680.0, 17.0}, {-56.0, 942.0, 307.0, 15.0}, {-29.0, 1392.0, 166.0, 14.0}, {-79.0, 534.0, 681.0, 13.0}, {3.0, 140.0, 53.0, 11.0}, {-29.0, 924.0, 89.0, 13.0}, {-72.0, 788.0, 682.0, 12.0}, {-61.0, 1076.0, 691.0, 11.0}, {-64.0, 370.0, 185.0, 12.0}, {-53.0, 162.0, 680.0, 17.0}, {-69.0, 492.0, 315.0, 16.0}, {-68.0, 767.0, 181.0, 15.0}, {-47.0, 436.0, 517.0, 13.0}, {-57.0, 333.0, 673.0, 15.0}, {-18.0, 1395.0, 83.0, 11.0}, {-73.0, 492.0, 315.0, 16.0}, {-51.0, 1086.0, 175.0, 16.0}, {-51.0, 492.0, 315.0, 16.0}, {-20.0, 1395.0, 83.0, 11.0}, {-39.0, 162.0, 680.0, 17.0}, {-32.0, 752.0, 187.0, 13.0}, {-66.0, 1076.0, 691.0, 11.0}, {-31.0, 156.0, 519.0, 16.0}, {-46.0, 722.0, 315.0, 15.0}, {-45.0, 1177.0, 350.0, 15.0}, {-74.0, 498.0, 311.0, 18.0}, {-50.0, 154.0, 349.0, 20.0}, {-17.0, 1403.0, 81.0, 11.0}, {-35.0, 532.0, 141.0, 14.0}, {-69.0, 734.0, 545.0, 14.0}, {-49.0, 727.0, 51.0, 10.0}, {-86.0, 556.0, 678.0, 14.0}, {-164.0, 1398.0, 611.0, 15.0}, {-70.0, 1084.0, 180.0, 14.0}, {-78.0, 914.0, 91.0, 12.0}, {-85.0, 1076.0, 691.0, 11.0}, {-123.0, 1304.0, 706.0, 14.0}, {-91.0, 565.0, 139.0, 13.0}, {-40.0, 470.0, 62.0, 11.0}, {-63.0, 1398.0, 160.0, 14.0}, {-87.0, 500.0, 310.0, 18.0}, {-94.0, 446.0, 521.0, 16.0}, {-99.0, 792.0, 681.0, 13.0}, {-65.0, 154.0, 349.0, 21.0}, {-117.0, 1107.0, 172.0, 16.0}, {-48.0, 156.0, 58.0, 21.0}, {-126.0, 538.0, 680.0, 14.0}, {-39.0, 471.0, 61.0, 10.0}, {-58.0, 1398.0, 160.0, 14.0}, {-81.0, 1097.0, 173.0, 18.0}, {17.0, 944.0, 174.0, 10.0}, {-56.0, 925.0, 90.0, 13.0}, {-107.0, 953.0, 298.0, 15.0}, {-43.0, 169.0, 167.0, 22.0}, {-104.0, 947.0, 307.0, 16.0}, {-152.0, 345.0, 671.0, 14.0}, {-51.0, 1120.0, 98.0, 15.0}, {-128.0, 1184.0, 348.0, 16.0}, {-121.0, 726.0, 314.0, 15.0}, {-86.0, 162.0, 680.0, 18.0}, {-42.0, 156.0, 58.0, 20.0}, {-83.0, 443.0, 521.0, 15.0}, {-47.0, 156.0, 58.0, 21.0}, {-76.0, 162.0, 680.0, 17.0}, {-152.0, 1317.0, 701.0, 13.0}, {-77.0, 162.0, 680.0, 17.0}, {-145.0, 1076.0, 691.0, 12.0}, {-49.0, 156.0, 58.0, 21.0}, {-98.0, 1371.0, 353.0, 18.0}, {-141.0, 1306.0, 705.0, 13.0}, {-71.0, 922.0, 90.0, 12.0}, {-152.0, 1379.0, 352.0, 18.0}, {-96.0, 340.0, 672.0, 15.0}, {-43.0, 726.0, 51.0, 10.0}, {-61.0, 1398.0, 160.0, 14.0}, {-66.0, 160.0, 519.0, 22.0}, {-94.0, 493.0, 315.0, 16.0}, {-74.0, 919.0, 91.0, 12.0}, {-92.0, 160.0, 519.0, 24.0}, {16.0, 873.0, 172.0, 9.0}, {-29.0, 1395.0, 83.0, 11.0}, {-80.0, 379.0, 172.0, 14.0}, {-79.0, 379.0, 172.0, 16.0}, {-46.0, 156.0, 58.0, 20.0}, {-120.0, 950.0, 307.0, 16.0}, {-34.0, 726.0, 51.0, 10.0}, {-65.0, 1397.0, 161.0, 14.0}, {-88.0, 569.0, 138.0, 14.0}, {-110.0, 1241.0, 536.0, 13.0}, {-60.0, 154.0, 349.0, 21.0}, {-126.0, 344.0, 671.0, 16.0}, {-49.0, 1111.0, 99.0, 13.0}, {-76.0, 160.0, 519.0, 22.0}, {-165.0, 1398.0, 611.0, 14.0}, {-116.0, 795.0, 681.0, 12.0}, {-88.0, 569.0, 138.0, 14.0}, {-46.0, 1393.0, 165.0, 14.0}, {-125.0, 1109.0, 172.0, 14.0}, {-45.0, 181.0, 162.0, 22.0}, {-85.0, 1234.0, 536.0, 13.0}, {-99.0, 735.0, 310.0, 18.0}, {-103.0, 355.0, 670.0, 12.0}, {-126.0, 355.0, 670.0, 12.0}, {-47.0, 181.0, 162.0, 23.0}, {-62.0, 154.0, 349.0, 20.0}, {-103.0, 993.0, 546.0, 13.0}, {-110.0, 449.0, 520.0, 15.0}, {-46.0, 156.0, 58.0, 20.0}, {-69.0, 160.0, 519.0, 21.0}, {-51.0, 156.0, 58.0, 21.0}, {15.0, 896.0, 171.0, 9.0}, {-54.0, 154.0, 349.0, 22.0}, {-89.0, 569.0, 138.0, 14.0}, {-85.0, 790.0, 682.0, 11.0}, {-94.0, 740.0, 544.0, 13.0}, {-42.0, 726.0, 51.0, 10.0}, {-45.0, 181.0, 162.0, 23.0}, {-64.0, 154.0, 349.0, 20.0}, {-143.0, 749.0, 544.0, 13.0}, {-107.0, 556.0, 678.0, 13.0}, {-47.0, 172.0, 165.0, 22.0}, {-85.0, 996.0, 548.0, 15.0}, {-85.0, 162.0, 680.0, 18.0}, {-50.0, 925.0, 90.0, 12.0}, {-124.0, 1109.0, 172.0, 14.0}, {-45.0, 727.0, 51.0, 11.0}, {-42.0, 727.0, 51.0, 11.0}, {-67.0, 154.0, 349.0, 20.0}, {-35.0, 1395.0, 83.0, 11.0}, {-129.0, 1398.0, 611.0, 15.0}, {-102.0, 770.0, 178.0, 16.0}, {-47.0, 1121.0, 98.0, 13.0}, {-54.0, 1114.0, 99.0, 12.0}, {-34.0, 1395.0, 83.0, 11.0}, {-46.0, 1397.0, 161.0, 14.0}, {14.0, 856.0, 173.0, 9.0}, {-152.0, 1400.0, 350.0, 18.0}, {-80.0, 162.0, 680.0, 18.0}, {-97.0, 744.0, 544.0, 14.0}, {-153.0, 1398.0, 611.0, 14.0}, {-98.0, 1076.0, 691.0, 11.0}, {-130.0, 1176.0, 350.0, 16.0}, {-83.0, 735.0, 310.0, 17.0}, {-54.0, 1115.0, 99.0, 12.0}, {-153.0, 1068.0, 693.0, 12.0}, {-91.0, 160.0, 519.0, 24.0}, {-80.0, 378.0, 172.0, 16.0}, {-105.0, 1105.0, 173.0, 17.0}, {-91.0, 991.0, 547.0, 13.0}, {-40.0, 165.0, 169.0, 23.0}, {-60.0, 925.0, 90.0, 13.0}, {-78.0, 912.0, 91.0, 11.0}, {-141.0, 1398.0, 611.0, 14.0}, {-116.0, 495.0, 313.0, 17.0}, {-123.0, 496.0, 313.0, 18.0}, {-85.0, 160.0, 519.0, 22.0}, {-40.0, 181.0, 162.0, 23.0}, {-89.0, 162.0, 680.0, 17.0}, {-103.0, 493.0, 314.0, 16.0}, {-89.0, 1067.0, 693.0, 13.0}, {-67.0, 160.0, 519.0, 20.0}, {-74.0, 912.0, 91.0, 12.0}, {-47.0, 175.0, 164.0, 22.0}, {-146.0, 464.0, 515.0, 16.0}, {-84.0, 162.0, 680.0, 20.0}, {-79.0, 160.0, 519.0, 24.0}, {-51.0, 156.0, 58.0, 20.0}, {-82.0, 338.0, 672.0, 17.0}, {-96.0, 1098.0, 175.0, 16.0}, {-28.0, 470.0, 62.0, 11.0}, {-103.0, 764.0, 179.0, 16.0}, {-86.0, 377.0, 177.0, 14.0}, {-33.0, 1395.0, 83.0, 11.0}, {-85.0, 160.0, 519.0, 21.0}, {-84.0, 952.0, 298.0, 16.0}, {-87.0, 162.0, 680.0, 18.0}, {-116.0, 1175.0, 350.0, 16.0}, {-46.0, 181.0, 162.0, 23.0}, {-140.0, 355.0, 670.0, 13.0}, {-143.0, 540.0, 680.0, 15.0}, {-107.0, 1317.0, 701.0, 13.0}, {-88.0, 569.0, 138.0, 14.0}, {-84.0, 738.0, 545.0, 13.0}, {-101.0, 452.0, 517.0, 15.0}, {-93.0, 355.0, 670.0, 12.0}, {-41.0, 166.0, 168.0, 22.0}, {-59.0, 1394.0, 164.0, 14.0}, {-43.0, 156.0, 58.0, 20.0}, {-83.0, 162.0, 680.0, 17.0}, {-71.0, 160.0, 519.0, 20.0}, {-55.0, 1113.0, 99.0, 12.0}, {-88.0, 1300.0, 707.0, 13.0}, {-88.0, 569.0, 138.0, 14.0}, {-76.0, 916.0, 91.0, 12.0}, {-76.0, 160.0, 519.0, 21.0}, {-48.0, 727.0, 51.0, 10.0}, {-64.0, 154.0, 349.0, 20.0}, {-72.0, 162.0, 680.0, 18.0}, {-68.0, 924.0, 90.0, 12.0}, {-121.0, 1244.0, 536.0, 13.0}, {-90.0, 1398.0, 611.0, 15.0}, {-116.0, 743.0, 544.0, 13.0}, {-151.0, 823.0, 677.0, 12.0}, {-47.0, 178.0, 163.0, 24.0}, {-121.0, 1108.0, 172.0, 15.0}, {-91.0, 1101.0, 172.0, 14.0}, {-47.0, 1111.0, 100.0, 12.0}, {-71.0, 913.0, 91.0, 12.0}, {-25.0, 470.0, 62.0, 10.0}, {-132.0, 496.0, 312.0, 18.0}, {-95.0, 536.0, 681.0, 15.0}, {-120.0, 1317.0, 701.0, 13.0}, {-81.0, 1398.0, 611.0, 15.0}, {-48.0, 156.0, 58.0, 21.0}, {-51.0, 1120.0, 98.0, 12.0}, {-69.0, 160.0, 519.0, 21.0}, {-80.0, 759.0, 180.0, 16.0}, {-38.0, 726.0, 51.0, 11.0}, {-33.0, 470.0, 62.0, 11.0}, {14.0, 910.0, 171.0, 11.0}, {-88.0, 162.0, 680.0, 20.0}, {-116.0, 1407.0, 351.0, 18.0}, {-111.0, 341.0, 672.0, 16.0}, {-34.0, 470.0, 62.0, 11.0}, {-64.0, 925.0, 90.0, 13.0}, {-45.0, 1111.0, 100.0, 13.0}, {-75.0, 160.0, 519.0, 22.0}, {-60.0, 1398.0, 160.0, 14.0}, {-85.0, 162.0, 680.0, 17.0}, {-87.0, 162.0, 680.0, 17.0}, {-109.0, 455.0, 516.0, 14.0}, {-40.0, 156.0, 58.0, 20.0}, {-58.0, 154.0, 349.0, 20.0}, {-60.0, 154.0, 349.0, 20.0}, {-33.0, 471.0, 61.0, 12.0}, {-85.0, 162.0, 680.0, 17.0}, {-140.0, 1176.0, 349.0, 16.0}, {-68.0, 1396.0, 162.0, 15.0}, {-137.0, 1375.0, 353.0, 18.0}, {-130.0, 1398.0, 611.0, 15.0}, {-143.0, 1398.0, 611.0, 15.0}, {-83.0, 1091.0, 178.0, 16.0}, {-136.0, 460.0, 516.0, 16.0}, {-92.0, 160.0, 519.0, 26.0}, {-32.0, 156.0, 58.0, 20.0}, {-134.0, 1248.0, 535.0, 13.0}, {-92.0, 762.0, 179.0, 16.0}, {-102.0, 1105.0, 172.0, 16.0}, {-65.0, 154.0, 349.0, 20.0}, {-36.0, 471.0, 61.0, 10.0}, {-144.0, 1377.0, 352.0, 18.0}, {-155.0, 1255.0, 541.0, 21.0}, {-44.0, 170.0, 166.0, 22.0}, {-89.0, 162.0, 680.0, 14.0}, {-104.0, 1182.0, 348.0, 16.0}, {-122.0, 1108.0, 172.0, 16.0}, {-117.0, 995.0, 546.0, 13.0}, {-110.0, 725.0, 315.0, 15.0}, {-125.0, 1109.0, 172.0, 16.0}, {-76.0, 377.0, 172.0, 14.0}, {-64.0, 1397.0, 160.0, 14.0}, {-40.0, 163.0, 169.0, 22.0}, {-90.0, 567.0, 138.0, 14.0}, {-64.0, 1394.0, 164.0, 15.0}, {-141.0, 1402.0, 350.0, 18.0}, {-85.0, 376.0, 179.0, 13.0}, {-49.0, 156.0, 58.0, 20.0}, {-56.0, 1394.0, 165.0, 14.0}, {-49.0, 156.0, 58.0, 12.0}, {-40.0, 471.0, 61.0, 10.0}, {-65.0, 914.0, 91.0, 13.0}, {-60.0, 1398.0, 160.0, 14.0}, {-117.0, 556.0, 678.0, 13.0}, {-118.0, 769.0, 177.0, 16.0}, {-87.0, 769.0, 179.0, 15.0}, {-87.0, 162.0, 680.0, 18.0}, {-30.0, 471.0, 61.0, 10.0}, {-53.0, 1118.0, 98.0, 13.0}, {-33.0, 1395.0, 83.0, 11.0}, {-94.0, 449.0, 517.0, 14.0}, {-98.0, 1238.0, 536.0, 13.0}, {-84.0, 447.0, 517.0, 14.0}, {-151.0, 355.0, 670.0, 14.0}, {-82.0, 374.0, 181.0, 14.0}, {-119.0, 823.0, 677.0, 12.0}, {-88.0, 569.0, 138.0, 13.0}, {-55.0, 154.0, 349.0, 20.0}, {-129.0, 454.0, 520.0, 16.0}, {-84.0, 569.0, 138.0, 13.0}, {-171.0, 1398.0, 611.0, 16.0}, {-61.0, 1398.0, 160.0, 15.0}, {-58.0, 154.0, 349.0, 20.0}, {-51.0, 1112.0, 99.0, 14.0}, {-109.0, 494.0, 314.0, 17.0}, {-87.0, 162.0, 680.0, 19.0}, {-53.0, 917.0, 90.0, 13.0}, {-95.0, 556.0, 678.0, 13.0}, {-82.0, 379.0, 174.0, 15.0}, {-81.0, 499.0, 311.0, 18.0}};
double coeffs[1][542] = {{-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -0.7437120807891436, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -0.2721702755351039, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -0.19253645640593928, -0.6769223557196912, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -0.3781692742524143, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -0.45040366709876867, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -0.1657681709872092, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -0.029940496019803177, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -0.5273753759684356, -1.0, -1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.1775280409425408, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.2594701118339681, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}};
double inters[1] = {0.18881955810579687};
int weights[2] = {273, 269};

int predict_svc (float features[]) {
    int i, j, k, d, l;
    float kernels[N_VECTORS];
    float kernel;
    switch (KERNEL_TYPE) {
        case 'l':
            // <x,x'>
            for (i = 0; i < N_VECTORS; i++) {
                kernel = 0.;
                for (j = 0; j < N_FEATURES; j++) {
                    kernel += vectors[i][j] * features[j];
                }
                kernels[i] = kernel;
            }
            break;
        case 'p':
            // (y<x,x'>+r)^d
            for (i = 0; i < N_VECTORS; i++) {
                kernel = 0.;
                for (j = 0; j < N_FEATURES; j++) {
                    kernel += vectors[i][j] * features[j];
                }
                kernels[i] = pow((KERNEL_GAMMA * kernel) + KERNEL_COEF, KERNEL_DEGREE);
            }
            break;
        case 'r':
            // exp(-y|x-x'|^2)
            for (i = 0; i < N_VECTORS; i++) {
                kernel = 0.;
                for (j = 0; j < N_FEATURES; j++) {
                    kernel += pow(vectors[i][j] - features[j], 2);
                }
                kernels[i] = exp(-KERNEL_GAMMA * kernel);
            }
            break;
        case 's':
            // tanh(y<x,x'>+r)
            for (i = 0; i < N_VECTORS; i++) {
                kernel = 0.;
                for (j = 0; j < N_FEATURES; j++) {
                    kernel += vectors[i][j] * features[j];
                }
                kernels[i] = tanh((KERNEL_GAMMA * kernel) + KERNEL_COEF);
            }
            break;
    }

    int starts[N_WEIGHTS];
    int start;
    for (i = 0; i < N_WEIGHTS; i++) {
        if (i != 0) {
            start = 0;
            for (j = 0; j < i; j++) {
                start += weights[j];
            }
            starts[i] = start;
        } else {
            starts[0] = 0;
        }
    }

    int ends[N_WEIGHTS];
    for (i = 0; i < N_WEIGHTS; i++) {
        ends[i] = weights[i] + starts[i];
    }

    if (N_CLASSES == 2) {

        for (i = 0; i < N_VECTORS; i++) {
            kernels[i] = -kernels[i];
        }

        float decision = 0.;
        for (k = starts[1]; k < ends[1]; k++) {
            decision += kernels[k] * coeffs[0][k];
        }
        for (k = starts[0]; k < ends[0]; k++) {
            decision += kernels[k] * coeffs[0][k];
        }
        decision += inters[0];

        if (decision > 0) {
            return 0;
        }
        return 1;

    }

    float decisions[N_INTERS];
    float tmp;
    for (i = 0, d = 0, l = N_WEIGHTS; i < l; i++) {
        for (j = i + 1; j < l; j++) {
            tmp = 0.;
            for (k = starts[j]; k < ends[j]; k++) {
                tmp += kernels[k] * coeffs[i][k];
            }
            for (k = starts[i]; k < ends[i]; k++) {
                tmp += kernels[k] * coeffs[j - 1][k];
            }
            decisions[d] = tmp + inters[d];
            d = d + 1;
        }
    }

    int votes[N_INTERS];
    for (i = 0, d = 0, l = N_WEIGHTS; i < l; i++) {
        for (j = i + 1; j < l; j++) {
            votes[d] = decisions[d] > 0 ? i : j;
            d = d + 1;
        }
    }

    int amounts[N_CLASSES];
    for (i = 0, l = N_CLASSES; i < l; i++) {
        amounts[i] = 0;
    }
    for (i = 0; i < N_INTERS; i++) {
        amounts[votes[i]] += 1;
    }

    int classVal = -1;
    int classIdx = -1;
    for (i = 0; i < N_CLASSES; i++) {
        if (amounts[i] > classVal) {
            classVal = amounts[i];
            classIdx= i;
        }
    }
    return classIdx;
}

int run(int argc,float argv[]) {
    printf("Loading model...\n");
    /* Features: */
    float features[argc-1];
    for (int i = 0; i < argc; i++) {
        printf("Feature #%d: %f\n", i, argv[i]);
        features[i] = argv[i];
    }

    /* Get class prediction: */
    printf("Predicted class: #%d\n", predict_svc(features));

    return 0;
}