//
// Created by hank on 2025/1/27.
//

#include "mcs_gesture.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

/* 函数声明 */
void load_gesture_data(Point* input, int len, GestureData* data);
void translation_invariance(GestureData* data);
void temporal_invariance(GestureData* data);
void signal_fusion_normalization(GestureData* data);
double cosine_similarity(const double* vec1, const double* vec2, int dim);
int gesture_match(GestureData* test, GestureData* train, int train_size, double threshold);



Point arr_circle_p[] = {
        {1125,1321},
        {1104,1311},
        {1081,1297},
        {1056,1281},
        {1031,1262},
        {1003,1239},
        {974,1212},
        {944,1180},
        {912,1146},
        {879,1106},
        {848,1064},
        {821,1025},
        {797,985},
        {776,943},
        {757,901},
        {739,856},
        {724,806},
        {710,756},
        {701,704},
        {696,654},
        {695,610},
        {699,568},
        {706,528},
        {718,491},
        {732,460},
        {749,430},
        {769,401},
        {794,374},
        {824,350},
        {854,329},
        {886,311},
        {921,297},
        {955,283},
        {989,269},
        {1024,252},
        {1060,235},
        {1096,217},
        {1131,201},
        {1167,188},
        {1204,180},
        {1239,177},
        {1275,178},
        {1314,184},
        {1351,197},
        {1389,215},
        {1428,236},
        {1467,264},
        {1505,301},
        {1542,342},
        {1577,391},
        {1610,442},
        {1639,496},
        {1664,554},
        {1684,610},
        {1699,669},
        {1707,724},
        {1708,779},
        {1705,832},
        {1698,879},
        {1687,922},
        {1671,963},
        {1651,1002},
        {1626,1039},
        {1597,1074},
        {1565,1109},
        {1531,1143},
        {1494,1175},
        {1458,1203},
        {1422,1226},
        {1385,1242},
        {1350,1253},
        {1317,1259},
        {1285,1263},
        {1255,1266},
        {1227,1270},
        {1201,1274},
        {1176,1281},
        {1153,1290},
        {1134,1300},
        {1119,1309},
        {1110,1316},
        {1105,1320},
        {1107,1324},
        {1107,1324}
};


Point arr_circle_p2[] = {
        {855,1254},
        {841,1257},
        {824,1259},
        {805,1257},
        {782,1252},
        {755,1242},
        {725,1228},
        {692,1209},
        {656,1187},
        {617,1161},
        {578,1133},
        {539,1100},
        {499,1062},
        {461,1022},
        {423,977},
        {387,927},
        {354,873},
        {327,815},
        {305,756},
        {291,696},
        {284,638},
        {283,581},
        {289,525},
        {302,471},
        {320,418},
        {342,368},
        {367,320},
        {396,273},
        {430,226},
        {465,183},
        {504,144},
        {546,112},
        {590,87},
        {639,71},
        {688,68},
        {739,77},
        {792,99},
        {845,134},
        {901,177},
        {954,225},
        {1009,279},
        {1062,334},
        {1113,395},
        {1161,453},
        {1205,513},
        {1241,573},
        {1270,635},
        {1292,699},
        {1306,765},
        {1311,832},
        {1308,899},
        {1296,964},
        {1276,1025},
        {1248,1083},
        {1211,1139},
        {1168,1190},
        {1117,1233},
        {1063,1266},
        {1006,1290},
        {950,1304},
        {896,1310},
        {844,1310},
        {798,1305},
        {759,1299},
        {730,1293},
        {710,1287},
        {697,1277},
        {691,1262},
        {691,1262}
};


