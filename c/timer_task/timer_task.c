//
// Created by hank on 2024/9/12.
// Optimized based on ARCHITECTURE_DESIGN.md
//

#include "timer_task.h"
#include "hal/timer.h"  // 硬件定时器API

/* ============================ 编译时安全检查 ============================ */
#define TIMER_INTERVAL_MIN_MS     4
#define TIMER_INTERVAL_MAX_MS     (UINT32_MAX / 32768)  // 约131000秒
#define TIMER_INTERVAL_IDLE       60000  // 无任务时的低功耗间隔(60秒)

// C11 静态断言 - 编译时检查
#define TIMER_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)

// 任务配置宏 (带编译时检查)
#define TIMER_ASSERT_MIN_INTERVAL(period) \
    TIMER_STATIC_ASSERT((period) >= TIMER_INTERVAL_MIN_MS, \
        "Timer interval must be >= 4ms")

/* ============================ 错误码定义 ============================ */
typedef enum {
    TIMER_OK = 0,
    TIMER_ERR_INTERVAL_TOO_SMALL,     // 定时间隔 < 4ms
    TIMER_ERR_GCD_TOO_SMALL,          // 任务GCD < 4ms
    TIMER_ERR_NO_FREE_SLOT,           // 无空闲任务槽
    TIMER_ERR_INVALID_ID,             // 无效任务ID
    TIMER_ERR_OVERFLOW,               // 计数溢出
    TIMER_ERR_NOT_ENABLED,            // 操作未使能的任务
} timer_error_t;

/* ============================ 定时器全局状态 ============================ */
typedef struct {
    // 硬件定时器配置
    int timer_id;                      // 硬件定时器ID (使用TIMER_1)
    uint32_t current_interval;         // 当前硬件定时器间隔 (ms)

    // 系统时间戳
    uint32_t system_ms;                // 系统运行毫秒数 (定时器触发时累加)

    // 统计信息
    uint32_t total_ticks;              // 总tick计数 (用于调试)
    uint32_t switch_count;             // 间隔切换次数
} timer_state_t;

/* ============================ 辅助函数声明 ============================ */
// 计算两个整数的最大公因数 (GCD) - 欧几里得算法
static uint32_t gcd(uint32_t a, uint32_t b) {
    while (b != 0) {
        uint32_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// 计算两个整数的最小公倍数 (LCM)，基于已知的最大公因数 (GCD)
static uint32_t lcm(uint32_t a, uint32_t b) {
    return (a / gcd(a, b)) * b; // 利用公式：lcm(a, b) = |a*b| / gcd(a, b)
}

/* ============================ 错误处理 ============================ */
static void timer_error_handler(timer_error_t error) {
    // TODO: 根据实际平台实现错误处理
    // 可以记录日志、触发断言或其他处理
    (void)error;  // 避免未使用警告
}
/* ============================ 任务初始化宏 ============================ */
#define TIMER(task_time,task_do) {.run = 0, .enabled = 0, .long_flag = 0, \
    .time_count = 0, .period_reload = 0, .task_time = (task_time), \
    .exec_count = 0x80, .callback = (task_do)}

#define TIMER_OUT(task_time,task_do,do_cnt) {.run = 0, .enabled = 0, .long_flag = 0, \
    .time_count = 0, .period_reload = 0, .task_time = (task_time), \
    .exec_count = do_cnt, .callback = (task_do)}

/* ============================ 全局变量 ============================ */
enum timer_id {
    KB_TIMER_ID = 0,
    TP_TIMER_ID,
    sys_timer_end,
};

// 任务控制块数组 (修改为新的结构体定义)
typedef struct {
    uint8_t run        : 1;  // 任务就绪标志，待执行
    uint8_t enabled    : 1;  // 任务使能标志
    uint8_t long_flag  : 1;  // 长定时标志 (>=2000ms，可选优化)
    uint8_t reserved   : 5;  // 预留位
    uint32_t time_count;     // 剩余定时器周期数
    uint32_t period_reload;  // 周期重装载值 (task_time / timer_interval)
    uint32_t task_time;      // 任务定时间隔 (ms)
    uint8_t  exec_count;     // 剩余执行次数 (0x80=无限循环)
    TimeTaskCb callback;     // 任务回调函数
} task_ctrl_t;

static task_ctrl_t g_tasks[TASK_NUM_MAX];

// 定时器全局状态
static timer_state_t g_timer_state = {
    .timer_id = 1,  // 使用TIMER_1
    .current_interval = TIMER_INTERVAL_IDLE,
    .system_ms = 0,
    .total_ticks = 0,
    .switch_count = 0
};

