//
// Created by 18494 on 2023/12/27.
//

#ifndef C_LOOP_H
#define C_LOOP_H

void callInitFunctions();

typedef void (*LoopFunction)();

#define MAX_INIT_FUNCTIONS 10
extern LoopFunction loopFunctions[];
extern int loopFunctionCount;

#define LOOP_FUNCTION(name) \
    void name(); \
    void __attribute__((constructor)) init_##name() { \
          loopFunctions[loopFunctionCount++] = &name; \
    } \
    void name()

#endif //C_LOOP_H
