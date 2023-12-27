//
// Created by 18494 on 2023/12/27.
//

#include "led.h"

#include <stdio.h>
#include "loop.h"

LOOP_FUNCTION(LED_Init){
        printf("LED Module loop\n");
}

int led_test(){
    printf("led test\n");
}