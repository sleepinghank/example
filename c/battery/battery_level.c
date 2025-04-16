#include "battery_level.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
//#include "debug.h"
//#include "debug_log.h"
//#include "back_LED.h"
//#include "config.h"
//#include "gpadc.h"
//#include "pliot_lamp_LED.h"
//#include "SET.h"
//#include "Storage1.h"
//#include "ble.h"
//#include "HID_OVER_GATT_KEYBOARD_OTA_Profile_PTP.h"
/**
 * 假设电池容量是150mAh= 540000mAs
 * 在10mA下可以使用的时间为540000mAs / 10 = 54000秒
 * 10mA下可以使用54000秒
 * 每一秒进行一次电量计算
 * 开/关背光，背光灯亮度都需要计算进来
 * ADC采集只作为电量趋势判断
 *
 * BATTERY_CAPACITY, VOLTAGE_COEFFICIENT, current_power_usage 需要根据实际情况修改
 * 涓流充电、恒流充电、恒压充电的电量计算需要根据实际情况修改
 */
// Constants for battery management
#define BATTERY_CAPACITY_MAH    500     // Battery capacity in mAh
#define BATTERY_CAPACITY_MAS    (BATTERY_CAPACITY_MAH * 3600)  // Convert to mAs
#define VOLTAGE_DIVIDER_RATIO   493     // Voltage divider ratio (R2/(R1+R2))*1024
#define MIN_VOLTAGE_MV          3000    // Minimum operating voltage in mV
#define LOW_BATTERY_WARN_PCT    20      // Low battery warning percentage
#define LOW_BATTERY_SHUTDOWN_PCT 5      // Low battery shutdown percentage

#define DEBUG_TEST

#ifdef DEBUG_TEST

static uint32_t adc_idx  = 0;

int rand_test(void){
    int a;
    srand(adc_idx);
    a = rand() % 10;
    return a;
}
// 满电：575 0%： 410
#define ADC_LEVEL 575
static uint16_t adc_buf[] = {521,521,519,516,520,521,519,517,521,522};
#define FILTER_N 10
uint16_t gpadc_get_value(){
    uint16_t voltage = 0;
   if(adc_idx >= FILTER_N)adc_idx = 0;
    // int randint = rand_test();
    // voltage = 0x226 - (adc_idx/10)*2 + randint;
    
//    if (adc_idx > 100) {
//        voltage = 561;
//    } else {
//         adc_idx++;
//    }

    // printf("adc_idx=%d,----------------voltage = %d \r\n",adc_idx,voltage);
    return adc_buf[adc_idx];
}

// 添加卡尔曼滤波相关结构体
typedef struct {
    float estimate;     // 当前估计值
    float errorEst;     // 估计误差
    float kalmanGain;   // 卡尔曼增益
    float errorMea;     // 测量误差
    float processNoise; // 过程噪声
} SimpleKalman_t;


// 添加卡尔曼滤波器实例
static SimpleKalman_t batteryKalman = {
        .estimate = 100.0f,    // 初始估计值为100%
        .errorEst = 1.0f,      // 初始估计误差
        .kalmanGain = 0.0f,    // 卡尔曼增益将在更新时计算
        .errorMea = 50.0f,      // 测量误差（可调，值越大表示越不信任电压测量）
        .processNoise = 0.1f   // 过程噪声（可调，值越大表示越不信任库仑计数）
};

// 添加简化的卡尔曼滤波计算函数
float kalman_filter_update(float measurement, float coulomb_count) {
    // 预测步骤：使用库仑计数作为预测值
    batteryKalman.estimate = coulomb_count;

    // 预测误差更新
    batteryKalman.errorEst += batteryKalman.processNoise;

    // 计算卡尔曼增益
    batteryKalman.kalmanGain = batteryKalman.errorEst /
                               (batteryKalman.errorEst + batteryKalman.errorMea);

    // 更新步骤：结合测量值进行校正
    batteryKalman.estimate += batteryKalman.kalmanGain *
                              (measurement - batteryKalman.estimate);

    // 更新估计误差
    batteryKalman.errorEst = (1.0f - batteryKalman.kalmanGain) *
                             batteryKalman.errorEst;

    // 确保估计值在有效范围内
    if (batteryKalman.estimate > 100.0f) batteryKalman.estimate = 100.0f;
    if (batteryKalman.estimate < 0.0f) batteryKalman.estimate = 0.0f;

    return batteryKalman.estimate;
}



