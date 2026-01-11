#include <string.h>
#include "timer.h"
#include "delay.h"
#include "gpio.h"
#include "pxi_par2860_ble_lib.h"
#include "kb_para_profile.h"

static TIMER_CTRL_TYPE*         _timer_ctrl     = ((TIMER_CTRL_TYPE*) TIMER_CTRL_BASE);
static H_TIMER_CTRL_TYPE*   _h_timer_ctrl = ((H_TIMER_CTRL_TYPE*) H_TIMER_CTRL_BASE);

uint8_t led_stact_flag = 0;
//static h_timer_int_type _htimer_int_status = H_TIMER_INT_NONE;
static timer_irq_callback_type _timer_cb[TIMER_NUM];
static high_speed_timer_irq_callback_type _hi_timer_cb;
void led_time_cnt(void)	;

void led_1_sotp(void);



void timer_delay_32us(uint8_t id, uint32_t cnt)
{

    switch (id)
    {

    case 0:
        _timer_ctrl->TIMER_0_VAL = cnt;
        _timer_ctrl->TIMER_0_INT_MASK = 1;
        _timer_ctrl->TIMER_0_INT_CLR = 1;
        _timer_ctrl->TIMER_0_EN = 1;

        while (_timer_ctrl->TIMER_0_INT_STATUS == 0);

        _timer_ctrl->TIMER_0_EN = 0;
        _timer_ctrl->TIMER_0_INT_CLR = 1;
        break;

    case 1:
        _timer_ctrl->TIMER_1_VAL = cnt;
        _timer_ctrl->TIMER_1_INT_MASK = 1;
        _timer_ctrl->TIMER_1_INT_CLR = 1;
        _timer_ctrl->TIMER_1_EN = 1;

        while (_timer_ctrl->TIMER_1_INT_STATUS == 0);

        _timer_ctrl->TIMER_1_EN = 0;
        _timer_ctrl->TIMER_1_INT_CLR = 1;
        break;

    case 2:
        _timer_ctrl->TIMER_2_VAL = cnt;
        _timer_ctrl->TIMER_2_INT_MASK = 1;
        _timer_ctrl->TIMER_2_INT_CLR = 1;
        _timer_ctrl->TIMER_2_EN = 1;

        while (_timer_ctrl->TIMER_2_INT_STATUS == 0);

        _timer_ctrl->TIMER_2_EN = 0;
        _timer_ctrl->TIMER_2_INT_CLR = 1;
        break;

    }
}


void timer_disable(uint8_t timer_id)
{
    switch (timer_id)
    {

    case 0:
        _timer_ctrl->TIMER_0_EN = 0;
        _timer_ctrl->TIMER_0_INT_CLR = 1;
        NVIC_DisableIRQ(TIMER0_IRQn);
        
        break;        

    case 1:
        _timer_ctrl->TIMER_1_EN = 0;
        _timer_ctrl->TIMER_1_INT_CLR = 1;
        NVIC_DisableIRQ(TIMER1_IRQn);
        
        break;

    case 2:
        _timer_ctrl->TIMER_2_EN = 0;
        _timer_ctrl->TIMER_2_INT_CLR = 1;
        NVIC_DisableIRQ(TIMER2_IRQn);
        
        break;

    default:
        break;
    }

}
uint8_t timer_check_enabled(int id)
{

    uint8_t en = 0;

    switch (id)
    {

    case 0:
        en = _timer_ctrl->TIMER_0_EN;
        break;

    case 1:
        en = _timer_ctrl->TIMER_1_EN;
        break;

    case 2:
        en = _timer_ctrl->TIMER_2_EN;
        break;

    default:
        break;
    }

    return en;
}


uint32_t timer_enable(int id, timer_irq_callback_type callback, uint32_t interval, int int_enable)
{
    if (id >= TIMER_NUM || timer_check_enabled(id))
        return 0;

    _timer_cb[id] = callback;

    switch (id)
    {

    case 0:

        _timer_ctrl->TIMER_0_VAL = interval;
        pxi_delay_us(64);
        _timer_ctrl->TIMER_0_RGLR = 1;       //1:reload enable
        _timer_ctrl->TIMER_0_INT_MASK = (int_enable == 1) ? 0 : 1; //1:mask int
        _timer_ctrl->TIMER_0_INT_CLR = 1;
        _timer_ctrl->TIMER_0_EN = 1;
        NVIC_EnableIRQ(TIMER0_IRQn);
        
        break;

    case 1:

        _timer_ctrl->TIMER_1_VAL = interval;
        pxi_delay_us(64);
        _timer_ctrl->TIMER_1_RGLR = 1;       //1:reload enable
        _timer_ctrl->TIMER_1_INT_MASK = (int_enable == 1) ? 0 : 1; //1:mask int
        _timer_ctrl->TIMER_1_INT_CLR = 1;
        _timer_ctrl->TIMER_1_EN = 1;
        NVIC_EnableIRQ(TIMER1_IRQn);

        break;

    case 2:

        _timer_ctrl->TIMER_2_VAL = interval;
        pxi_delay_us(64);
        _timer_ctrl->TIMER_2_RGLR = 1;       //1:reload enable
        _timer_ctrl->TIMER_2_INT_MASK = (int_enable == 1) ? 0 : 1; //1:mask int
        _timer_ctrl->TIMER_2_INT_CLR = 1;        
        _timer_ctrl->TIMER_2_EN = 1;      
        NVIC_EnableIRQ(TIMER2_IRQn);

        break;
    }


    return interval;
}



