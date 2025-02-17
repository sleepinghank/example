//
// Created by 18494 on 2023/12/27.
//

#include "main.h"
#include <stdio.h>
//#include "functions/led.h"
//#include "functions/motor.h"
//#include "functions/loop.h"
//#include "ota/ota.h"
//#include "crc/crc16.h"
//#include "sklearn/svc/svc.h"
//#include "sklearn/bayes/bayes.h"
//#include "bit/bit.h"
//#include "kalman/filter.h"
//#include "hmac_sha256/hmac_c_example.h"
//#include "aes/aes_example.h"
//#include "aes/AES128.h"
//#include <time.h>
//#include "pixart/process_combo.h"
//#include "pixart/linkedlist.h"
//#include "pixart/keycode.h"
//#include "pixart/Storage1.h"
//#include "touchpad_online_update/touchpad_update.h"
//#include "timer_task/timer_task.h"
#include "battery/battery_level.h"
#include "preset_gesture/preset_gesture.h"
#include "preset_gesture/mcs_gesture.h"
// LOOP_FUNCTION(Main_Init){
//     printf("main Module loop\n");
// }

#define IDLE_INTERVAL                               (1)//(30 * 60 * 1000) /*2 sec to ms*/
#define KEYBOARD_TIMER                              204UL       /* 6ms, unit 30.5us */
#define KEYBOARD_INTERVAL                           ((KEYBOARD_TIMER + 1) * 1 / 32768)

#define INTERVAL_CNT(interval)                      (32768 * interval / (KEYBOARD_TIMER))

extern void test_gesture_recognition() ;
extern void select_best_gestures() ;
extern void generate_all_gesture_features();

int main(void)
{
    printf("------------------------------------------begin\n");
//    battery_init();
//    printf("init success\n");
//    for (int i = 0; i < 1000; ++i) {
//        battery_level_detection(1);
//    }
//    preset_gesture_init();
    test_gesture_recognition();
//
//    select_best_gestures();
//    generate_all_gesture_features();
    // int result = mcs_test();
    // printf("result:%d",result);
    printf("------------------------------------------end\r\n");
    return 0;
}