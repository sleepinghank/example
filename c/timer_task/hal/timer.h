/// @file  timer.h
/// @brief PAR2860 TIMER driver
#ifndef _TIMER_H_
#define _TIMER_H_

#include "ARMCM0.h"

/// @defgroup TIMERRoutines TIMER
/// @{
///     @brief driver document
///
///     Register
///
///     @ref TIMER_CTRL_TYPE
///         @ref H_TIMER_CTRL_TYPE
/// @brief Number of Timer modules


#define CS_IN_TIMER(x) { uint32_t old_primask_timer = NVIC_GetEnableIRQ((IRQn_Type)x); NVIC_DisableIRQ((IRQn_Type)x)
#define CS_OUT_TIMER(x) if(old_primask_timer) NVIC_EnableIRQ((IRQn_Type)x); }



#define    TIMER_NUM            3
typedef enum
{
    H_TIMER_PERSCALER_1 = 0,
    H_TIMER_PERSCALER_2 = 1,
    H_TIMER_PERSCALER_4 = 2,
    H_TIMER_PERSCALER_8 = 3,
    H_TIMER_PERSCALER_16 = 4,
    H_TIMER_PERSCALER_32 = 5,
    H_TIMER_PERSCALER_64 = 6,
    H_TIMER_PERSCALER_128 = 7,
} h_timer_perscaler_type;

typedef enum
{
    H_TIMER_UP = 0,
    H_TIME_DOWN = 1,
    H_TIMER_UP_AND_DOWN = 2,
} h_timer_counter_dir_type;

typedef enum
{
    H_TIMER_INT_NONE = 0,
    H_TIMER_INT = 1,
    H_TIMER_INT_TASK_STOP = 2,
    H_TIMER_INT_ALL  = 3,
} h_timer_int_type;

/// @brief Timer interrupt callback type
typedef void (* timer_irq_callback_type)(void);
typedef void (* high_speed_timer_irq_callback_type)(void);

//user  timer 
typedef void (* user_timer_task)(void);

/// @brief Enable timer
/// Interrupt callback will be invoked when counter reset
/// @param id Module ID<BR>Availabe option 0~2
/// @param callback @ref TIMER_IRQ_CALLBACK_TYPE
/// @param interval Timer reload value<BR><B>Frequency 32.768KHz</B>
/// @param int_enable 0 - Disable<BR>1 - Enable
/// @return 0 if error<BR>Reload value set if success
uint32_t timer_enable(int id, timer_irq_callback_type callback, uint32_t interval, int int_enable);

/// @brief Get counter value of timer
/// @param id Module ID<BR>Availabe option 0~2
/// @return Tick count
uint32_t timer_get_tick(int id);
/// @brief Disable timer
/// @param timer_id ,which timer selelct to disable
void timer_disable(uint8_t timer_id);

/// @brief Timer delay 32us,clk requency 32.768KHz
/// @param id Timer module id
/// @param cnt num of 32us,if cnt=1 delay 32us , and so forth.
void timer_delay_32us(uint8_t id, uint32_t cnt);
/// @brief interval reload value be used to interrupt
/// @param h_timer_perscaler_type prescaler ,Counter clk = mcu clock/(prescaler),default value :0 @ref H_TIMER_PERSCALER_TYPE
/// @param hi_timer_callback, hi timer interrupt  callback
void high_speed_timer_enable(uint16_t interval, h_timer_perscaler_type h_timer_perscaler_type, high_speed_timer_irq_callback_type hi_timer_callback);
/// @brief Timer delay 32us,clk requency 32.768KHz
/// @param id Timer module id
/// @param cnt num of 32us,if cnt=1 delay 32us , and so forth.
void high_speed_timer_disable(void);
/// @brief Check timer enable or disable
/// @param id Module ID<BR>Availabe option 0~2
/// @return 0 is disable, 1 is enable
uint8_t timer_check_enabled(int id);
/// @brief Get the reload value of timer
/// @param id Module ID<BR>Availabe option 0~2
/// @return reload value
uint32_t timer_get_interval(int id);
/// @brief Get the status of timer
/// @param id Module ID<BR>Availabe option 0~2
/// @return reload value indicates the timer is expired or not
int timer_expired(int id);
///@}

void led_stact(void);
// ·························································································································
typedef enum
{
    U_TIMER0 = 0,
    U_TIMER1,
    U_TIMER2,
    U_TIMER3,
    U_TIMER4,
    U_TIMER5,
    U_TIMER6,
    U_TIMER7,
} u_timer_id;

/// @brief 设置自定义任务，默认绑定定时器1，内部间隔5ms，最多支持8个任务,自动启动自动关闭，不能放在__enable_irq()之前执行
/// @param task_nu 任务编号 0~7
/// @param task_do 任务函数
/// @param task_time 任务定时时间 ms, 请设置为5的倍数，最小5ms
/// @param task_enable 任务使能
void user_timer_enable(uint8_t task_nu,user_timer_task task_do, uint32_t task_time, uint8_t task_enable);
/// @brief 关闭自定义任务
/// @param task_nu 任务编号 0~7
void user_timer_disable(uint8_t task_nu);
/// @brief 重置所有自定义任务
void user_timer_clear(void);

/// @brief 设置自定义任务，默认共用系统定时器，只需要添加任务即可。
/// @param task_nu 任务编号 0~7
/// @param task_do 任务函数
/// @param task_time 任务定时时间 ms, 请设置为系统定时器间隔的倍数
/// @param task_enable 任务使能
void u_timer_enable(uint8_t task_nu,user_timer_task task_do, uint32_t task_time, uint8_t task_enable);
/// @brief 超时触发任务（只执行一次），和u_timer_enable共用系统编号
/// @param task_nu 任务编号 0~7
/// @param task_do 任务函数
/// @param task_time 任务定时时间 ms, 请设置为系统定时器间隔的倍数
/// @param task_enable 任务使能
void u_timerout_enable(uint8_t task_nu,user_timer_task task_do, uint32_t task_time, uint8_t task_enable);
/// @brief 关闭自定义任务
/// @param task_nu 任务编号 0~7
void u_timer_disable(uint8_t task_nu);
/// @brief 用户自定义执行任务
void _u_timer_task(void);

#endif