uint32_t timer_get_tick(int id)
{
    uint32_t cnt = 0, cnt2 = 0;

    if (id >= TIMER_NUM || !timer_check_enabled(id))
        return 0;

    switch (id)
    {

        case 0:
            cnt = _timer_ctrl->TIMER_0_CNT;
            while(1)
            {
                cnt2 = _timer_ctrl->TIMER_0_CNT;
                if (cnt2 == cnt)
                    break;
                cnt = cnt2;
            }
            break;

        case 1:
            cnt = _timer_ctrl->TIMER_1_CNT;

            while(1)
            {
                cnt2 = _timer_ctrl->TIMER_1_CNT;
                if (cnt2 == cnt)
                    break;
                cnt = cnt2;
            }

            break;

        case 2:
            cnt = _timer_ctrl->TIMER_2_CNT;

            while(1)
            {
                cnt2 = _timer_ctrl->TIMER_2_CNT;
                if (cnt2 == cnt)
                    break;
                cnt = cnt2;
            }

            break;


        default:
            break;
    }

    return cnt;

}

uint32_t timer_get_interval(int id)
{

    uint32_t interval = 0;
    if (id >= TIMER_NUM || !timer_check_enabled(id))
        return 0;

    switch (id)
    {

        case 0:
            interval = _timer_ctrl->TIMER_0_VAL;

            break;

        case 1:
            interval = _timer_ctrl->TIMER_1_VAL;

			
            break;

        case 2:
            interval = _timer_ctrl->TIMER_2_VAL;
            break;
				
        default:
            break;
    }

    return interval;

}


int timer_expired(int id)
{
    uint8_t status;
    switch (id)
    {

        case 0:
            status = _timer_ctrl->TIMER_0_INT_STATUS;
            break;

        case 1:
            status = _timer_ctrl->TIMER_1_INT_STATUS;
            break;

        case 2:
            status = _timer_ctrl->TIMER_2_INT_STATUS;
            break;

    }

    return status;


}
void high_speed_timer_set_prescaler(h_timer_perscaler_type h_timer_perscaler_type)
{

    _h_timer_ctrl->PRESCALER = h_timer_perscaler_type;

    switch (h_timer_perscaler_type)
    {

    case H_TIMER_PERSCALER_1:

        break;

    case H_TIMER_PERSCALER_2:

        break;

    case H_TIMER_PERSCALER_4:

        break;

    case H_TIMER_PERSCALER_8:

        break;

    case H_TIMER_PERSCALER_16:

        break;

    case H_TIMER_PERSCALER_32:

        break;

    case H_TIMER_PERSCALER_64:

        break;

    case H_TIMER_PERSCALER_128:

        break;
    }

}

void high_speed_timer_enable(uint16_t interval, h_timer_perscaler_type h_timer_perscaler_type, high_speed_timer_irq_callback_type hi_timer_callback)
{

    if (_h_timer_ctrl->BUSY == 1)
        _h_timer_ctrl->STOP = 1;

    _h_timer_ctrl->ONE_TIME = 0;
    _h_timer_ctrl->COUNTER_TOP = interval;

    _h_timer_ctrl->PRESCALER = h_timer_perscaler_type;

    _hi_timer_cb = hi_timer_callback;
    _h_timer_ctrl->EVENTS = 1;       //clr Int
    _h_timer_ctrl->INTEN = 1;
    NVIC_EnableIRQ(HTIMER_IRQn);

    _h_timer_ctrl->START = 1;  //set H_TIMER_CTRL->BUSY

}
void high_speed_timer_disable(void)
{
    _h_timer_ctrl->STOP = 1;

}

void TIMER0_IRQHandler(void)
{
    if (_timer_ctrl->TIMER_0_INT_STATUS == 1)
    {
        _timer_ctrl->TIMER_0_INT_CLR = 1;


        if (_timer_cb[0] != 0)
            (*_timer_cb[0])();
    }
}
void TIMER1_IRQHandler(void)
{
    if (_timer_ctrl->TIMER_1_INT_STATUS == 1)
    {
        _timer_ctrl->TIMER_1_INT_CLR = 1;


        if (_timer_cb[1] != 0)
            (*_timer_cb[1])();
    }
}

