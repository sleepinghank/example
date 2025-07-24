//
// Created by 18494 on 2023/12/27.
//

#include "main.h"
#include <stdio.h>
#include <string.h>
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
//#include "battery/battery_level.h"
//#include "cmd_uart/cmd_uart.h"
//#include "preset_gesture/preset_gesture.h"
//#include "preset_gesture/mcs_gesture_test.h"
#include "algorithmic/algorithmic_filtering.h"
// LOOP_FUNCTION(Main_Init){
//     printf("main Module loop\n");
// }

#define REPORT_TIMER_ID                             0
#define KEYBOARD_TIMER                              183UL       /* 6ms, unit 30.5us */
#define KEYBOARD_INTERVAL                           ((KEYBOARD_TIMER + 1) * 1000 / 32768)
#define KEYBOARD_CNT(interval)                      (32768 * interval / (KEYBOARD_TIMER + 1))
#define KEYBOARD_CNT_MS(interval)                      ((interval * 178) / 1000)


extern void test_mcs(void);
extern void test_battery(void);

#define  ExternalCode  "N0046"
#define  FirmwareCode  "KB09119-10" //固件代码
#define  CountryCode   "DE" //国家代码
#define  SoftwareVersion     (0x000012)  //软件版本号
#define BIT15					0x8000



#define POWER_DOWN_INTERVAL                       (10 * 60 * 1000)
#define BACKLIGHT_OFF     180
#define BACKLIGHT_LOW     162 // 10%
#define BACKLIGHT_MEDIUM  144 // 20%
#define BACKLIGHT_HIGH    126 // 30%
// 获取下一个预设亮度级别
static uint8_t get_next_preset_brightness(uint8_t current_brightness) {
    const uint8_t preset_levels[] = {BACKLIGHT_OFF,BACKLIGHT_LOW, BACKLIGHT_MEDIUM, BACKLIGHT_HIGH };
    const uint8_t level_count = sizeof(preset_levels) / sizeof(preset_levels[0]);

    // 如果是自定义亮度，返回最接近的预设值的下一级
    for (uint8_t i = 0; i < level_count; i++) {
        if (current_brightness > preset_levels[i]) {
            return preset_levels[i];
        }
    }
    return BACKLIGHT_OFF; // 如果当前亮度大于所有预设值，返回最低亮度
}
#define  ProductModel        "inateck AceTouch Pro" //产品型号
#define MAX_VOLTAGE_TABLE 21
const uint16_t bat_vol_table[MAX_VOLTAGE_TABLE]={0x236,0x230,0x22B,0x225,0x21E,0x219,0x212,0x20C,0x206,0x201,0x1FD,0x1F9,0x1F7,499,496,493,489,485,481,468,437};// 新500mA 针对KB04122微调
#define NUM_OF_COLS 16
#define NUM_OF_ROWS  8
uint8_t key_output_io[NUM_OF_COLS] = {0, 1, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 17, 18, 19};
uint8_t key_input_io[NUM_OF_ROWS] = {25, 26, 27, 28, 29, 30, 31, 2};
#define U32BIT(s)			((uint32_t)1<<(s))
#define BIT(s)				((uint8_t)1<<(s))
extern void test_filters(void);

#define KEYBOARD_TIMER                              255UL       /* 6ms, unit 30.5us */
#define KEYBOARD_INTERVAL                           ((KEYBOARD_TIMER + 1) * 1000 / 32768)
#define KEYBOARD_CNT(interval)                      (32768 * interval / (KEYBOARD_TIMER))
#define KEYBOARD_CNT_MS(interval)                      ((interval * 128) / 1000)
int main(void)
{
    uint8_t  a = KEYBOARD_CNT_MS(1000);
    printf("Keyboard count for 1 second: %d\n", a);
    return 0;
}