#endif

// Battery state structure
typedef struct {
    uint32_t remainingCapacityMas;  // Remaining capacity in mAs
    uint8_t percentageRemaining;     // Remaining capacity as percentage
    uint16_t voltageMv;             // Current voltage in mV
    int16_t currentDrawMa;         // Current power usage in mA
    uint8_t isCharging;                // Charging status
} BatteryState_t;

static BatteryState_t batteryState = {
        .remainingCapacityMas = BATTERY_CAPACITY_MAS,
        .percentageRemaining = 100,
        .voltageMv = 0xFFFF,
        .currentDrawMa = 0,
        .isCharging = 0
};

static const uint8_t  trickle_charge = 10; // 涓流充电(单位:mAs)
static const uint8_t constant_current = 100; // 恒流充电(单位:mAs)
static const uint8_t constant_voltage = 10; // 恒压充电(单位:mAs)

uint8_t battery_check_counter = 0; // 电池电量检测计数器
uint8_t anti_jump_change = 0; // 防止电压跳变

// 电压-容量对照表
static const uint16_t VOLTAGE_TO_CAPACITY_TABLE[11]={420,402,391,385,379,375,370,368,366,362,300};

#define FULL_BATTERY_VOLTAGE 420 // 4.2V 表示为 4200 mV
#define CHARGING_VOLTAGE_THRESHOLD 10 // 充电电压阈值，单位 mV
#define VOLTAGE_RISE_THRESHOLD 15 // 电压突然上升阈值，单位 mV

/**
 * 420 100%
 * 410 95%
 * 400 90%
 * 390 80%
 * 380 65%
 * 370 45%
 * 360 30%
 * 350 20%
 * 340 20%
 * 330 15%
 * 320 10%
 * 310 5%
 * 300 0%
 */


uint8_t power_st = 0;

KeyboardConfig_t KeyboardConfig;

// extern uint8_t backlight_color;//颜色
// extern uint8_t backlight_brightness;//亮度
// extern void power_down_cb(void);
extern void led_low_bat_start(void);
extern void led_low_bat_stop(void);


#define GET_DIFFERENCE(voltageA, voltageB) ((voltageA) > (voltageB) ? (voltageA) - (voltageB) : (voltageB) - (voltageA))

#ifdef KEYCODE_BKLED
extern uint8_t _backlight_falg;

#endif

uint8_t get_batter_level(void){
    return batteryState.percentageRemaining;
}

