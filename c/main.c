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
#include <sys/time.h>

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


extern combo_t key_combos[];
extern uint8_t active_event;
int main()
{
    init();
    printf("------------------------------------------begin\n");

    // Start measuring time
    struct timeval begin, end;
    gettimeofday(&begin, 0);
//    for (int i = 0; i < 2; ++i) {
//        combo_t *combo = &key_combos[i];
//        printf("combo:%d,long_ticks:%d,keys:%d,%d,event:%d\n", i, combo->long_press_ticks,combo->keys[0], combo->keys[1], combo->event);
//    }
//    return 0;

     node_t* current = _key_code_list->head;
     node_t* curr_tmp = current;

     printf("combo task befor\n");
     while (current != NULL) {
         uint16_t keycode = current->data.key_code;
         printf("Key:0x%02X,", keycode);
         current = current->next;
     }
     printf("\r\n");

    combo_task(KEY_UPDATE);

    uint8_t is_extend = 0;
    current = _key_code_list->head;
    printf("combo task end\n");
    if (current == NULL && active_event == 1 ) {
        current = _key_code_list_extend->head;
        is_extend = 1;
    }

    while (current != NULL) {
        curr_tmp = current;
        current = current->next;
        if (current == NULL && active_event == 1 && is_extend == 0) {
            current = _key_code_list_extend->head;
            is_extend = 1;
        }
        if (curr_tmp->data.is_report == 0) {
            continue;
        }
        uint16_t keycode = curr_tmp->data.key_code;
        printf("0x%02X,", keycode);
    }
     printf("\r\n");

    // Stop measuring time and calculate the elapsed time
    gettimeofday(&end, 0);
    long seconds = end.tv_sec - begin.tv_sec;
    long microseconds = end.tv_usec - begin.tv_usec;
    printf("seconds:%ld,microseconds:%ld\n", seconds, microseconds);

    printf("------------------------------------------end\r\n");
    return 0;
}