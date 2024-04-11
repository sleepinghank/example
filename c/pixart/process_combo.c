//
// Created by Hank on 2024/4/10.
//

#include "process_combo.h"
#include "linkedlist.h"
#include <stdio.h>
#include <string.h>

#    define COMBO_DISABLED(combo) (combo->disabled)
#    define COMBO_STATE(combo) (combo->state)

#    define DISABLE_COMBO(combo)    \
        do {                        \
            combo->disabled = true; \
        } while (0)
#    define RESET_COMBO_STATE(combo) \
        do {                         \
            combo->disabled = false; \
            combo->event    = 0;     \
        } while (0)
#define NO_COMBO_KEYS_ARE_DOWN (0 == COMBO_STATE(combo))
#define ALL_COMBO_KEYS_ARE_DOWN(state, key_count) (((1 << key_count) - 1) == state)
#define ONLY_ONE_KEY_IS_DOWN(state) !(state & (state - 1))
#define KEY_NOT_YET_RELEASED(state, key_index) ((1 << key_index) & state)
#define KEY_STATE_DOWN(state, key_index) \
    do {                                 \
        state |= (1 << key_index);       \
    } while (0)
#define KEY_STATE_UP(state, key_index) \
    do {                               \
        state &= ~(1 << key_index);    \
    } while (0)
#define PRESS_REPEAT_MAX_NUM  15 /*!< The maximum value of the repeat counter */

extern list_t* _key_code_list;          // current press key
extern list_t* _key_code_list_extend;          // current press key
extern combo_t key_combos[];

/**
  * @brief  组合键触发后，按键状态机判断
  * @param  combo_t *combo: 组合键对象
  * @param  uint8_t is_active: 组合是否激活
  */
void button_ticks(combo_t *combo, uint8_t is_active);
/**
  * @brief  从_key_code_list中删除激活的组合键
  * @param  combo_t *combo: 组合键对象
  */
void del_combo_keys(const uint16_t *keys);
/**
  * @brief  将激活的组合键加入_key_code_list中
  * @param  combo_t *combo: 组合键对象
  * @param  uint8_t* buf: 数组指针，全局缓存数组，防止重复申请内存
  */
void add_combo_result(const combo_t *combo,uint8_t* arr);
/**
  * @brief  判断组合键是否触发
  * @param  uint16_t combo_index: 组合键索引
  * @param  combo_t *combo: 组合键对象
  * @retval uint8_t: 组合键是否触发
  */
uint8_t apply_combo(uint16_t combo_index, combo_t *combo);

/**
  * @brief  查找键在当前组合键中索引和总计数
  * @param  const uint16_t *keys: 组合键数组
  * @param  uint16_t keycode: 当前按键
  * @param uint16_t *key_index: 当前按键索引指针
  * @param uint8_t *key_count: 当前按键总数指针
  */
static inline void _find_key_index_and_count(const uint16_t *keys, uint16_t keycode, uint16_t *key_index, uint8_t *key_count) {
    while (true) {
        uint16_t key = pgm_read_word(&keys[*key_count]);
        if (keycode == key) *key_index = *key_count;
        if (COMBO_END == key) break;
        (*key_count)++;
    }
}

uint8_t apply_combo(uint16_t combo_index, combo_t *combo) {
    node_t* current = _key_code_list->head;
    uint8_t state = 0;

    if (COMBO_DISABLED(combo)) {
        return 0;
    }
    while (current != NULL) {
        uint16_t keycode = current->data.key_code;
        uint8_t  key_count = 0;
        uint16_t key_index = -1;
        current ->data.is_report = 1;
        _find_key_index_and_count(combo->keys, keycode, &key_index, &key_count);

        if (-1 == (int16_t)key_index) {
            // key not part of this combo
            current = current->next;
            continue;
        }
        KEY_STATE_DOWN(state, key_index);
        if (ALL_COMBO_KEYS_ARE_DOWN(state, key_count)) {
            // All keys are down
            return 1;
        }
        current = current->next;
    }
}


