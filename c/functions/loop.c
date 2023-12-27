//
// Created by 18494 on 2023/12/27.
//

#include "loop.h"

LoopFunction  loopFunctions[MAX_INIT_FUNCTIONS];
int loopFunctionCount = 0;


void callInitFunctions(){
    for (int i = 0; i < loopFunctionCount; ++i) {
        loopFunctions[i]();
    }
}
