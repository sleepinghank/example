/**
 * @file timer_task.c
 * @brief 生产级动态定时器任务管理系统实现
 * @author hank
 * @date 2024/9/12
 *
 * 生产级特性：
 * - 全面的安全性检查和错误处理
 * - 任务优先级和立即执行机制
 * - 系统监控和性能统计
 * - 简化的动态调整算法
 */

#include "timer_task.h"
#include <string.h>

// 全局变量
TaskComps_t task_list[TASK_NUM_MAX];
static TimerTask_t timer_task = {0};

// ==================== 安全性函数 ====================

/// @brief 安全的时间间隔验证
/// @param time_ms 时间间隔(ms)
/// @return 1:有效 0:无效
static uint8_t validate_time_interval(uint32_t time_ms) {
    return (time_ms >= TIMER_INTERVAL_MIN_MS && time_ms <= TIMER_INTERVAL_MAX_MS);
}

/// @brief 安全的除法操作，防止除零
/// @param dividend 被除数
/// @param divisor 除数
/// @return 安全的除法结果，出错时返回UINT32_MAX
static uint32_t safe_division(uint32_t dividend, uint32_t divisor) {
    if (divisor == 0) {
        return UINT32_MAX;
    }
    return dividend / divisor;
}

/// @brief 快速GCD计算（使用欧几里得算法）
/// @param a 第一个数
/// @param b 第二个数
/// @return 最大公约数
static uint32_t fast_gcd(uint32_t a, uint32_t b) {
    while (b != 0) {
        uint32_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}
#define TIMER(task_time,task_do) {.run = 0, .long_flag = 0, .time_count = 0, .do_cnt = 0x80, .task_time = (task_time), .task_cb = (task_do)};
#define TIMER_OUT(task_time,task_do,do_cnt) {.run = 0, .long_flag = 0, .time_count = 0, .do_cnt = do_cnt, .task_time = (task_time), .task_cb = (task_do)};

// 示例任务函数
void key_board_task(void){
    // 键盘扫描任务 (5ms间隔)
}

void tp_task(void){
    // 触摸扫描任务 (8ms间隔)
}

void idle_task(void){
    // Idle检测任务 (2s间隔，用于动态调整定时器)
}

void system_maintenance_task(void){
    // 系统维护任务 (1s间隔)
}

enum timer_id {
    KB_TIMER_ID = 0,     // 键盘扫描任务
    TP_TIMER_ID,          // 触摸扫描任务
    IDLE_TIMER_ID,        // Idle检测任务
    SYS_MAINTENANCE_ID,   // 系统维护任务
    sys_timer_end,
} ;
TaskComps_t task_list[TASK_NUM_MAX];
// TaskComps_t task_list[TASK_NUM_MAX] = {
//     [KB_TIMER_ID] =  TIMER(5,key_board_task),
//     [TP_TIMER_ID] =  TIMER(5,tp_task),
// };

static TimerTask_t timer_task;
static uint16_t timer_task_en = 0;//每一位对应一个任务使能
void modify_task(TimeTaskCb task_do, uint32_t task_time, uint8_t do_cnt, uint16_t i);

// 更改定时器间隔
void change_timer_interval(void){
    uint64_t timer_cnt = ((uint64_t)timer_task.timer_interval * 32768 /1000 - 1 + 0.5);
}

uint8_t task_init(void) {
    if (timer_task.initialized) {
        return TIMER_OK;  // 已经初始化
    }

    // 清空任务列表
    memset(task_list, 0, sizeof(task_list));
    memset(&timer_task, 0, sizeof(timer_task));

    // 初始化默认值
    timer_task.timer_interval = TIMER_INTERVAL_DEFAULT_MS;
    timer_task.interval_gcd = TIMER_INTERVAL_DEFAULT_MS;
    timer_task.reset_flag = 0;
    timer_task.initialized = 1;

    return TIMER_OK;
}

/// @brief 初始化示例任务 - 展示如何使用定时器系统
uint8_t task_init_example(void) {
    // 初始化基础定时器系统
    uint8_t result = task_init();
    if (result != TIMER_OK) {
        return result;
    }

    // 创建示例任务
    // 键盘扫描任务：每5ms执行一次 (高频任务)
    u_timer(key_board_task, 5);

    // 触摸扫描任务：每8ms执行一次 (高频任务)
    u_timer(tp_task, 8);

    // 系统维护任务：每1s执行一次 (低频任务)
    u_timer(system_maintenance_task, 1000);

    // 注意：Idle任务应该在外部条件满足时动态创建和删除
    // 例如：当检测到系统进入空闲状态时
    // u_timer(idle_task, 2000);

    // 此时系统会自动计算最优定时器间隔：
    // GCD(5, 8, 1000) = 1ms < 4ms => 使用最小间隔4ms

    return TIMER_OK;
}

void task_handler(void){
    uint8_t i = 0;

    // 执行非立即执行的任务（在主循环中执行）
    for (i = 0; i < TASK_NUM_MAX; ++i) {
        if(task_list[i].enabled && task_list[i].run > 0 && !task_list[i].immediate){
            task_list[i].task_cb();
            task_list[i].run = 0;

            // 处理一次性任务
            if (task_list[i].do_cnt == 0){
                u_timer_disable(i);
            }
        }
    }
}

void task_schedule(void){
    uint8_t i = 0;

    // 处理所有启用的任务
    for (i = 0; i < TASK_NUM_MAX; ++i) {
        if (task_list[i].enabled) {
            if (task_list[i].time_count == 0) {
                uint32_t time_reload = safe_division(task_list[i].task_time, timer_task.timer_interval);
                if (time_reload == UINT32_MAX || task_list[i].do_cnt == 0){
                    continue;
                }

                if (task_list[i].do_cnt & 0x80) {
                    // 无限重复执行
                    task_list[i].run = 1;
                    task_list[i].time_count = time_reload;

                    // 立即执行高优先级任务
                    if (task_list[i].immediate) {
                        task_list[i].task_cb();
                        task_list[i].run = 0;  // 清除运行标志
                    }
                } else if (task_list[i].do_cnt > 0) {
                    // 限定执行次数
                    task_list[i].run = 1;
                    task_list[i].time_count = time_reload;
                    task_list[i].do_cnt--;

                    // 立即执行高优先级任务
                    if (task_list[i].immediate) {
                        task_list[i].task_cb();
                        task_list[i].run = 0;  // 清除运行标志
                    }
                }
            } else {
                task_list[i].time_count--;
            }
        }
    }
    // 需要更改定时器间隔，在此位置更改才能保证间隔时间绝对准确
    if (timer_task.reset_flag == 1){
        uint8_t has_runing_task = 0;
        for (i = 0; i < TASK_NUM_MAX; ++i) {
            // 任务开启并且
            if (((timer_task_en >> i ) & 0x01) && (task_list[i].long_flag != 1) ) {
                if (task_list[i].run == 0){
                    has_runing_task = 1;
                }
            }
        }
        if (has_runing_task == 0){
            // 重新计算更改定时器
            for (i = 0; i < TASK_NUM_MAX; ++i) {
                // 计算任务已定时时间
                uint32_t time_has_passed = task_list[i].time_count * timer_task.timer_interval;
                // 计算任务剩余时间
                uint32_t time_left = task_list[i].task_time - time_has_passed;
                // 计算任务剩余时间所需定时器周期
                uint32_t time_reload = (time_left / timer_task.timer_interval);
                task_list[i].time_count = time_reload;
            }
            timer_task.timer_interval = timer_task.interval_gcd;
            timer_task.reset_flag = 0;
            // TODO: 重新设置定时器
            change_timer_interval();
        }
    }
}

/// @brief 计算最优定时器间隔
/// @return 返回计算得到的定时器间隔(ms)，应用最小间隔限制
static uint32_t calculate_optimal_interval(void) {
    uint32_t min_interval = UINT32_MAX;
    uint32_t gcd_val = 0;
    uint8_t active_tasks = 0;
    uint8_t i = 0;

    // 遍历所有活跃任务，找到最小时间间隔和GCD
    for (i = 0; i < TASK_NUM_MAX; ++i) {
        if ((timer_task_en >> i) & 0x01) {
            active_tasks++;

            // 更新最小间隔
            if (task_list[i].task_time < min_interval) {
                min_interval = task_list[i].task_time;
            }

            // 计算GCD
            if (gcd_val == 0) {
                gcd_val = task_list[i].task_time;
            } else {
                gcd_val = gcd(gcd_val, task_list[i].task_time);
            }
        }
    }

    // 如果没有活跃任务，返回最大间隔
    if (active_tasks == 0) {
        return 1000; // 默认1s间隔
    }

    // 如果只有一个任务，直接使用其间隔
    if (active_tasks == 1) {
        return (min_interval < TIMER_INTERVAL_MIN_MS) ? TIMER_INTERVAL_MIN_MS : min_interval;
    }

    // 多个任务：优先使用GCD，但要满足最小间隔要求
    if (gcd_val >= TIMER_INTERVAL_MIN_MS) {
        return gcd_val;
    } else {
        // 如果GCD太小，使用最小间隔
        return TIMER_INTERVAL_MIN_MS;
    }
}

// ==================== 简化的动态调整逻辑 ====================

/// @brief 简化的最优间隔计算 - 使用最小间隔策略
/// @return 计算得到的最优定时器间隔(ms)
static uint32_t calculate_simple_optimal_interval(void) {
    uint32_t min_interval = UINT32_MAX;
    uint32_t gcd_val = 0;
    uint8_t active_tasks = 0;
    uint8_t i = 0;

    // 找到所有活跃任务的最小时间间隔
    for (i = 0; i < TASK_NUM_MAX; ++i) {
        if (task_list[i].enabled) {
            active_tasks++;

            // 更新最小间隔
            if (task_list[i].task_time < min_interval) {
                min_interval = task_list[i].task_time;
            }

            // 计算GCD
            if (gcd_val == 0) {
                gcd_val = task_list[i].task_time;
            } else {
                gcd_val = fast_gcd(gcd_val, task_list[i].task_time);
            }
        }
    }

    // 没有活跃任务：使用默认间隔
    if (active_tasks == 0) {
        return TIMER_INTERVAL_DEFAULT_MS;
    }

    // 单个任务：使用任务间隔（不小于最小值）
    if (active_tasks == 1) {
        return (min_interval < TIMER_INTERVAL_MIN_MS) ? TIMER_INTERVAL_MIN_MS : min_interval;
    }

    // 多个任务：使用GCD但不小于最小间隔
    return (gcd_val < TIMER_INTERVAL_MIN_MS) ? TIMER_INTERVAL_MIN_MS : gcd_val;
}

/// @brief 简化的定时器间隔计算
void calculate_timer_interval(void) {
    uint32_t new_interval = calculate_simple_optimal_interval();

    // 需要更新定时器间隔
    if (new_interval != timer_task.timer_interval) {
        timer_task.interval_gcd = new_interval;
        timer_task.reset_flag = 1;
    }
}

uint8_t u_timer_priority(TimeTaskCb task_do, uint32_t task_time, uint8_t priority, uint8_t immediate) {
    uint8_t i = 0;

    // 参数验证
    if (!task_do || !validate_time_interval(task_time)) {
        return 0xFF;
    }

    // 找到空闲位置
    for (i = 0; i < TASK_NUM_MAX; ++i) {
        if (!task_list[i].enabled) {
            // 配置任务
            task_list[i].run = 0;
            task_list[i].long_flag = (task_time >= 2000) ? 1 : 0;
            task_list[i].immediate = immediate;
            task_list[i].enabled = 1;
            task_list[i].priority = priority;
            task_list[i].time_count = safe_division(task_time, timer_task.timer_interval);
            task_list[i].do_cnt = 0x80;  // 无限重复
            task_list[i].task_time = task_time;
            task_list[i].task_cb = task_do;

            calculate_timer_interval();
            return i;
        }
    }

    return 0xFF;  // 没有空闲位置
}

uint8_t u_timer(TimeTaskCb task_do, uint32_t task_time) {
    return u_timer_priority(task_do, task_time, TASK_PRIORITY_NORMAL, 0);
}

uint8_t u_timer_out(TimeTaskCb task_do, uint32_t task_time, uint8_t do_cnt)
{
    uint8_t i = 0;

    // 参数验证
    if (!task_do || !validate_time_interval(task_time)) {
        return 0xFF;
    }

    // 找到空闲位置
    for (i = 0; i < TASK_NUM_MAX; ++i) {
        if (!task_list[i].enabled) {
            // 配置任务
            task_list[i].run = 0;
            task_list[i].long_flag = (task_time >= 2000) ? 1 : 0;
            task_list[i].immediate = 0;
            task_list[i].enabled = 1;
            task_list[i].priority = TASK_PRIORITY_NORMAL;
            task_list[i].time_count = safe_division(task_time, timer_task.timer_interval);
            task_list[i].do_cnt = do_cnt;  // 限定执行次数
            task_list[i].task_time = task_time;
            task_list[i].task_cb = task_do;

            calculate_timer_interval();
            return i;
        }
    }

    return 0xFF;  // 没有空闲位置
}

void u_timer_out_modify(uint16_t task_id, TimeTaskCb task_do ,uint32_t task_time, uint8_t do_cnt){
    if (task_id >= TASK_NUM_MAX || !task_list[task_id].enabled || !validate_time_interval(task_time)){
        return;
    }

    task_list[task_id].task_cb = task_do;
    task_list[task_id].task_time = task_time;
    task_list[task_id].do_cnt = do_cnt;
    task_list[task_id].time_count = safe_division(task_time, timer_task.timer_interval);
    task_list[task_id].run = 0;

    calculate_timer_interval();
}

void u_timer_modify(uint16_t task_id, TimeTaskCb task_do ,uint32_t task_time){
    u_timer_out_modify(task_id, task_do, task_time, 0x80);
}

void u_timer_reset(uint16_t task_id){
    if (task_id >= TASK_NUM_MAX){
        return;
    }
    {
        uint32_t time_reload = (task_list[task_id].task_time / timer_task.timer_interval);
        task_list[task_id].time_count = time_reload;
    }
}


void u_timer_disable(uint16_t task_id){
    if (task_id >= TASK_NUM_MAX){
        return;
    }
    timer_task_en &= ~(BIT(task_id));
    task_list[task_id].run = 0;
    task_list[task_id].time_count = 0;
    calculate_timer_interval();
}

void u_timer_enable(uint16_t task_id){
    if (task_id >= TASK_NUM_MAX || task_list[task_id].do_cnt == 0){
        return;
    }

    uint32_t time_reload = (task_list[task_id].task_time / timer_task.timer_interval);
    task_list[task_id].time_count = time_reload;
    task_list[task_id].run = 0;
    timer_task_en |= BIT(task_id);
    calculate_timer_interval();
}

void u_timer_clear(uint16_t task_id){
    task_list[task_id].do_cnt = 0;
    u_timer_disable(task_id);
}

uint8_t u_timer_check_enabled(uint16_t id){
    if (id >= TASK_NUM_MAX){
        return 0;
    }
    return (timer_task_en >> id) & 0x01;
}
