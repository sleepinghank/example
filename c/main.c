//
// Created by 18494 on 2023/12/27.
//

#include "main.h"
#include <stdio.h>
#include "functions/led.h"
#include "functions/motor.h"
#include "functions/loop.h"
#include "ota/ota.h"
#include "crc/crc16.h"
#include "sklearn/svc/svc.h"
#include "sklearn/bayes/bayes.h"
#include "bit/bit.h"
#include "kalman/filter.h"
#include "hmac_sha256/hmac_c_example.h"
#include "aes/aes_example.h"
#include "aes/AES128.h"
#include <time.h>
#include "pixart/process_combo.h"
#include "pixart/linkedlist.h"
#include "pixart/keycode.h"
#include "pixart/Storage1.h"
#include <sys/time.h>
#include "touchpad_online_update/touchpad_update.h"

LOOP_FUNCTION(Main_Init){
    printf("main Module loop\n");
}

list_t* _key_code_list;
list_t* _key_code_list_extend;

int init(void){
    _key_code_list = make_list_proc();
    _key_code_list_extend = make_list_proc();
    add(KB_OPEN_BRACKET_N_BRANCE, _key_code_list);
    add(KB_A, _key_code_list);
    add(KB_B, _key_code_list);
    add(KB_C, _key_code_list);
    add(KB_CLOSE_BRACKET_N_BRANCE, _key_code_list);
};


int test_1(uint8_t a){
    if (a > 0){
        printf("a:%d\n",a);
    }
    return 0;
}

static uint32_t _count_bit_set(uint8_t num)
{
    uint32_t count = 0;

    while (num)
    {
        count++;
        num &= (num - 1);
    }

    return count;
}


extern combo_t key_combos[];
extern uint8_t active_event;


/*Backlight power down*/
#define BACKLIGHT_POWER_DOWN_INTERVAL               (25 * 1000) /*1 min to ms*/
#define KEYBOARD_INTERVAL                           4
uint32_t _BAKC_LIGHT_POWER_DOWN_cnt_th = BACKLIGHT_POWER_DOWN_INTERVAL / KEYBOARD_INTERVAL;
int main()
{
    init();
    printf("------------------------------------------begin\n");

    printf("------------------------------------------\r\n");
//    start_update();
    float vat = 3450;
    uint16_t  bat_level_temp = 0;

//  x < 2750 y = 0
//  2750 < x < 3350 y=32*x/25 - 3529
//  3350 < x < 3650 y=1697*x/100 - 56070
//  3650 < x < 4050 y=191*x/20 - 28997
//  4050 < x < 4150 y=16*x/5 - 3280
//  4150 < x  y=10000
//    if(vat <= 2750)
//    {
//        bat_level_temp = 0;
//    }
//    else if(vat <= 3350)
//    {
//        bat_level_temp = vat*32/25 - 3529;
//    }
//    else if(vat <= 3650)
//    {
//        bat_level_temp = vat*1697/100 - 56070;
//    }
//    else if(vat <= 4050)
//    {
//        bat_level_temp = vat*191/20 - 28997;
//    }
//    else if(vat <= 4150)
//    {
//        bat_level_temp = vat*16/5 - 3280;
//    }
//    else
//        bat_level_temp = 10000;
    uint8_t bat_level = ((630000 * 100) / 900000 );
    printf("bat_level:%d\n",bat_level);
    printf("------------------------------------------end\r\n");
    return 0;
}