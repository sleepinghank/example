

#ifndef _BATTERY_LEVEL_H_
#define _BATTERY_LEVEL_H_
#include <stdint.h>
// #include <stdint.h>
// #include <stdio.h>

// #define DBG(fmt, ...) printf("[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

// //键盘配置结构体
// typedef struct
// {
//     uint8_t backlight_color;//背光板颜色
//     uint8_t backlight_brightness;//背光板亮度
//     uint8_t FN_Lock_flag[3];//FN锁
//     uint8_t SystemType[3];//系统类型
//     uint32_t bat_level;
// #ifdef CONFIG_ENABLE_TOUCH
//     uint8_t touch_flag;//触摸开关
// #endif
//     // uint8_t ORIGIN_UUID[16];//ORIGIN UUID
// }KeyboardConfig_t;

// #define CHARGING_DETECTION  //物理充电检测

#ifdef CHARGING_DETECTION
#define CHARGING_DETECTION_GPIO_PIN 18    //充电检测管脚
#endif


extern uint16_t  current_power_usage;//当前用电量
extern uint8_t   battery_level_percentage; //电池剩余电量（百分比）


// void battery_init(void);
// //电池电量检测
// // void battery_level_detection(uint8_t time);
// //充电检测
// uint8_t battery_charging_detection(void);
// // 电量显示检测
// void Bat_LED(void);
// // 获取当前电量百分比
// uint8_t get_batter_level(void);
// // 电量校准
// void battery_level_calibration(void);



/****
 * public
 * 1. 电量管理初始化方法
 * 2. 定时电量检测
 *
 * private
 * 1. 获取当前检测电池电压
 * 2. 根据当前电压计算电池剩余容量
 * 3. 根据当前电池容量计算电池剩余电量百分比
 * 4. 根据当前容量计算电池电压
 * 5. 计算当前充电状态
 * 6. 根据当前充电状态计算当前电量变化
 * 7. 根据当前电量变化计算电池剩余容量
 * 8. 检查当前电量状态，进行低电量警告、低电量关机
 */

/**
 * @brief 获取当前检测电池电压，进行滤波，转换为电压值.
 * @return 电池电压 单位mV
 */
uint16_t get_battery_voltage(void);

/**
 * @brief 根据当前电压计算电池剩余百分比
 * @param voltage 当前电压
 * @return 电池剩余电量百分比
 */
uint8_t convert_voltage_to_percentage(uint16_t voltage);

/**
 * @brief 根据当前容量计算电池剩余电池容量
 * @param remaining_capacity 当前容量
 * @return 电池剩余容量 单位mAs
 */
inline uint32_t calculate_remaining_capacity(uint8_t percentage);

/**
 * @brief 根据当前容量计算电池剩余百分比
 * @param remaining_capacity 当前容量
 * @return 电池剩余百分比
 */
uint8_t calculate_capacity_percentage(uint32_t remaining_capacity);
/**
 * @brief 根据当前容量计算预估的电池电压
 * @param remaining_capacity 当前容量
 * @return 预估的电池电压 单位mV
 */
uint16_t calculate_voltage(uint32_t remaining_capacity);

/**
 * @brief 计算当前充电状态
 * @return 充电状态：1-充电，0-放电
 */
uint8_t calculate_charging_state(void);

/**
 * @brief 根据背光灯的亮度跟颜色计算用电量，单位 mAs
 * @return 预计放电容量 mAs
 */
uint16_t calculate_power_consumption(void);

/**
 * @brief 根据当前充电状态计算当前电量变化
 * @param charging_state 充电状态
 * @return 当前电量变化 单位mAs
 */
int16_t calculate_current_power_usage(uint8_t charging_state);

/**
 * @brief 检查当前电量状态，进行低电量警告、低电量关机
 */
void check_low_battery(void);

/**
 * @brief 电量检测初始化
 */
void battery_init(void);

/**
 * @brief 电量检测，需要循环调用，
 * @param time threshold：传入值为循环次数
 */
void battery_level_detection(uint16_t threshold);

//键盘配置结构体
typedef struct
{
    uint8_t backlight_color;//背光板颜色 1
    uint8_t backlight_brightness;//背光板亮度 1
    // uint8_t FN_Lock_flag[3];//FN锁
    uint8_t SystemType[3];//系统类型 3
    uint32_t remaining_capacity_mas; //4
    uint8_t sys_power_state;//系统电源   1:开机  2:关机  3:休眠 1
}KeyboardConfig_t;


#endif //_BATTERY_LEVEL_H_
