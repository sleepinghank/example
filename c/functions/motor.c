//
// Created by 18494 on 2023/12/27.
//

#include "motor.h"
#include "loop.h"

#include <stdio.h>

LOOP_FUNCTION(Motor_Init){
        printf("Motor Module loop\n");
}

int motor_test(){
    printf("motor test\n");
}