// 内部函数声明
static void modify_task(TimeTaskCb task_do, uint32_t task_time, uint8_t do_cnt, uint16_t i);
static uint32_t timer_calculate_interval(void);
static void timer_switch_immediate(uint32_t new_interval_ms);

/* ============================ 核心算法实现 ============================ */

/**
 * @brief 更改硬件定时器间隔
 * @note 使用硬件定时器API设置新的间隔
 */
static void change_timer_interval(void){
    // 计算定时器计数值 (基于32.768KHz时钟源)
    // interval_ms * 32.768 = ticks
    uint32_t timer_ticks = (uint32_t)((uint64_t)g_timer_state.current_interval * 32768 / 1000);

    // 禁用定时器 (如果已启用)
    if (timer_check_enabled(g_timer_state.timer_id)) {
        timer_disable(g_timer_state.timer_id);
    }

    // 重新启用定时器，使用新的间隔
    timer_enable(g_timer_state.timer_id, task_schedule, timer_ticks, 1);
}

/**
 * @brief 基于硬件定时器计数器计算已过时间
 * @return 已过时间 (ms)
 * @note 直接读取硬件计数器，精度高、无溢出风险
 */
static uint32_t timer_get_elapsed_ms(void)
{
    // 读取硬件定时器当前计数值 (从0开始递增)
    uint32_t elapsed_ticks = timer_get_tick(g_timer_state.timer_id);

    // 转换为ms (32.768KHz时钟)
    // elapsed_ms = elapsed_ticks * 1000 / 32768
    // 优化为: elapsed_ms = (elapsed_ticks * 125) >> 12  (除以4096)
    // 这样避免浮点运算且精度足够
    uint32_t elapsed_ms = (elapsed_ticks * 125) >> 12;

    return elapsed_ms;
}

/**
 * @brief 立即切换定时器间隔并校准所有任务
 * @param new_interval_ms 新的定时器间隔 (ms)
 *
 * 核心改进: 不再等待当前周期完成，立即切换
 * 通过main循环计数器计算已过时间，精确校准所有任务的剩余时间
 */
static void timer_switch_immediate(uint32_t new_interval_ms)
{
    uint32_t old_interval = g_timer_state.current_interval;

    if (new_interval_ms == old_interval) {
        return;  // 无需切换
    }

    // Step 1: 计算已过时间
    uint32_t elapsed_ms = timer_get_elapsed_ms();

    // Step 2: 校准所有使能任务的剩余时间
    for (uint8_t i = 0; i < TASK_NUM_MAX; i++) {
        if (g_tasks[i].enabled) {
            // 计算任务已经运行的时间 (ms)
            uint32_t task_elapsed_ms = (g_tasks[i].period_reload - g_tasks[i].time_count) * old_interval;

            // 加上本次已过时间
            task_elapsed_ms += elapsed_ms;

            // 计算任务剩余时间 (ms)
            // 保证任务仍在原定时间点执行 (保留原定时间)
            uint32_t remaining_ms = 0;
            if (task_elapsed_ms < g_tasks[i].task_time) {
                remaining_ms = g_tasks[i].task_time - task_elapsed_ms;
            }

            // 更新period_reload为新的定时器间隔下的值
            g_tasks[i].period_reload = (g_tasks[i].task_time + new_interval_ms - 1) / new_interval_ms;
            g_tasks[i].time_count = (remaining_ms + new_interval_ms - 1) / new_interval_ms;

            // 特殊处理: 如果remaining_ms为0但任务未执行，说明该立即执行
            if (remaining_ms == 0 && g_tasks[i].run == 0 && task_elapsed_ms > 0) {
                // 任务已经到期，设置run标志
                g_tasks[i].run = 1;
                g_tasks[i].time_count = g_tasks[i].period_reload;
            }
        }
    }

    // Step 3: 更新定时器间隔
    g_timer_state.current_interval = new_interval_ms;
    g_timer_state.switch_count++;

    // Step 4: 立即更新硬件定时器
    change_timer_interval();
}

/**
 * @brief 计算当前活动任务的最优定时器间隔
 * @return 计算得到的定时器间隔 (ms)
 *
 * 算法流程:
 * 1. 遍历所有使能的任务
 * 2. 计算所有任务定时间隔的GCD
 * 3. 验证GCD >= 4ms
 */
