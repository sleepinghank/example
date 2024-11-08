//
// Created by hank on 2024/9/12.
//

#include "timer_task.h"

// 计算两个整数的最大公因数 (GCD)
uint32_t gcd(uint32_t a, uint32_t b) {
    while (b != 0) {
        uint32_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}
// 计算两个整数的最小公倍数 (LCM)，基于已知的最大公因数 (GCD)
uint32_t lcm(uint32_t a, uint32_t b) {
    return (a / gcd(a, b)) * b; // 利用公式：lcm(a, b) = |a*b| / gcd(a, b)
}
#define TIMER(task_time,task_do) {.run = 0, .long_flag = 0, .time_count = 0, .do_cnt = 0x80, .task_time = (task_time), .task_cb = (task_do)};
#define TIMER_OUT(task_time,task_do,do_cnt) {.run = 0, .long_flag = 0, .time_count = 0, .do_cnt = do_cnt, .task_time = (task_time), .task_cb = (task_do)};

void key_board_task(void){
    // 按键扫描
}
void tp_task(void){
    // 按键扫描
}
enum timer_id {
    KB_TIMER_ID = 0,
    TP_TIMER_ID,
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

void task_init(void){
    // 初始化定时器
    uint32_t gcd_val = 0;
    uint8_t i = 0;
    for (i = 0; i < sys_timer_end; ++i) {
        uint32_t time_reload = (task_list[i].task_time / timer_task.timer_interval);
        task_list[i].time_count = time_reload;
        timer_task_en |= BIT(i);
        if (gcd_val == 0){
            gcd_val = task_list[i].task_time;
        } else {
            gcd_val = gcd(gcd_val, task_list[i].task_time);
        }
    }
    timer_task.interval_gcd = gcd_val;
    timer_task.timer_interval = gcd_val;
    // TODO: 重新设置定时器
    change_timer_interval();
}

void task_handler(void){
    uint8_t i = 0;
    if (timer_task_en == 0)
        return;
    for (i = 0; i < TASK_NUM_MAX; ++i) {
        if(((timer_task_en >> i ) & 0x01) > 0 && task_list[i].run > 0){
            task_list[i].task_cb();
            task_list[i].run = 0;
            if (task_list[i].do_cnt == 0){
                u_timer_disable(i);
            }
        }
    }
}

void task_schedule(void){
    uint8_t i = 0;
    // 判断是否有任务使能
    if (timer_task_en == 0)
        return;
    for (i = 0; i < TASK_NUM_MAX; ++i) {
        if ((timer_task_en >> i ) & 0x01) {
            if (task_list[i].time_count == 0) {
                uint32_t time_reload = (task_list[i].task_time / timer_task.timer_interval);
                if (task_list[i].do_cnt == 0){
                    continue;
                }
                if (task_list[i].do_cnt & 0x80) {
                    // 反复执行
                    task_list[i].run = 1;
                    task_list[i].time_count = time_reload;
                } else if (task_list[i].do_cnt > 0) {
                    // 执行次数
                    task_list[i].run = 1;
                    task_list[i].time_count = time_reload;
                    task_list[i].do_cnt--;
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

// 更改定时器间隔，计算
void calculate_timer_interval(void){
    uint32_t gcd_val = 0;
    uint8_t i = 0;
    for (i = 0; i < TASK_NUM_MAX; ++i) {
        if ((timer_task_en >> i) & 0x01){
            if (gcd_val == 0){
                gcd_val = task_list[i].task_time;
            } else {
                gcd_val = gcd(gcd_val, task_list[i].task_time);
            }
        }
    }
    // 需要更新定时器间隔
    if (gcd_val != timer_task.timer_interval){
        timer_task.interval_gcd = gcd_val;
        timer_task.reset_flag = 1;
    }
}

uint8_t u_timer_out(TimeTaskCb task_do ,uint32_t task_time, uint8_t do_cnt)
{
    uint8_t timer_id = 0xff;
    uint8_t i = 0;
    // 找到空闲位置
    for (i = 0; i < TASK_NUM_MAX; ++i) {
        if (((timer_task_en >> i) & 0x01) == 0 && task_list[i].do_cnt == 0){
            modify_task(task_do, task_time, do_cnt, i);
            timer_id = i;
        }
    }
    if (timer_id != 0xff){
        calculate_timer_interval();
    }
    return timer_id;
}

void modify_task(TimeTaskCb task_do, uint32_t task_time, uint8_t do_cnt, uint16_t i) {// 新增任务
    uint32_t time_reload = (task_time / timer_task.timer_interval);
    task_list[i].run = 0;
    task_list[i].task_time = task_time;
    task_list[i].do_cnt = do_cnt; // 执行几次
    task_list[i].task_cb = task_do;
    task_list[i].time_count = time_reload;
    if (task_time >= 2000){
        // 为长时间定时器，
        task_list[i].long_flag = 1;
    }
    timer_task_en |= BIT(i);
}

uint8_t u_timer(TimeTaskCb task_do ,uint32_t task_time)
{
    return u_timer_out(task_do, task_time, 0x80);
}
void u_timer_out_modify(uint16_t task_id, TimeTaskCb task_do ,uint32_t task_time, uint8_t do_cnt){
    if (task_id >= TASK_NUM_MAX){
        return;
    }
    modify_task(task_do, task_time, do_cnt, task_id);
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
    timer_task_en &= ~(BIT(task_id));
    task_list[task_id].run = 0;
    task_list[task_id].time_count = 0;
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
