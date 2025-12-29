/**
 * @file timer_task.h
 * @brief 动态定时器任务管理系统
 * @author hank
 * @date 2024/9/12
 *
 * @note 这是一个智能的动态定时器管理系统，能够根据当前活跃任务的时间间隔
 *       自动计算最优的定时器间隔，从而在满足任务执行精度的同时最大程度降低功耗。
 *
 * 核心特性：
 * - 智能间隔计算：根据所有活跃任务的GCD自动计算最优定时器间隔
 * - 功耗优化：高频任务活跃时使用小间隔，只有低频任务时使用大间隔
 * - 最小间隔保护：限制最小间隔为4ms，确保系统稳定性
 * - 动态调整：任务添加/删除时自动重新计算定时器间隔
 * - 灵活执行模式：支持重复执行和指定次数执行
 * - 完整的任务管理：支持启用、禁用、重置、修改等操作
 *
 * 典型应用场景：
 * - 键盘扫描任务(5ms) + 触摸扫描任务(8ms) + 系统维护任务(1000ms)
 *   -> GCD(5,8,1000) = 1ms < 4ms -> 使用4ms间隔
 * - 只有idle检测任务(2000ms)
 *   -> 只有一个任务 -> 使用2000ms间隔，大大降低功耗
 *
 * 使用示例：
 * @code
 * // 1. 系统初始化
 * task_init();
 *
 * // 2. 创建不同频率的任务
 * uint8_t kb_id = u_timer(keyboard_scan_task, 5);     // 5ms键盘扫描
 * uint8_t tp_id = u_timer(touch_scan_task, 8);       // 8ms触摸扫描
 * uint8_t idle_id = u_timer(idle_check_task, 2000);  // 2s空闲检测
 *
 * // 3. 系统运行
 * while(1) {
 *     // 定时器中断中调用 (频率由系统自动调整)
 *     task_schedule();
 *
 *     // 主循环中调用，执行到期任务
 *     task_handler();
 *
 *     // 其他应用逻辑...
 *
 *     // 4. 动态任务管理示例
 *     if (enter_idle_mode) {
 *         // 进入idle模式：停止高频任务，节省功耗
 *         u_timer_disable(kb_id);
 *         u_timer_disable(tp_id);
 *         // 系统自动将定时器间隔调整为2000ms
 *     } else if (exit_idle_mode) {
 *         // 退出idle模式：重新启动高频任务
 *         u_timer_enable(kb_id);
 *         u_timer_enable(tp_id);
 *         // 系统自动将定时器间隔调整为4ms
 *     }
 * }
 * @endcode
 */

#ifndef C_TIMER_TASK_H
#define C_TIMER_TASK_H
#include <stdint.h>

#define TASK_NUM_MAX 16
#define TIMER_INTERVAL_MIN_MS    4   // 最小定时器间隔 4ms，确保系统稳定性
#define TIMER_INTERVAL_MAX_MS    (UINT32_MAX / 1000)  // 最大安全间隔约49天
#define TIMER_INTERVAL_DEFAULT_MS 1000  // 默认间隔1s

// 任务优先级定义
#define TASK_PRIORITY_LOWEST      0   // 最低优先级
#define TASK_PRIORITY_LOW        64  // 低优先级
#define TASK_PRIORITY_NORMAL     128 // 普通优先级
#define TASK_PRIORITY_HIGH       192 // 高优先级
#define TASK_PRIORITY_CRITICAL   255 // 最高优先级

// 错误码定义
#define TIMER_OK                 0   // 操作成功
#define TIMER_ERROR_INVALID_ID  1   // 无效任务ID
#define TIMER_ERROR_NO_SPACE     2   // 没有空闲槽位
#define TIMER_ERROR_INVALID_TIME 3   // 无效时间参数
#define TIMER_ERROR_SYSTEM       4   // 系统错误

#define U32BIT(s)			((uint32_t)1<<(s))
#define BIT(s)				((uint8_t)1<<(s))

typedef void (* TimeTaskCb)(void);

typedef struct {
    uint8_t run:1;             // 任务是否运行
    uint8_t long_flag:1;       // 是否为长时间定时器，切换报告率时会特殊处理
    uint8_t immediate:1;       // 是否在中断中立即执行（高优先级任务）
    uint8_t enabled:1;         // 任务是否启用
    uint8_t :4;                // 预留
    uint8_t priority;          // 任务优先级 (0-255)
    uint32_t time_count;       // 任务计时 以定时器为周期计算
    uint8_t do_cnt;            // 任务执行次数 (0x80表示无限执行)
    uint32_t task_time;        // 任务时间 ms
    TimeTaskCb task_cb;        // 任务函数
    const char* name;          // 任务名称（调试用）
} TaskComps_t;