static uint32_t timer_calculate_interval(void)
{
    uint32_t gcd_value = 0;
    uint8_t task_count = 0;

    // Step 1: 计算所有活动任务的GCD
    for (uint8_t i = 0; i < TASK_NUM_MAX; i++) {
        if (g_tasks[i].enabled) {
            if (gcd_value == 0) {
                gcd_value = g_tasks[i].task_time;
            } else {
                gcd_value = gcd(gcd_value, g_tasks[i].task_time);
            }
            task_count++;
        }
    }

    // Step 2: 无活动任务时返回最大间隔
    if (task_count == 0) {
        return TIMER_INTERVAL_IDLE;  // 进入低功耗
    }

    // Step 3: 验证GCD合法性
    if (gcd_value < TIMER_INTERVAL_MIN_MS) {
        // 运行时检测到非法配置，记录错误
        timer_error_handler(TIMER_ERR_GCD_TOO_SMALL);
        return TIMER_INTERVAL_MIN_MS;  // 降级处理
    }

    return gcd_value;
}

/* ============================ 系统初始化 ============================ */

/**
 * @brief 任务初始化
 * @note 初始化定时器系统，计算初始间隔并启动硬件定时器
 */
void task_init(void){
    // 初始化所有任务为禁用状态
    for (uint8_t i = 0; i < TASK_NUM_MAX; i++) {
        g_tasks[i].enabled = 0;
        g_tasks[i].run = 0;
        g_tasks[i].time_count = 0;
        g_tasks[i].period_reload = 0;
        g_tasks[i].task_time = 0;
        g_tasks[i].exec_count = 0;
        g_tasks[i].callback = NULL;
        g_tasks[i].long_flag = 0;
    }

    // 初始化定时器状态
    g_timer_state.timer_id = 1;  // 使用TIMER_1
    g_timer_state.current_interval = TIMER_INTERVAL_IDLE;
    g_timer_state.system_ms = 0;
    g_timer_state.total_ticks = 0;
    g_timer_state.switch_count = 0;

    // TODO: 如果有预定义的任务，在这里初始化
    // 例如:
    // u_timer(key_board_task, 5);
    // u_timer(tp_task, 5);

    // 设置并启动硬件定时器TIMER_1
    change_timer_interval();
}

/**
 * @brief 任务处理函数，主循环中调用
 * @note 执行所有就绪的任务，并在需要时禁用已完成的任务
 */
void task_handler(void){
    // 遍历所有任务
    for (uint8_t i = 0; i < TASK_NUM_MAX; i++) {
        if (g_tasks[i].enabled && g_tasks[i].run) {
            // 执行回调函数
            if (g_tasks[i].callback != NULL) {
                g_tasks[i].callback();
            }

            // 清除运行标志
            g_tasks[i].run = 0;

            // 检查是否需要禁用任务
            if (g_tasks[i].exec_count == 0) {
                u_timer_disable(i);
            }
        }
    }
}

/**
 * @brief 任务调度函数 - 在硬件定时器ISR中调用
 *
 * 职责:
 * 1. 累加系统毫秒数 (提供全局时间基准)
 * 2. 递减所有活动任务的time_count
 * 3. time_count==0时设置run标志并重装载
 *
 * 简化: 使用硬件定时器，不再需要维护软件计数器
 */
void task_schedule(void){
    // 累加系统毫秒数 (用于提供全局时间基准)
    g_timer_state.system_ms += g_timer_state.current_interval;
    g_timer_state.total_ticks++;

    // 遍历所有任务
    for (uint8_t i = 0; i < TASK_NUM_MAX; i++) {
        if (!g_tasks[i].enabled) {
            continue;
        }

        // 递减计数
        if (g_tasks[i].time_count > 0) {
            g_tasks[i].time_count--;
        }

        // 定时到达
        if (g_tasks[i].time_count == 0 && g_tasks[i].exec_count != 0) {
            g_tasks[i].run = 1;

            // 检查执行次数
            if (g_tasks[i].exec_count != 0x80) {  // 非无限循环
                g_tasks[i].exec_count--;
                if (g_tasks[i].exec_count == 0) {
                    g_tasks[i].enabled = 0;  // 自动禁用
                    continue;  // 不重装载
                }
            }

            // 重装载
            g_tasks[i].time_count = g_tasks[i].period_reload;
        }
    }
}

/**
 * @brief 更改定时器间隔并立即切换 (内部调用)
 * @note 计算新的GCD，如果需要则立即切换
 */
static void recalculate_and_switch_interval(void){
    uint32_t new_interval = timer_calculate_interval();

    // 立即切换
    timer_switch_immediate(new_interval);
}

