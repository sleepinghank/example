//
// Created by hank on 2025/1/13.
//

#include "preset_gesture.h"
#include <stdio.h>

int16_t arr_circle[][2] = {
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


int16_t arr_circle2[][2] = {
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

int16_t arr_c[][2] = {
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

uint32_t calculate_matrix_distance(int16_t *array1, int16_t *array2, uint16_t x_len, uint16_t y_len) {
    uint32_t total_distance = 0;
    
    // 遍历矩阵的每个位置
    for(uint16_t x = 0; x < x_len; x++) {
        for(uint16_t y = 0; y < y_len; y++) {
            // 计算相同位置元素的差的绝对值
            int16_t diff = abs(array1[x * y_len + y] - array2[x * y_len + y]);
            total_distance += diff;
        }
    }
    
    return total_distance;
}

uint8_t build_coordinate_matrix(int16_t *array, uint16_t len, int16_t *coordinate_matrix) {
    // 2. 找到坐标的最大最小值来确定缩放比例
    int16_t x_min = array[0], x_max = array[0];
    int16_t y_min = array[1], y_max = array[1];
    
    for(uint16_t i = 0; i < len; i++) {
        if(array[i*2] < x_min) x_min = array[i*2];
        if(array[i*2] > x_max) x_max = array[i*2];
        if(array[i*2+1] < y_min) y_min = array[i*2+1];
        if(array[i*2+1] > y_max) y_max = array[i*2+1];
    }
    
    // 3. 计算缩放因子
    float x_scale = (float)(COORDINATE_MATRIX_X_RESOLUTION - 1) / (x_max - x_min);
    float y_scale = (float)(COORDINATE_MATRIX_Y_RESOLUTION - 1) / (y_max - y_min);
    
    // 4. 将坐标点映射到矩阵中
    for(uint16_t i = 0; i < len; i++) {
        int16_t x = (array[i*2] - x_min) * x_scale;
        int16_t y = (array[i*2+1] - y_min) * y_scale;
        
        // 确保坐标在有效范围内
        if(x >= 0 && x < COORDINATE_MATRIX_X_RESOLUTION && 
           y >= 0 && y < COORDINATE_MATRIX_Y_RESOLUTION) {
            coordinate_matrix[x * COORDINATE_MATRIX_Y_RESOLUTION + y] = 1;
        }
    }
    return 0;
}

uint8_t subtract_mean_from_coordinates(int16_t *array, uint16_t len){
    // 计算x和y坐标的平均值
    int32_t x_sum = 0;
    int32_t y_sum = 0;
    
    // 遍历二维数组,累加x和y坐标值
    for(uint16_t i = 0; i < len; i++) {
        x_sum += array[i*2];     // x坐标在偶数位
        y_sum += array[i*2+1];   // y坐标在奇数位
    }
    
    // 计算平均值
    int16_t x_mean = x_sum / len;
    int16_t y_mean = y_sum / len;
    // printf("x_mean:%d,y_mean:%d\r\n",x_mean,y_mean);
    // 减去平均值
    for(uint16_t i = 0; i < len; i++) {
        array[i*2] -= x_mean;      // 减去x平均值
        array[i*2+1] -= y_mean;    // 减去y平均值
        
        // 打印处理后的坐标
        // printf("Point %d: (%d, %d)\n", i, array[i*2], array[i*2+1]);
    }
    return 1;
}


uint8_t expand_coordinate_point(int16_t *array, uint16_t x_len, uint16_t y_len){
    // 创建临时矩阵用于存储扩展后的值
    int16_t expanded_matrix[COORDINATE_MATRIX_X_RESOLUTION][COORDINATE_MATRIX_Y_RESOLUTION] = {0};
    
    // 遍历原始矩阵中的每个点
    for(uint16_t x = 0; x < x_len; x++) {
        for(uint16_t y = 0; y < y_len; y++) {
            // 如果当前点是接触点
            if(array[x * y_len + y] == 1) {
                // 设置中心点的影响值
                expanded_matrix[x][y] = IMPACT_FACTOR;
                
                // 向四周扩展,计算影响值
                for(int16_t dx = -2; dx <= 2; dx++) {
                    for(int16_t dy = -2; dy <= 2; dy++) {
                        // 跳过中心点
                        if(dx == 0 && dy == 0) continue;
                        
                        int16_t new_x = x + dx;
                        int16_t new_y = y + dy;
                        
                        // 检查是否在矩阵范围内
                        if(new_x >= 0 && new_x < x_len && 
                           new_y >= 0 && new_y < y_len) {
                            // 计算到中心点的距离
                            int16_t distance = abs(dx) + abs(dy);
                            // 计算影响值
                            int16_t impact = IMPACT_FACTOR - (distance * DECREASING_PARAM);
                            
                            // 如果新的影响值大于当前值，则更新
                            if(impact > expanded_matrix[new_x][new_y]) {
                                expanded_matrix[new_x][new_y] = impact;
                            }
                        }
                    }
                }
            }
        }
    }
    
    // 将扩展后的值复制回原始数组
    for(uint16_t x = 0; x < x_len; x++) {
        for(uint16_t y = 0; y < y_len; y++) {
            array[x * y_len + y] = expanded_matrix[x][y];
        }
    }
    
    // 输出扩展后的矩阵
//    printf("\nExpanded coordinate matrix:\n");
//    for(uint16_t x = 0; x < x_len; x++) {
//        for(uint16_t y = 0; y < y_len; y++) {
//            printf("%3d ", array[x * y_len + y]);
//        }
//        printf("\n");
//    }
    return 1;
}




void preset_gesture_init(void) {
   uint8_t result = subtract_mean_from_coordinates(arr_circle,83);
    int16_t coordinate_matrix[COORDINATE_MATRIX_X_RESOLUTION][COORDINATE_MATRIX_Y_RESOLUTION] = {0};
   uint8_t result2 = build_coordinate_matrix(arr_circle,83,coordinate_matrix);

    // 5. 打印坐标矩阵
//    printf("\nCoordinate Matrix:\n");
//    for(int i = 0; i < COORDINATE_MATRIX_Y_RESOLUTION; i++) {
//        for(int j = 0; j < COORDINATE_MATRIX_X_RESOLUTION; j++) {
//            printf("%d ", coordinate_matrix[i][j]);
//        }
//        printf("\n");
//    }
    expand_coordinate_point(coordinate_matrix,COORDINATE_MATRIX_X_RESOLUTION,COORDINATE_MATRIX_Y_RESOLUTION);
    // ------------------------------------
   subtract_mean_from_coordinates(arr_circle2,68);
    int16_t coordinate_matrix2[COORDINATE_MATRIX_X_RESOLUTION][COORDINATE_MATRIX_Y_RESOLUTION] = {0};
   build_coordinate_matrix(arr_circle,68,coordinate_matrix2);

    // 5. 打印坐标矩阵
//    printf("\nCoordinate Matrix:\n");
//    for(int i = 0; i < COORDINATE_MATRIX_Y_RESOLUTION; i++) {
//        for(int j = 0; j < COORDINATE_MATRIX_X_RESOLUTION; j++) {
//            printf("%d ", coordinate_matrix2[i][j]);
//        }
//        printf("\n");
//    }
     expand_coordinate_point(coordinate_matrix2,COORDINATE_MATRIX_X_RESOLUTION,COORDINATE_MATRIX_Y_RESOLUTION);
    
// ------------------------------------
    subtract_mean_from_coordinates(arr_c,45);
    int16_t coordinate_matrix3[COORDINATE_MATRIX_X_RESOLUTION][COORDINATE_MATRIX_Y_RESOLUTION] = {0};
    build_coordinate_matrix(arr_c,45,coordinate_matrix3);

    // 5. 打印坐标矩阵
    printf("\nCoordinate Matrix:\n");
    for(int i = 0; i < COORDINATE_MATRIX_Y_RESOLUTION; i++) {
        for(int j = 0; j < COORDINATE_MATRIX_X_RESOLUTION; j++) {
            printf("%d ", coordinate_matrix3[i][j]);
        }
        printf("\n");
    }
    expand_coordinate_point(coordinate_matrix3,COORDINATE_MATRIX_X_RESOLUTION,COORDINATE_MATRIX_Y_RESOLUTION);

    uint32_t result4 = calculate_matrix_distance(coordinate_matrix,coordinate_matrix3,COORDINATE_MATRIX_X_RESOLUTION,COORDINATE_MATRIX_Y_RESOLUTION);
    printf("result4:%d\r\n",result4);
}