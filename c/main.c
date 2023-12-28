//
// Created by 18494 on 2023/12/27.
//

#include "main.h"
#include <stdio.h>
#include <windows.h>
#include "functions/led.h"
#include "functions/motor.h"
#include "functions/loop.h"

LOOP_FUNCTION(Main_Init){
    printf("main Module loop\n");
}

int main()
{

    printf("begin! \n");
    led_test();
    motor_test();

    while (1){
        callInitFunctions();
        Sleep(500);
    }

    return 0;
}