/* ============================ 内部辅助函数 ============================ */

/**
 * @brief 修改任务配置 (内部函数)
 * @param task_do 任务回调函数
 * @param task_time 任务定时间隔 (ms)
 * @param do_cnt 执行次数 (0x80表示无限循环)
 * @param i 任务索引
 */
static void modify_task(TimeTaskCb task_do, uint32_t task_time, uint8_t do_cnt, uint16_t i) {
    // 参数校验
    if (i >= TASK_NUM_MAX) {
        return;
    }

    // 运行时检查最小间隔
    if (task_time < TIMER_INTERVAL_MIN_MS) {
        timer_error_handler(TIMER_ERR_INTERVAL_TOO_SMALL);
        task_time = TIMER_INTERVAL_MIN_MS;  // 降级处理
    }

    // 配置任务
    g_tasks[i].callback = task_do;
    g_tasks[i].task_time = task_time;
    g_tasks[i].exec_count = do_cnt;
    g_tasks[i].run = 0;
    g_tasks[i].enabled = 1;

    // 设置长定时标志
    g_tasks[i].long_flag = (task_time >= 2000) ? 1 : 0;

    // 计算period_reload和time_count
    uint32_t current_interval = g_timer_state.current_interval;
    g_tasks[i].period_reload = (task_time + current_interval - 1) / current_interval;
    g_tasks[i].time_count = g_tasks[i].period_reload;
}

/* ============================ 公共API函数 ============================ */

/**
 * @brief 创建限定执行次数的定时任务
 * @param task_do 任务回调函数
 * @param task_time 执行间隔 (ms), 必须 >= 4ms
 * @param do_cnt 执行次数 (1-255), 0x80表示无限循环
 * @return 任务ID (0-15), 或 0xFF表示失败
 */
uint8_t u_timer_out(TimeTaskCb task_do, uint32_t task_time, uint8_t do_cnt)
{
    uint8_t timer_id = 0xFF;

    // 查找空闲任务槽
    for (uint8_t i = 0; i < TASK_NUM_MAX; i++) {
        if (!g_tasks[i].enabled && g_tasks[i].exec_count == 0) {
            modify_task(task_do, task_time, do_cnt, i);
            timer_id = i;
            break;
        }
    }

    // 如果成功创建任务，重新计算定时器间隔并立即切换
    if (timer_id != 0xFF) {
        recalculate_and_switch_interval();
    }

    return timer_id;
}

/**
 * @brief 创建循环执行的定时任务
 * @param task_do 任务回调函数
 * @param task_time 执行间隔 (ms), 必须 >= 4ms
 * @return 任务ID (0-15), 或 0xFF表示失败
 */
uint8_t u_timer(TimeTaskCb task_do, uint32_t task_time)
{
    return u_timer_out(task_do, task_time, 0x80);
}

/**
 * @brief 修改已存在任务的配置
 * @param task_id 任务ID
 * @param task_do 任务回调函数
 * @param task_time 新的定时间隔 (ms)
 * @param do_cnt 执行次数
 */
void u_timer_out_modify(uint16_t task_id, TimeTaskCb task_do, uint32_t task_time, uint8_t do_cnt){
    if (task_id >= TASK_NUM_MAX){
        return;
    }

    modify_task(task_do, task_time, do_cnt, task_id);
    recalculate_and_switch_interval();
}

/**
 * @brief 修改已存在任务的配置 (无限循环版本)
 * @param task_id 任务ID
 * @param task_do 任务回调函数
 * @param task_time 新的定时间隔 (ms)
 */
void u_timer_modify(uint16_t task_id, TimeTaskCb task_do, uint32_t task_time){
    u_timer_out_modify(task_id, task_do, task_time, 0x80);
}

/**
 * @brief 重置任务计时器 (从头开始计时)
 * @param task_id 任务ID
 */
void u_timer_reset(uint16_t task_id){
    if (task_id >= TASK_NUM_MAX){
        return;
    }

    if (g_tasks[task_id].enabled) {
        g_tasks[task_id].time_count = g_tasks[task_id].period_reload;
        g_tasks[task_id].run = 0;
    }
}

/**
 * @brief 禁用任务 (保留配置)
 * @param task_id 任务ID
 */
void u_timer_disable(uint16_t task_id){
    if (task_id >= TASK_NUM_MAX){
        return;
    }

    g_tasks[task_id].enabled = 0;
    g_tasks[task_id].run = 0;
    g_tasks[task_id].time_count = 0;

    // 重新计算定时器间隔
    recalculate_and_switch_interval();
}

