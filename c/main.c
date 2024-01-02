//
// Created by 18494 on 2023/12/27.
//

#include "main.h"
#include <stdio.h>
#include <windows.h>
#include "functions/led.h"
#include "functions/motor.h"
#include "functions/loop.h"
#include "ota/ota.h"
#include "crc/crc16.h"
LOOP_FUNCTION(Main_Init){
    printf("main Module loop\n");
}



int main()
{
    printf("begin\n");
//    uint8_t data[] = {0x10,0x20,0x04,0x03,0x02,0x01};
//    ota_cmd(data,6);
//    printf("\n begin! \n");

    uint8_t  buff[] = {224, 92, 0, 32, 133, 2, 0, 16, 141, 2, 0, 16, 143, 2, 0, 16};

    uint16_t crc_16 = GetQuickCRC16(buff, 16);
    printf("crc_16 = %d\n",crc_16);
//    led_test();
//    motor_test();
//
//    while (1){
//        callInitFunctions();
//        Sleep(500);
//    }

    return 0;
}