//
// Created by hank on 2024/9/12.
//

#ifndef C_TIMER_TASK_H
#define C_TIMER_TASK_H
#include <stdint.h>

#define TASK_NUM_MAX 16
#define U32BIT(s)			((uint32_t)1<<(s))
#define BIT(s)				((uint8_t)1<<(s))
typedef void (* TimeTaskCb)(void);

typedef struct {
    uint8_t run:1; // 任务是否运行
    uint8_t long_flag:1; // 是否为长时间定时器，切换报告率时会特殊处理
    uint8_t :6; // 预留
    uint32_t time_count; // 任务计时 以定时器为周期计算
    uint8_t do_cnt; // 任务执行次数
    uint32_t task_time; // 任务时间 ms
    TimeTaskCb task_cb; // 任务函数
} TaskComps_t;

typedef struct {
    uint16_t timer_interval; // 当前定时器间隔
    uint16_t interval_gcd; // 最大公约数
    uint8_t reset_flag:1; // 是否需要重设定时器间隔
    uint8_t :7;
} TimerTask_t;

/// @brief 任务初始化
void task_init(void);
/// @brief 用于计算任务是否执行,定时器中调用
void task_schedule(void);
/// @brief 任务处理函数，主循环中调用
void task_handler(void);
/// @brief 停止一个定时任务
/// @param task_id 任务id
void u_timer_disable(uint16_t task_id);
/// @brief 清除一个定时任务
/// @param task_id 任务id
void u_timer_clear(uint16_t task_id);
/// @brief 检测一个任务是否使能
/// @param task_id 任务id
/// @return 1：使能 0：无使能
uint8_t u_timer_check_enabled(uint16_t task_id);
/// @brief 创建一个定时任务
/// @param task_do 执行的任务函数
/// @param task_time 执行间隔时间
/// @return 任务id，可用于后续操作任务
uint8_t u_timer(TimeTaskCb task_do ,uint32_t task_time);
/// @brief 创建一个定时执行，限定执行次数的任务
/// @param task_do 执行的任务函数
/// @param task_time 执行间隔时间
/// @param do_cnt 任务执行次数
/// @return 任务id，可用于后续操作任务
uint8_t u_timer_out(TimeTaskCb task_do ,uint32_t task_time, uint8_t do_cnt);
/// @brief 重置一个任务
/// @param task_id 任务id
void u_timer_reset(uint16_t task_id);
/// @brief 修改一个任务
/// @param task_id 任务id
/// @param task_do 执行的任务函数
/// @param task_time 执行间隔时间
void u_timer_modify(uint16_t task_id, TimeTaskCb task_do ,uint32_t task_time);
/// @brief 修改一个任务
/// @param task_id 任务id
/// @param task_do 执行的任务函数
/// @param task_time 执行间隔时间
/// @param do_cnt 任务执行次数
void u_timer_out_modify(uint16_t task_id, TimeTaskCb task_do ,uint32_t task_time, uint8_t do_cnt);
#endif //C_TIMER_TASK_H