Point arr_circle_p3[] = {
        {1408,1171},
        {1405,1172},
        {1399,1174},
        {1391,1177},
        {1379,1180},
        {1365,1183},
        {1348,1184},
        {1330,1184},
        {1311,1182},
        {1288,1178},
        {1262,1173},
        {1236,1166},
        {1213,1159},
        {1191,1150},
        {1169,1139},
        {1145,1127},
        {1120,1110},
        {1094,1086},
        {1069,1056},
        {1043,1022},
        {1015,982},
        {986,938},
        {960,893},
        {936,841},
        {914,791},
        {895,738},
        {880,684},
        {868,632},
        {861,578},
        {858,526},
        {861,475},
        {868,424},
        {878,377},
        {891,334},
        {908,293},
        {930,252},
        {957,214},
        {985,176},
        {1018,141},
        {1053,109},
        {1091,81},
        {1130,57},
        {1173,39},
        {1218,28},
        {1265,23},
        {1314,26},
        {1363,34},
        {1413,49},
        {1462,70},
        {1513,97},
        {1563,130},
        {1608,165},
        {1651,202},
        {1689,238},
        {1723,278},
        {1752,319},
        {1774,360},
        {1791,404},
        {1803,450},
        {1812,494},
        {1818,543},
        {1820,591},
        {1819,637},
        {1815,686},
        {1808,733},
        {1796,779},
        {1782,826},
        {1764,872},
        {1743,914},
        {1721,956},
        {1696,998},
        {1667,1035},
        {1636,1069},
        {1604,1102},
        {1568,1132},
        {1528,1160},
        {1488,1184},
        {1445,1205},
        {1402,1222},
        {1363,1237},
        {1327,1248},
        {1295,1256},
        {1268,1262},
        {1248,1267},
        {1233,1270},
        {1225,1271},
        {1220,1272},
        {1217,1269},
        {1217,1269},
};

Point arr_circle_p4[] = {
        {825,938},
        {813,903},
        {802,859},
        {793,809},
        {788,758},
        {785,703},
        {786,651},
        {792,599},
        {804,547},
        {822,499},
        {844,455},
        {871,413},
        {903,374},
        {939,342},
        {978,317},
        {1021,296},
        {1066,279},
        {1113,266},
        {1163,256},
        {1213,252},
        {1265,252},
        {1316,257},
        {1366,267},
        {1414,287},
        {1461,318},
        {1504,354},
        {1545,401},
        {1580,452},
        {1611,509},
        {1634,568},
        {1652,628},
        {1663,689},
        {1670,749},
        {1670,809},
        {1665,865},
        {1653,921},
        {1637,976},
        {1617,1027},
        {1593,1077},
        {1566,1123},
        {1533,1164},
        {1496,1199},
        {1455,1226},
        {1411,1244},
        {1368,1254},
        {1327,1259},
        {1288,1260},
        {1253,1257},
        {1222,1252},
        {1197,1244},
        {1176,1235},
        {1158,1225},
        {1141,1214},
        {1141,1214},
};


Point arr_c_p[] = {
        {1538,1180},
        {1526,1187},
        {1508,1193},
        {1486,1195},
        {1459,1194},
        {1426,1188},
        {1385,1177},
        {1337,1160},
        {1282,1139},
        {1222,1111},
        {1159,1077},
        {1095,1038},
        {1033,995},
        {976,947},
        {924,898},
        {878,844},
        {841,789},
        {811,730},
        {788,669},
        {773,606},
        {768,543},
        {771,483},
        {782,427},
        {801,377},
        {829,337},
        {863,305},
        {904,283},
        {950,267},
        {1002,258},
        {1056,256},
        {1113,257},
        {1171,261},
        {1229,266},
        {1287,272},
        {1339,279},
        {1388,287},
        {1432,297},
        {1467,308},
        {1493,319},
        {1510,326},
        {1520,331},
        {1525,334},
        {1528,336},
        {1529,337},
        {1529,338},
        {1529,338}
};