// uint16_t filter(uint16_t *voltage_buf, uint8_t length) {
//     uint16_t sum = 0;
//     for (int i = 0; i < length; i++) {
//         sum += voltage_buf[i];
//     }
//     return sum / length;
// }
// 冒泡排序辅助函数
void bubble_sort(uint16_t arr[], uint8_t len) {
    uint8_t i, j;
    uint16_t temp;
    for (i = 0; i < len - 1; i++) {
        for (j = 0; j < len - 1 - i; j++) {
            if (arr[j] > arr[j + 1]) {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// 中位值平均滤波
uint16_t filter(uint16_t *buf, uint8_t length) {
    if (length < 3) return buf[0];  // 数据太少时直接返回

    // 创建临时数组，避免修改原始数据
    uint16_t temp[FILTER_N];
    memcpy(temp, buf, length * sizeof(uint16_t));
    
    // 排序
    bubble_sort(temp, length);
    
    // 去除最高和最低的若干个值，取中间值的平均
    uint8_t remove_count = length / 6;  // 去除首尾的数据量
    if (remove_count < 1) remove_count = 1;
    
    uint32_t sum = 0;
    uint8_t valid_count = 0;
    
    // 计算中间值的平均值
    for (uint8_t i = remove_count; i < length - remove_count; i++) {
        sum += temp[i];
        valid_count++;
    }
    
    return (uint16_t)((sum + (valid_count >> 1)) / valid_count);
}

static uint16_t voltage_buf[FILTER_N];
static uint8_t voltage_buf_idx = 0xff;
static uint16_t last_voltage = 0xffff;
uint16_t get_battery_voltage(void){
    uint16_t voltage = 0;
//    gpadc_start();
    if(voltage_buf_idx == 0xff){
        voltage_buf_idx = 0;
        for(uint8_t i = 0; i < FILTER_N; i++){
            voltage = gpadc_get_value();
            voltage_buf[voltage_buf_idx] = voltage;
            voltage_buf_idx++;
//            pxi_delay_ms(10);
        }
    } else {
        voltage = gpadc_get_value();
        voltage_buf[voltage_buf_idx] = voltage;
        voltage_buf_idx++;
        if(voltage_buf_idx >= FILTER_N){
            voltage_buf_idx = 0;
        }
    }

    voltage = filter(voltage_buf,FILTER_N);
    printf("voltage = %d \r\n",voltage);
    voltage = (uint16_t)((uint32_t)voltage * 360 / VOLTAGE_DIVIDER_RATIO); // ADC转换为电压
    printf("cal voltage = %d \r\n",voltage);
    if(GET_DIFFERENCE(last_voltage, voltage) > 2)
    {
        // uint8_t the_unit = 0;
        last_voltage = voltage;

        // the_unit = (last_voltage % 10);
        // // 四舍五入
        // if(the_unit >= 5) last_voltage = (last_voltage - the_unit) + 10;
        // else last_voltage = last_voltage - the_unit;
    }
    return last_voltage;
}

uint8_t convert_voltage_to_percentage(uint16_t voltage){
    uint8_t i = 0, level = 0;
    // 容量初始化
    for (i = 0; i < 11; i++)
    {
        if(voltage >= VOLTAGE_TO_CAPACITY_TABLE[i])
        {
            level = 100 - (i * 10);
            if (i != 0){
                level += (uint8_t)(voltage - VOLTAGE_TO_CAPACITY_TABLE[i])*10/(VOLTAGE_TO_CAPACITY_TABLE[i-1] - VOLTAGE_TO_CAPACITY_TABLE[i]);
            }
            break;
        }
    }
    return level;
}

inline uint32_t calculate_remaining_capacity(uint8_t percentage){
    return (BATTERY_CAPACITY_MAS * percentage) / 100;
}

// 根据剩余容量计算百分比
uint8_t calculate_capacity_percentage(uint32_t remaining_capacity){
    return (uint8_t)((uint32_t)(remaining_capacity * 100 + BATTERY_CAPACITY_MAS / 2) / BATTERY_CAPACITY_MAS);
}

uint16_t calculate_voltage(uint32_t remaining_capacity){
    uint8_t i;
    uint16_t voltage;
    uint8_t percentage = (remaining_capacity * 100) / BATTERY_CAPACITY_MAS;
    printf("---------------------percentage = %d \r\n",percentage);
    // 根据电量百分比查表获取对应电压值
    for (i = 10; i > 0; i--)
    {
        if (percentage >= i * 10)
        {
            voltage = VOLTAGE_TO_CAPACITY_TABLE[10-i];
            if (i == 10)
            {
                break;
            }
//            uint16_t t  = (VOLTAGE_TO_CAPACITY_TABLE[9-i]-VOLTAGE_TO_CAPACITY_TABLE[10-i]) * (percentage - (100 - i * 10)) / 100;
//            printf("voltage = %d,t = %d \r\n",voltage,t);
            voltage += (VOLTAGE_TO_CAPACITY_TABLE[9-i]-VOLTAGE_TO_CAPACITY_TABLE[10-i]) * (percentage - (100 - i * 10)) / 100;
            break;
        }
    }

    // 如果电量为0，返回最低电压值
    if (i == 0)
    {
        voltage = VOLTAGE_TO_CAPACITY_TABLE[10];
    }

    return voltage;
}

// 添加最小二乘法计算斜率的函数
float calculate_voltage_slope(uint16_t *buf, uint8_t length) {
    float sum_x = 0;    // Σx
    float sum_y = 0;    // Σy
    float sum_xy = 0;   // Σ(xy)
    float sum_xx = 0;   // Σ(x²)
    
    // 计算各项和
    for (uint8_t i = 0; i < length; i++) {
        sum_x += i;
        sum_y += buf[i];
        sum_xy += i * buf[i];
        sum_xx += i * i;
    }
    
    // 计算斜率 k = (n*Σ(xy) - Σx*Σy) / (n*Σ(x²) - (Σx)²)
    float n = length;
    float slope = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
    
    return slope;
}


uint8_t calculate_charging_state(void){
    static uint8_t ChargingDetection = 0;

#ifdef CHARGING_DETECTION
    gpi_config(CHARGING_DETECTION_GPIO_PIN, GPI_PULL_UP, GPI_NOT_INVERTED); //充电检测管脚

        if(gpi_get_val(CHARGING_DETECTION_GPIO_PIN) == 0)
        {
            ChargingDetection = 1; // 充电

            power_st = 0xBB; // 充电状态设置未充电是0xAF 充电是0xBB
        }
        else
        {
            ChargingDetection = 0; // 放电
            power_st = 0xAF; // 充电状态设置未充电是0xAF 充电是0xBB
        }
#else
    static uint16_t last_voltages[3] = {0xffff,0xffff,0xffff};

    if(anti_jump_change > 0)anti_jump_change--;//防止电压跳变
    else
    {
        printf("battery_voltage = %d,last_voltages[0] = %d \r\n",batteryState.voltageMv,last_voltages[0]);
        // if ((GET_DIFFERENCE(battery_voltage, last_voltages[0]) >=  20) && (last_voltages[0] != 0xffff))
        // {

        //     // if(last_voltages[0] > battery_voltage)//正在放电
        //     // {
        //     //     ChargingDetection = 0;//放电
        //     //     power_st = 0xAF;
        //     // }
        //     // else if(last_voltages[0] < battery_voltage)//正在充电
        //     // {
        //     //     ChargingDetection = 1;//充电
        //     //     power_st = 0xBB;
        //     // }
        //     if(last_voltages[0] < battery_voltage && voltage_buf[voltage_buf_idx] >= 561)//正在充电
        //     {
        //         ChargingDetection = 1;//充电
        //         power_st = 0xBB;
        //     }
        //     else if(last_voltages[0] > battery_voltage)//正在放电
        //     {
        //         ChargingDetection = 0;//放电
        //         power_st = 0xAF;
        //     }
        // }
        // last_voltages[0] = last_voltages[1];
        // last_voltages[1] = last_voltages[2];
        // last_voltages[2] = battery_voltage;
        //  DBG("battery_voltage = %d last_voltages[0] = %d last_voltages[1] = %d last_voltages[2] = %d \r\n",battery_voltage,last_voltages[0],last_voltages[1],last_voltages[2]);

        // 计算电压变化趋势
        float voltage_slope = calculate_voltage_slope(voltage_buf, FILTER_N);
        printf("voltage_slope = %f\r\n", voltage_slope);    


        // 设定斜率阈值，根据实际情况调整
        const float CHARGING_SLOPE_THRESHOLD = 0.5;    // 充电斜率阈值
        const float DISCHARGING_SLOPE_THRESHOLD = -0.3; // 放电斜率阈值
        
        printf("Voltage slope: %.2f\n", voltage_slope);
        
        // 根据斜率判断充放电状态
        if (voltage_slope > CHARGING_SLOPE_THRESHOLD) {

            uint16_t  a = calculate_voltage(batteryState.remainingCapacityMas);
            int16_t diff = a - batteryState.voltageMv;
            printf("a=%d,diff = %d \r\n",a,diff);
            if (diff >= 100)
            {
                ChargingDetection = 1; // 充电
                power_st = 0xBB;
                printf("Charging detected (slope analysis). Slope: %.2f\n", voltage_slope);
            }
        } else if (voltage_slope < DISCHARGING_SLOPE_THRESHOLD) {
            ChargingDetection = 0; // 放电
            power_st = 0xAF;
            printf("Discharging detected (slope analysis). Slope: %.2f\n", voltage_slope);
        }
        // 如果斜率在阈值之间，保持当前状态


        // 检测电池电压突然上升并达到充电电压
        // if ((batteryState.voltageMv >= FULL_BATTERY_VOLTAGE - CHARGING_VOLTAGE_THRESHOLD) &&
        //     (batteryState.voltageMv - last_voltages[0] >= VOLTAGE_RISE_THRESHOLD)) {
        //     ChargingDetection = 1; // 充电
        //     power_st = 0xBB;
        //     printf("Charging detected. Battery voltage: %d mV\r\n", batteryState.voltageMv);
        // } else if (batteryState.voltageMv < last_voltages[0]) {
        //     ChargingDetection = 0; // 放电
        //     power_st = 0xAF;
        //     printf("Discharging detected. Battery voltage: %d mV\r\n", batteryState.voltageMv);
        // }





        // 更新电压历史记录
        last_voltages[0] = last_voltages[1];
        last_voltages[1] = last_voltages[2];
        last_voltages[2] = batteryState.voltageMv;
    }
#endif
#ifdef BATTERY_POWER_STATE_IDX
    if (ble_is_connected() && ble_check_notify_enabled()){
        // 充电状态设置未充电是0xAF 充电是0xBB
        ble_tx_data(BATTERY_POWER_STATE_IDX, 1, &power_st);
    }
#endif
    return ChargingDetection;
}

uint16_t calculate_power_consumption(void)
{
    uint16_t current_power_usage = 0;
#ifdef KEYCODE_BKLED
    static uint8_t brightness = 0xff;

    uint16_t current_power_usage = 0;
    if (KeyboardConfig.backlight_brightness == BACKLIGHT_OFF || _backlight_falg == 0)
    {
        // 需要根据实际情况修改
        current_power_usage = 5; // 当前用电量

        if(brightness != KeyboardConfig.backlight_brightness)
        {
            anti_jump_change = 5; // 防止电压跳变
        }
        brightness = KeyboardConfig.backlight_brightness;
    }else
    {
        switch (KeyboardConfig.backlight_color)
        {
            case 1:
                // 黄
                if (KeyboardConfig.backlight_brightness == BACKLIGHT_LOW)        current_power_usage = 17;//当前用电量
                else if(KeyboardConfig.backlight_brightness == BACKLIGHT_MEDIUM) current_power_usage = 33;//当前用电量
                else if(KeyboardConfig.backlight_brightness == BACKLIGHT_HIGH)   current_power_usage = 50;//当前用电量
                break;
            case 2:
                // 青
                if (KeyboardConfig.backlight_brightness == BACKLIGHT_LOW)        current_power_usage = 18;//当前用电量
                else if(KeyboardConfig.backlight_brightness == BACKLIGHT_MEDIUM) current_power_usage = 36;//当前用电量
                else if(KeyboardConfig.backlight_brightness == BACKLIGHT_HIGH)   current_power_usage = 54;//当前用电量
                break;
            case 3:
                // 绿
                if (KeyboardConfig.backlight_brightness == BACKLIGHT_LOW)        current_power_usage = 12;//当前用电量
                else if(KeyboardConfig.backlight_brightness == BACKLIGHT_MEDIUM) current_power_usage = 21;//当前用电量
                else if(KeyboardConfig.backlight_brightness == BACKLIGHT_HIGH)   current_power_usage = 31;//当前用电量
                break;

            case 4:
                // 蓝
                if (KeyboardConfig.backlight_brightness == BACKLIGHT_LOW)        current_power_usage = 12;//当前用电量
                else if(KeyboardConfig.backlight_brightness == BACKLIGHT_MEDIUM) current_power_usage = 21;//当前用电量
                else if(KeyboardConfig.backlight_brightness == BACKLIGHT_HIGH)   current_power_usage = 30;//当前用电量
                break;

            case 5:
                //红色
                if (KeyboardConfig.backlight_brightness == BACKLIGHT_LOW)        current_power_usage = 13;//当前用电量
                else if(KeyboardConfig.backlight_brightness == BACKLIGHT_MEDIUM) current_power_usage = 25;//当前用电量
                else if(KeyboardConfig.backlight_brightness == BACKLIGHT_HIGH)   current_power_usage = 37;//当前用电量
                break;
            case 6:
                // 紫
                if (KeyboardConfig.backlight_brightness == BACKLIGHT_LOW)        current_power_usage = 18;//当前用电量
                else if(KeyboardConfig.backlight_brightness == BACKLIGHT_MEDIUM) current_power_usage = 35;//当前用电量
                else if(KeyboardConfig.backlight_brightness == BACKLIGHT_HIGH)   current_power_usage = 53;//当前用电量
                break;
            case 0:
            default:
                // 白色
                if (KeyboardConfig.backlight_brightness == BACKLIGHT_LOW)        current_power_usage = 20;//当前用电量
                else if(KeyboardConfig.backlight_brightness == BACKLIGHT_MEDIUM) current_power_usage = 41;//当前用电量
                else if(KeyboardConfig.backlight_brightness == BACKLIGHT_HIGH)   current_power_usage = 62;//当前用电量
                break;
        }
        brightness = KeyboardConfig.backlight_brightness;
    }
#else
    current_power_usage = 5; // 当前用电量
    anti_jump_change = 5; // 防止电压跳变
#endif
    return current_power_usage;
}

int16_t calculate_current_power_usage(uint8_t charging_state){
    // 根据充电状态返回电量变化值
    if (charging_state) {
        // 充电状态
        if (batteryState.percentageRemaining <= 5) {
            // 涓流充电
            return trickle_charge;
        } else if (batteryState.percentageRemaining < 100) {
            // 恒流充电
            return constant_current;
        } else {
            // 恒压充电
            return constant_voltage;
        }
    } else {
        // 放电状态
        return (-calculate_power_consumption());
    }
}

// 低电检测
void check_low_battery(void){
    if (batteryState.percentageRemaining > LOW_BATTERY_SHUTDOWN_PCT)
    {
        // 关闭低电闪烁
//        led_low_bat_stop();
        printf("Turn off low flash \r\n");
    } else if (batteryState.percentageRemaining <= LOW_BATTERY_WARN_PCT)
    {
        // 低电闪烁
//        led_low_bat_start();
        printf("Low electric flicker\r\n");
    } else  if (batteryState.isCharging == 0 && batteryState.percentageRemaining <= LOW_BATTERY_SHUTDOWN_PCT)
    {
        // 低电关机
//        led_low_bat_start();
//        _set_evt(SYS_ST, SYS_LOW_POWER);
        //    u_timerout_enable(U_TIMER5,power_down_cb,2000,1);
        printf("Low power shutdown \r\n");
    }
}

void battery_init(void){
    // 初始化ADC - 多次采样平均模式，不启用中断
//    gpadc_init(AVERAGE_MODE, GPIO29_CHANNEL, 0, 0);

    // 获取电池电压并存入batteryState
    batteryState.voltageMv = get_battery_voltage();
    printf("battery_voltage = %d \r\n", batteryState.voltageMv);

    if (batteryState.voltageMv != 0xffff) {
        // 使用convert_voltage_to_percentage方法计算电量百分比
        uint8_t currentLevel = convert_voltage_to_percentage(batteryState.voltageMv);
        printf(" level = %d\r\n", currentLevel);

        // 计算历史电量百分比
        uint8_t historicalPercentage = (uint8_t)((KeyboardConfig.remaining_capacity_mas * 100) / BATTERY_CAPACITY_MAS);

        // 如果计算的电量与电压表对应的电量差异过大，或电量异常，则重置电量
        if (GET_DIFFERENCE(historicalPercentage, currentLevel) > 10 ||
            KeyboardConfig.remaining_capacity_mas == 0 ||
            KeyboardConfig.remaining_capacity_mas > BATTERY_CAPACITY_MAS) {
            // 如果电量异常或差异过大，重置电量状态
            printf("Reset battery level due to large difference or invalid value\r\n");
            printf("Historical: %d%%, Current: %d%%\r\n", historicalPercentage, currentLevel);

            batteryState.remainingCapacityMas = (uint32_t)((uint64_t)BATTERY_CAPACITY_MAS * currentLevel / 100);
            batteryState.percentageRemaining = (uint8_t)currentLevel;

            uint16_t  a = calculate_voltage(batteryState.remainingCapacityMas);
            printf("a = %d \r\n",a);

            // 使用当前电压对应的电量百分比重新初始化电量状态
            KeyboardConfig.remaining_capacity_mas = batteryState.remainingCapacityMas;
        } else {
            // 使用历史电量
            batteryState.remainingCapacityMas = KeyboardConfig.remaining_capacity_mas;
            batteryState.percentageRemaining = (uint8_t)historicalPercentage;
        }
    }
}
void battery_reset(void) {
    batteryState.voltageMv = get_battery_voltage();
    uint8_t currentLevel = convert_voltage_to_percentage(batteryState.voltageMv);
    batteryState.remainingCapacityMas = (uint32_t)((uint64_t)BATTERY_CAPACITY_MAS * currentLevel / 100);
    batteryState.percentageRemaining = (uint8_t)currentLevel;
    KeyboardConfig.remaining_capacity_mas = batteryState.remainingCapacityMas;
    #ifdef BATTERY_LEVEL_IDX
        if(ble_is_connected() && ble_check_notify_enabled()) {
            uint8_t newPercentageLevel = get_batter_level();
            ble_tx_data(BATTERY_LEVEL_IDX, 1, &newPercentageLevel);
        }
    #endif
}


void battery_level_detection(uint16_t threshold){

    static uint16_t bi_timer = 0;

    bi_timer++;

    if (bi_timer < threshold)
        return;

    bi_timer = 0;

    batteryState.voltageMv = get_battery_voltage();

    // 1. 判断充电状态
    uint8_t chargingState = calculate_charging_state();
    printf("chargingState:%d\r\n",chargingState);
    batteryState.isCharging = chargingState;
    // 2. 计算电量变化
    int16_t powerUsage = calculate_current_power_usage(chargingState);

    // 3. 更新电量
    batteryState.remainingCapacityMas += powerUsage;

    // 确保电量在有效范围内
    if(batteryState.remainingCapacityMas > BATTERY_CAPACITY_MAS) {
        batteryState.remainingCapacityMas = BATTERY_CAPACITY_MAS;
    } else if(batteryState.remainingCapacityMas < 0) {
        batteryState.remainingCapacityMas = 0;
    }
    printf("batteryState.remainingCapacityMas:%d\r\n",batteryState.remainingCapacityMas);
    // 计算新的电量百分比 电量百分比 = 剩余容量 / 总容量 * 100 需要四舍五入
    uint8_t newPercentageLevel = calculate_capacity_percentage(batteryState.remainingCapacityMas);

    printf("newPercentageLevel:%d, batteryState.percentageRemaining:%d\r\n",newPercentageLevel,batteryState.percentageRemaining);
    printf("batteryState.voltageMv:%d\r\n",batteryState.voltageMv);
    if (chargingState == 0 ){
        printf("==========");
        //    卡尔曼滤波
        float voltageBasedPercentage = (float)convert_voltage_to_percentage(batteryState.voltageMv);

        // 使用卡尔曼滤波器融合两种测量
        uint8_t filteredPercentage = (uint8_t)(kalman_filter_update(voltageBasedPercentage, (float )newPercentageLevel)+ 0.5f);

        printf("voltageBasedPercentage=%d, newPercentageLevel=%d, filteredPercentage=%d\r\n", (int)voltageBasedPercentage, newPercentageLevel, filteredPercentage);

        if (filteredPercentage != newPercentageLevel) {
            // 更新电池状态
            batteryState.remainingCapacityMas = (uint32_t)((filteredPercentage * BATTERY_CAPACITY_MAS) / 100);
            // 只能降低，不能升高，除非处于充电中
            if (filteredPercentage < newPercentageLevel){
                newPercentageLevel = filteredPercentage;
            }
        }
    }

    // 如果电量百分比发生变化
    if(newPercentageLevel != batteryState.percentageRemaining) {
        batteryState.percentageRemaining = newPercentageLevel;
        KeyboardConfig.remaining_capacity_mas = batteryState.remainingCapacityMas; // 存储剩余容量
        printf("Battery level: %d%%\r\n", batteryState.percentageRemaining);
        // 发送蓝牙通知
#ifdef BATTERY_LEVEL_IDX
        if(ble_is_connected() && ble_check_notify_enabled()) {
            uint8_t newPercentageLevel = get_batter_level();
            ble_tx_data(BATTERY_LEVEL_IDX, 1, &newPercentageLevel);
        }
#endif
        // 4. 检查低电量状态
        check_low_battery();
    }
}




uint8_t Bat_num(void)
{
    uint8_t Num = 0;
    if(batteryState.percentageRemaining >= 75){
        Num = 4;
    }else if (75 > batteryState.percentageRemaining && batteryState.percentageRemaining >= 50){
        Num = 3;
    }else if (50 > batteryState.percentageRemaining && batteryState.percentageRemaining >= 25){
        Num = 2;
    }else{
        Num = 1;
    }
    return Num;
}

void test_battery(void){
    // uint16_t voltage_buf[FILTER_N];
    // for(uint8_t i = 0; i < FILTER_N; i++){
    //     voltage_buf[i] = gpadc_get_value() - i*5;
    //     printf("voltage_buf[%d] = %d\r\n", i, voltage_buf[i]);
    // }
    // float slope = calculate_voltage_slope(voltage_buf, FILTER_N);
    // printf("slope = %f\r\n", slope);
    battery_init();
}