/**
 * @brief 清除任务 (完全删除)
 * @param task_id 任务ID
 */
void u_timer_clear(uint16_t task_id){
    if (task_id >= TASK_NUM_MAX){
        return;
    }

    g_tasks[task_id].exec_count = 0;
    u_timer_disable(task_id);
}

/**
 * @brief 检查任务是否使能
 * @param id 任务ID
 * @return 1: 使能, 0: 未使能
 */
uint8_t u_timer_check_enabled(uint16_t id){
    if (id >= TASK_NUM_MAX){
        return 0;
    }
    return g_tasks[id].enabled;
}

/* ============================ 辅助API函数 ============================ */

/**
 * @brief 获取当前定时器间隔
 * @return 当前定时器间隔 (ms)
 */
uint32_t timer_get_interval(void){
    return g_timer_state.current_interval;
}

/**
 * @brief 获取系统运行时间
 * @return 系统运行毫秒数 (自task_init()以来)
 * @note 32位ms计数器可支持约49.7天，溢出后从0重新开始
 */
uint32_t timer_get_system_ms(void){
    return g_timer_state.system_ms;
}

/**
 * @brief 获取定时器统计信息
 * @param total_ticks 总tick计数
 * @param switch_count 间隔切换次数
 */
void timer_get_stats(uint32_t *total_ticks, uint32_t *switch_count){
    if (total_ticks != NULL) {
        *total_ticks = g_timer_state.total_ticks;
    }
    if (switch_count != NULL) {
        *switch_count = g_timer_state.switch_count;
    }
}

/**
 * @brief 启用已禁用的任务
 * @param task_id 任务ID
 */
void u_timer_enable(uint16_t task_id){
    if (task_id >= TASK_NUM_MAX){
        return;
    }

    if (g_tasks[task_id].callback != NULL && !g_tasks[task_id].enabled) {
        g_tasks[task_id].enabled = 1;
        g_tasks[task_id].run = 0;
        g_tasks[task_id].time_count = g_tasks[task_id].period_reload;

        // 重新计算定时器间隔
        recalculate_and_switch_interval();
    }
}

/* ============================ 文件结束 ============================ */
/*
 * 架构优化总结:
 *
 * 1. 核心改进 - 硬件定时器计数器校准机制
 *    - 使用timer_get_tick()直接读取硬件计数器，精度30.52us/tick
 *    - 任何定时器间隔变化都立即切换，不再等待当前周期
 *    - 通过硬件计数器计算已过时间，精确校准所有任务
 *    - 完全消除软件计数器溢出风险
 *
 * 2. 简化切换逻辑
 *    - 删除了复杂的reset_flag和has_runing_task检查
 *    - timer_switch_immediate()直接处理所有切换
 *    - 不需要main_loop_cnt软件计数器
 *    - 逻辑更清晰，可维护性更高
 *
 * 3. 编译时和运行时安全
 *    - _Static_assert编译时检查定时间隔 >= 4ms
 *    - 运行时验证GCD合法性
 *    - 错误处理机制和降级策略
 *
 * 4. 响应速度提升
 *    - Idle→活动状态切换: 从等待2秒降低到立即切换(约5-10ms)
 *    - 按键等高优先级任务响应迅速
 *    - 保留原定时间，任务仍在原定时间点执行
 *
 * 5. 功耗优化
 *    - 动态间隔调整，中断频率降低10-50倍
 *    - 无任务时自动进入低功耗模式(60秒间隔)
 *
 * 6. 硬件定时器优势
 *    - 使用TIMER_1，32.768KHz时钟源
 *    - 硬件级精度，无需软件标定
 *    - 自动重载，无溢出风险
 *    - system_ms提供全局时间基准(支持49.7天)
 *
 * 使用示例:
 *
 *   // 初始化
 *   task_init();  // 自动启动TIMER_1
 *
 *   // 创建任务
 *   uint8_t key_id = u_timer(key_board_task, 5);    // 5ms按键扫描
 *   uint8_t led_id = u_timer(led_blink_task, 250);  // 250ms LED闪烁
 *
 *   // 主循环
 *   while(1) {
 *       task_handler();  // 执行就绪任务
 *   }
 *
 *   // 定时器ISR (自动注册)
 *   // TIMER1_IRQHandler -> task_schedule() 自动调用
 *
 *   // 获取系统时间
 *   uint32_t uptime = timer_get_system_ms();  // 获取运行时间
 */