typedef struct {
    uint16_t timer_interval;      // 当前定时器间隔(ms)
    uint16_t interval_gcd;        // 最大公约数
    uint8_t reset_flag:1;         // 是否需要重设定时器间隔
    uint8_t initialized:1;        // 系统是否已初始化
    uint8_t :6;                   // 预留
} TimerTask_t;

// ==================== 系统接口 ====================

/// @brief 初始化定时器系统
/// @return 错误码 (TIMER_OK表示成功)
uint8_t task_init(void);

/// @brief 初始化示例任务 - 展示如何使用定时器系统
/// @return 错误码 (TIMER_OK表示成功)
uint8_t task_init_example(void);

/// @brief 定时器中断服务函数中调用 - 负责任务调度和计时
/// @note 需要在定时器中断中以固定频率调用
void task_schedule(void);

/// @brief 主循环中调用 - 执行到期的任务
/// @note 需要在主循环中调用，处理实际任务执行
void task_handler(void);

// ==================== 任务管理接口 ====================

/// @brief 创建重复执行的高优先级定时任务
/// @param task_do 任务回调函数
/// @param task_time 执行间隔时间(ms)
/// @param priority 任务优先级
/// @param immediate 是否在中断中立即执行
/// @return 任务ID (0xFF表示失败)
uint8_t u_timer_priority(TimeTaskCb task_do, uint32_t task_time, uint8_t priority, uint8_t immediate);

/// @brief 创建重复执行的定时任务（兼容性接口）
/// @param task_do 任务回调函数
/// @param task_time 执行间隔时间(ms)
/// @return 任务ID (0xFF表示失败)
uint8_t u_timer(TimeTaskCb task_do, uint32_t task_time);

/// @brief 创建指定执行次数的定时任务
/// @param task_do 任务回调函数
/// @param task_time 执行间隔时间(ms)
/// @param do_cnt 任务执行次数(0表示一次性，0x80表示无限重复)
/// @return 任务ID (0xFF表示失败)
uint8_t u_timer_out(TimeTaskCb task_do, uint32_t task_time, uint8_t do_cnt);

/// @brief 禁用(停止)一个定时任务
/// @param task_id 任务ID
/// @return 错误码 (TIMER_OK表示成功)
uint8_t u_timer_disable(uint16_t task_id);

/// @brief 启用一个定时任务
/// @param task_id 任务ID
/// @return 错误码 (TIMER_OK表示成功)
uint8_t u_timer_enable(uint16_t task_id);

/// @brief 清除一个定时任务(完全删除)
/// @param task_id 任务ID
/// @return 错误码 (TIMER_OK表示成功)
uint8_t u_timer_clear(uint16_t task_id);

/// @brief 重置一个任务的计时器(重新开始计时)
/// @param task_id 任务ID
/// @return 错误码 (TIMER_OK表示成功)
uint8_t u_timer_reset(uint16_t task_id);

/// @brief 检查任务是否处于使能状态
/// @param task_id 任务ID
/// @return 1：使能 0：禁用
uint8_t u_timer_check_enabled(uint16_t task_id);

/// @brief 修改一个重复执行的任务
/// @param task_id 任务ID
/// @param task_do 新的任务回调函数
/// @param task_time 新的执行间隔时间(ms)
/// @return 错误码 (TIMER_OK表示成功)
uint8_t u_timer_modify(uint16_t task_id, TimeTaskCb task_do, uint32_t task_time);

/// @brief 修改一个指定执行次数的任务
/// @param task_id 任务ID
/// @param task_do 新的任务回调函数
/// @param task_time 新的执行间隔时间(ms)
/// @param do_cnt 新的任务执行次数
/// @return 错误码 (TIMER_OK表示成功)
uint8_t u_timer_out_modify(uint16_t task_id, TimeTaskCb task_do, uint32_t task_time, uint8_t do_cnt);

/// @brief 获取当前定时器间隔
/// @return 当前定时器间隔(ms)
uint16_t task_get_current_interval(void);
#endif //C_TIMER_TASK_H