void TIMER2_IRQHandler(void)
{
    if (_timer_ctrl->TIMER_2_INT_STATUS == 1)
    {
        _timer_ctrl->TIMER_2_INT_CLR = 1;


        if (_timer_cb[2] != 0)
            (*_timer_cb[2])();
    }
}


void HTIMER_IRQHandler(void)
{
    //_htimer_int_status = (h_timer_int_type)_h_timer_ctrl->EVENTS;

    //clr INT

    _h_timer_ctrl->EVENTS = H_TIMER_INT_ALL;

    (*_hi_timer_cb)();

}
//--------------------------------------------------------
void led_stact(void)
{
  led_stact_flag = 1;
}

void led_1_sotp(void)
{
  led_stact_flag = 0;
}
uint8_t get_led_breath_stact(void){
    return led_stact_flag;
}



// ----------------------- 自定义定时器 -----------------------
struct _user_task_do_
{
    user_timer_task task_do;//自定义任务
    uint32_t task_th;// 定时触发 频次
    uint32_t task_cnt;
    uint8_t do_cnt;
};
static struct _user_task_do_ tasks[8] = {0};
static uint8_t _user_timer_task_ = 0;//8位 0-7位对应8个任务
#define USER_INTERVAL 5
#define USER_TIMER_NUM 8

/***
 *  定时任务执行
 *  USER_INTERVAL ms调用一次
 * */
void _u_timer_task(void)
{
    uint8_t i;
    if (_user_timer_task_ == 0)
        return;

    //按照每USER_INTERVAL ms触发一次计算时间任务
    for (i = 0; i < 8; i++)
    {
        if ((_user_timer_task_ >> i ) & 0x01)
        {
            if ( tasks[i].task_cnt == 0)
            {
                tasks[i].task_cnt = tasks[i].task_th;
                if (tasks[i].do_cnt > 0)
                    u_timer_disable(i);
                if (tasks[i].task_do != 0)
                    (*tasks[i].task_do)();
            } else {
                tasks[i].task_cnt --;
            }

        }
    }

}

// 检查自定义定时器任务使能是否开启
uint8_t u_timer_check_enabled(int id){
    uint8_t en = 0;
    if (id >= USER_TIMER_NUM){
        return en;
    }
    en = (_user_timer_task_ >> id) & 0x01;
    return en;
}

void u_timer_disable(uint8_t task_nu){
    _user_timer_task_ &= ~(BIT(task_nu));
}

void u_timer_enable(uint8_t task_nu, user_timer_task task_do ,uint32_t task_time, uint8_t task_enable)
{
    if (task_nu >= USER_TIMER_NUM || u_timer_check_enabled(task_nu))
        return ;

    tasks[task_nu].task_do = task_do;
    tasks[task_nu].task_th = KEYBOARD_CNT_MS(task_time);
    if (tasks[task_nu].task_th <= 0)
        tasks[task_nu].task_th = 1;

    tasks[task_nu].task_cnt = tasks[task_nu].task_th;
    if (task_enable) {
        _user_timer_task_ |= BIT(task_nu);
    }
}

void u_timerout_enable(uint8_t task_nu, user_timer_task task_do ,uint32_t task_time, uint8_t task_enable)
{
    if (task_nu >= USER_TIMER_NUM || u_timer_check_enabled(task_nu))
        return ;

    tasks[task_nu].task_do = task_do;
    tasks[task_nu].do_cnt = 1;
    tasks[task_nu].task_th = KEYBOARD_CNT_MS(task_time);
    if (tasks[task_nu].task_th <= 0)
        tasks[task_nu].task_th = 1;

    tasks[task_nu].task_cnt = tasks[task_nu].task_th;
    if (task_enable) {
        _user_timer_task_ |= BIT(task_nu);
    }
}

void user_timer_enable(uint8_t task_nu, user_timer_task task_do ,uint32_t task_time, uint8_t task_enable)
{
    // uint8_t befor_user_timer_task_ = _user_timer_task_;
    u_timer_enable(task_nu, task_do, task_time, task_enable);
    if (task_enable == 1) {
        // 判断是否需要开启定时器
        // if (befor_user_timer_task_ == 0 && _user_timer_task_ > 0){
        //     timer_enable(1, _u_timer_task,0xa3,1);
        // }
    }
}

// 关闭某一个定时任务
void user_timer_disable(uint8_t task_nu){
    // uint8_t befor_user_timer_task_ = _user_timer_task_;
    // // 判断是否需要关闭定时器
    // if (befor_user_timer_task_ > 0 && _user_timer_task_ == 0){
    //     timer_disable(1);
    // }
    u_timer_disable(task_nu);
}

// 清除所有自定义任务
void user_timer_clear(void){
    _user_timer_task_ = 0;
    memset(tasks, 0, sizeof(tasks));
    // timer_disable(1);s
}