/* 主函数示例 */
int mcs_test() {
    // 示例数据初始化
    int data_len = sizeof(arr_circle_p)/sizeof(Point);

    GestureData test_gesture;
    load_gesture_data(arr_circle_p, data_len, &test_gesture);
    // 处理流程
    translation_invariance(&test_gesture);
    temporal_invariance(&test_gesture);
    signal_fusion_normalization(&test_gesture);

    data_len = sizeof(arr_circle_p2)/sizeof(Point);
    GestureData train_set[4];
    load_gesture_data(arr_circle_p2, data_len, &train_set[0]);
    translation_invariance(&train_set[0]);
    temporal_invariance(&train_set[0]);
    signal_fusion_normalization(&train_set[0]);

    data_len = sizeof(arr_circle_p3)/sizeof(Point);
    load_gesture_data(arr_circle_p3, data_len, &train_set[1]);
    translation_invariance(&train_set[1]);
    temporal_invariance(&train_set[1]);
    signal_fusion_normalization(&train_set[1]);

    data_len = sizeof(arr_circle_p4)/sizeof(Point);
    load_gesture_data(arr_circle_p4, data_len, &train_set[2]);
    translation_invariance(&train_set[2]);
    temporal_invariance(&train_set[2]);
    signal_fusion_normalization(&train_set[2]);

    data_len = sizeof(arr_c_p)/sizeof(Point);
    load_gesture_data(arr_c_p, data_len, &train_set[3]);
    translation_invariance(&train_set[3]);
    temporal_invariance(&train_set[3]);
    signal_fusion_normalization(&train_set[3]);

//    // 训练数据匹配示例
//    GestureData train_set[5]; // 假设有5个训练样本
    double threshold = 0.7;
    int result = gesture_match(&test_gesture, train_set, 4, threshold);

    return result;
}

/* 加载原始数据 */
void load_gesture_data(Point* input, int len, GestureData* data) {
    data->points = (Point*)malloc(len * sizeof(Point));
    data->length = len;
    for(int i=0; i<len; i++) {
        data->points[i] = input[i];
    }
}

/* 平移不变性处理 */
void translation_invariance(GestureData* data) {
    double sum_x = 0, sum_y = 0;

    // 计算均值
    for(int i=0; i<data->length; i++) {
        sum_x += data->points[i].x;
        sum_y += data->points[i].y;
    }
    double mean_x = sum_x / data->length;
    double mean_y = sum_y / data->length;

    // 减去均值
    for(int i=0; i<data->length; i++) {
        data->inv_points[i].x = data->points[i].x - mean_x;
        data->inv_points[i].y = data->points[i].y - mean_y;
    }
}

/* 时间长度不变性处理（线性插值） */
void temporal_invariance(GestureData* data) {
    double step = (double)(data->length - 1) / (FIXED_LENGTH - 1);

    for(int i=0; i<FIXED_LENGTH; i++) {
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

    // 交替存储x,y坐标
    for(int i=0; i<FIXED_LENGTH; i++) {
        data->feature[2*i] = data->resampled[i].x;
        data->feature[2*i+1] = data->resampled[i].y;
    }

    // 计算L2范数
    for(int i=0; i<2*FIXED_LENGTH; i++) {
        sum_sq += data->feature[i] * data->feature[i];
    }
    double norm = sqrt(sum_sq);

    // 归一化
    if(norm > 1e-6) {
        for(int i=0; i<2*FIXED_LENGTH; i++) {
            data->feature[i] /= norm;
        }
    }
}

/* 余弦相似度计算 */
double cosine_similarity(const double* vec1, const double* vec2, int dim) {
    double dot = 0.0, norm1 = 0.0, norm2 = 0.0;

    for(int i=0; i<dim; i++) {
        dot += vec1[i] * vec2[i];
        norm1 += vec1[i] * vec1[i];
        norm2 += vec2[i] * vec2[i];
    }

    return dot / (sqrt(norm1) * sqrt(norm2));
}

/* 手势匹配与阈值拒识 */
int gesture_match(GestureData* test, GestureData* train, int train_size, double threshold) {
    double max_sim = -1.0;
    int best_match = -1;

    for(int i=0; i<train_size; i++) {
        double sim = cosine_similarity(test->feature, train[i].feature, 2*FIXED_LENGTH);
        printf("idx:%d,sim:%f\n",i,sim);
        if(sim > max_sim) {
            max_sim = sim;
            best_match = i;
        }
    }
    return (max_sim >= threshold) ? best_match : -1;
}