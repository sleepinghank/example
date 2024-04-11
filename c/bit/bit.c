//
// Created by Hank on 2024/1/25.
//
#include <stdio.h>
#include "bit.h"
#include <stdint.h>
#include <string.h>

typedef struct
{
    uint32_t Firmware_CRC32;//固件校验码
    uint32_t OTA_TimesTamp;//OTA时间戳
} Firmware_Info_t;
Firmware_Info_t Firmware_Info;

uint8_t Pre_Tip[6]= {0,0,0,0,0,0};
uint8_t Pre_Confidence[6]= {1,0,0,0,0,0};


static const int debruijn[32] = {
        0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

// 取从右往左第一个bit为1的位置
#define LOW1BIT(X) (debruijn[((unsigned int)(((X) & -(X)) * 0x077CB531U)) >> 27])

#include <stdbool.h>
bool checkLowestNBitsOnes(unsigned int num, int n) {
    unsigned int mask = (1u << n) - 1;
    return (num & mask) == mask;
}

void bit_test(void){
//    uint32_t sce_time = 1706175762;
//    uint8_t temp[] = {0x00,0x00,0x00,0x00};
//    memcpy(temp, &sce_time, sizeof(sce_time));
//    memcpy(&Firmware_Info.OTA_TimesTamp, &temp[0], 4);
//    printf("OTA_TimesTamp = 0x%x(%d)", Firmware_Info.OTA_TimesTamp, Firmware_Info.OTA_TimesTamp);
//    printf("test\n");
//    uint64_t tips = 0x0,confidences = 0x0;
//
//    memcpy(&tips, &Pre_Tip[0], 6);
//    memcpy(&confidences, &Pre_Confidence[0], 6);
//    printf("%d\r\n",(tips^confidences) & confidences);
//    uint8_t temp = 0xff;
//    temp &= ~(BIT(0));
//    printf("result:%d\r\n",LOW1BIT(temp));
    unsigned int number = 0b1011;  // 11111111 in binary
    int n = 4;

    if (checkLowestNBitsOnes(number, n)) {
        printf("The lowest %d bits are all 1s\n", n);
    } else {
        printf("The lowest %d bits are not all 1s\n", n);
    }

    return;
}