void button_ticks(combo_t *combo, uint8_t is_active) {
    //ticks counter working..
    if((combo->state) > 0) combo->ticks++;

    /*------------button debounce combo---------------*/
    if(is_active != combo->button_level) { //not equal to prev one
        //continue read 3 times same new level change
        if(++(combo->debounce_cnt) >= DEBOUNCE_TICKS) {
            combo->button_level = is_active;
            combo->debounce_cnt = 0;
        }
    } else { //level not change ,counter reset.
        combo->debounce_cnt = 0;
    }

    /*-----------------State machine-------------------*/
    switch (combo->state) {
        case 0:
            if(combo->button_level == combo->active_level) {	//start press down
                combo->event = (uint8_t)PRESS_DOWN;
//                    EVENT_CB(PRESS_DOWN);
                combo->ticks = 0;
                combo->repeat = 1;
                combo->state = 1;
            } else {
                combo->event = (uint8_t)NONE_PRESS;
            }
            break;

        case 1:
            if(combo->button_level != combo->active_level) { //released press up
                combo->event = (uint8_t)PRESS_UP;
//                    EVENT_CB(PRESS_UP);
                combo->ticks = 0;
                combo->state = 2;
            } else if(combo->ticks > combo->long_press_ticks) { // Customize the long press duration
                combo->event = (uint8_t)LONG_PRESS_START;
//                    EVENT_CB(LONG_PRESS_START);
                combo->state = 5;
            }
            break;

        case 2:
            if(combo->button_level == combo->active_level) { //press down again
                combo->event = (uint8_t)PRESS_DOWN;
//                    EVENT_CB(PRESS_DOWN);
                if(combo->repeat != PRESS_REPEAT_MAX_NUM) {
                    combo->repeat++;
                }
//                    EVENT_CB(PRESS_REPEAT); // repeat hit
                combo->ticks = 0;
                combo->state = 3;
            } else if(combo->ticks > SHORT_TICKS) { //released timeout
                if(combo->repeat == 1) {
                    combo->event = (uint8_t)SINGLE_CLICK;
//                        EVENT_CB(SINGLE_CLICK);
                } else if(combo->repeat == 2) {
                    combo->event = (uint8_t)DOUBLE_CLICK;
//                        EVENT_CB(DOUBLE_CLICK); // repeat hit
                }
                combo->state = 0;
            }
            break;

        case 3:
            if(combo->button_level != combo->active_level) { //released press up
                combo->event = (uint8_t)PRESS_UP;
//                    EVENT_CB(PRESS_UP);
                if(combo->ticks < SHORT_TICKS) {
                    combo->ticks = 0;
                    combo->state = 2; //repeat press
                } else {
                    combo->state = 0;
                }
            } else if(combo->ticks > SHORT_TICKS) { // SHORT_TICKS < press down hold time < LONG_TICKS
                combo->state = 1;
            }
            break;

        case 5:
            if(combo->button_level == combo->active_level) {
                //continue hold trigger
                combo->event = (uint8_t)LONG_PRESS_HOLD;
//                    EVENT_CB(LONG_PRESS_HOLD);
            } else { //released
                combo->event = (uint8_t)PRESS_UP;
//                    EVENT_CB(PRESS_UP);
                combo->state = 0; //reset
            }
            break;
        default:
            combo->state = 0; //reset
            break;
    }
}

void add_combo_result(const combo_t *combo,uint8_t* buf) {
    memset(buf, 0, 10);
    {
        uint8_t u8temp;
        uint8_t idx = combo->cb[combo->event](buf);
        for (u8temp = 0; u8temp < idx; ++u8temp) {
            add(buf[u8temp], _key_code_list_extend);
        }
    }
}

void del_combo_keys(const uint16_t *keys) {
    uint8_t key_count = 0;
    while (true) {
        uint16_t key = pgm_read_word(&keys[key_count]);
        if (COMBO_END == key) break;
        deactivate(key, _key_code_list);
        key_count++;
    }
}
extern uint8_t number_of_combos;
// 按键事件处理
// 1. 循环key_combos，从_key_code_list中判断是否满足组合键条件
// 2. 满足函数进行按键状态机处理，判断满足状态的事件
// 3. 满足事件后，调用回调函数，从_key_code_list中删除组合键，添加新增的键。
void combo_task(key_update_st_t _keyUpdateSt){
    uint8_t u8temp ;
    uint8_t buf[10] = {0};
    uint8_t is_active, active_event = 0;
    del_all_child(_key_code_list_extend);
    if (_keyUpdateSt == NO_KEY_UPDATE || _keyUpdateSt == GHOST_KEY){
        return;
    }

    // 判断是否触发组合键以及按键状态机处理
    for (u8temp = 0; u8temp < number_of_combos; ++u8temp) {
        combo_t *combo = &key_combos[u8temp];
        // 判断是否触发事件
        is_active = apply_combo(u8temp, combo);
        //  按键状态机处理
        button_ticks(combo, is_active);
        if (combo->event < number_of_event && combo->cb[combo->event]) {
            active_event = 1;
        }
    }
    if (!active_event) {
        return;
    }
    // 组合键触发事件处理
    for (u8temp = 0; u8temp < number_of_combos; ++u8temp) {
        combo_t *combo = &key_combos[u8temp];
        // 判断是否存在触发事件
        if (combo->event < number_of_event  && combo->cb[combo->event]) {
            // 先删除组合键
            del_combo_keys(combo->keys);
            // 添加新的键
            add_combo_result(combo,buf);
            RESET_COMBO_STATE(combo);
        }
    }
}

