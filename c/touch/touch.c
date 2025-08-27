//
// Created by hank on 2025/8/5.
//

#include "touch.h"
#include "math.h"
#include "stdio.h"
static PTP_Report ptp_reports;
static TOUCHPAD_EVENT original_reports;

void send_touch_data(void){
    // 发送触摸板数据
    // touch_tx_data(TOUCHPAD_REPORT_IDX,sizeof(PTP_Report),  (uint8_t*)&ptp_reports);
    // 这里可以添加实际的发送逻辑
//    打印坐标数据
//    printf("Sending touch data: X: %d, Y: %d, Contact Count: %d, Scan Time: %d\n",
//           ptp_reports.finger_rpt[0].x_l8 | (ptp_reports.finger_rpt[0].x_m4 << 8),
//           ptp_reports.finger_rpt[0].y_l4 | (ptp_reports.finger_rpt[0].y_m8 << 4),
//           ptp_reports.contactCnt,
//           ptp_reports.scantime_l8 | (ptp_reports.scantime_m8 << 8));
//       以json格式打印
    printf("{\"x\": %d, \"y\": %d, \"contact_count\": %d, \"scan_time\": %d},\n",
           ptp_reports.finger_rpt[0].x_l8 | (ptp_reports.finger_rpt[0].x_m4 << 8),
           ptp_reports.finger_rpt[0].y_l4 | (ptp_reports.finger_rpt[0].y_m8 << 4),
           ptp_reports.contactCnt,
           ptp_reports.scantime_l8 | (ptp_reports.scantime_m8 << 8));
    return ;
}

uint8_t touch_count = 0; // 触摸点计数
uint16_t scan_time =0;



#include <math.h>

// 获取圆坐标 - 使用sin和cos函数
void getCirclePoint(uint16_t current_time, uint16_t total_time, double radius, int16_t *x, int16_t *y) {
    // 计算角度（弧度）
    double angle = (2.0 * M_PI * current_time) / total_time;
    
    // 使用sin和cos函数计算坐标，直接使用radius
    *x = (int16_t)(radius * cos(angle));
    *y = (int16_t)(radius * sin(angle));
}

uint8_t circle_cnt = 0; // 圆的计数


// 测试触摸板,直接画特定圆，并发送触控板数据
uint8_t touch_auto_reporting(void){
    uint16_t x_pos, y_pos;
    int16_t x, y;
    
    // 计算圆心和半径
    uint16_t center_x = TOUCHPAD_MAX_X / 2;
    uint16_t center_y = TOUCHPAD_MAX_Y / 2;
    uint16_t radius = TOUCHPAD_MAX_Y / 3;
    
    // 使用sin和cos函数获取圆上的点，直接使用radius
    getCirclePoint(scan_time, 20000, (double )radius, &x, &y);
//    printf("Circle Count: %d, Scan Time: %d, X: %d, Y: %d\n",
//           circle_cnt, scan_time, x, y);
    // 计算实际坐标位置（x和y已经是相对于圆心的偏移）
    x_pos = center_x + x;
    y_pos = center_y + y;

    // 确保坐标在有效范围内
    if (x_pos >= TOUCHPAD_MAX_X) x_pos = TOUCHPAD_MAX_X - 1;
    if (y_pos >= TOUCHPAD_MAX_Y) y_pos = TOUCHPAD_MAX_Y - 1;
    if (x_pos < 0) x_pos = 0;
    if (y_pos < 0) y_pos = 0;
    // 以json格式打印
//    printf("{\"x\": %d, \"y\": %d, \"scan_time\": %d},\n",
//           x_pos, y_pos,  scan_time);
    

    ptp_reports.scantime_l8= (uint8_t)(scan_time&0x00ff);
    ptp_reports.scantime_m8= (uint8_t)((scan_time&0xff00)>>8);
    ptp_reports.finger_rpt[0].x_l8 = x_pos & 0xff;
    ptp_reports.finger_rpt[0].x_m4 = (x_pos  >> 8 )& 0x0f;
    ptp_reports.finger_rpt[0].y_l4 = y_pos & 0x0f;
    ptp_reports.finger_rpt[0].y_m8 = y_pos >> 4;
    ptp_reports.finger_rpt[0].tip = 1;
    ptp_reports.finger_rpt[0].confidence = 1;
    ptp_reports.finger_rpt[0].contactID = 0;
    ptp_reports.button = 1;
    ptp_reports.contactCnt = 1;
    send_touch_data();
    touch_count = 2;
    scan_time+=150;
    if (scan_time > 20000){
        scan_time = 0;
        circle_cnt ++;
    }
    return 1;
}


uint8_t touch_reporting_end(void){
    ptp_reports.finger_rpt[0].tip = 0;
    ptp_reports.finger_rpt[0].confidence = 1;
    ptp_reports.button = 0;
    ptp_reports.button1 = 0;
    ptp_reports.button2 = 0;
    ptp_reports.contactCnt = 0;
    ptp_reports.scantime_l8= (uint8_t)(scan_time&0x00ff);
    ptp_reports.scantime_m8= (uint8_t)((scan_time&0xff00)>>8);
    send_touch_data();
    scan_time = 0;
    return 0;
}



void test_touch(void){
    // 初始化触摸板数据
     ptp_reports.scantime_l8 = 0;
     ptp_reports.scantime_m8 = 0;
     ptp_reports.contactCnt = 0;
     ptp_reports.button = 0;
     ptp_reports.button1 = 0;
     ptp_reports.button2 = 0;

 //    循环150次
     for (int i = 0; i < 0xffff; i++) {
         // 模拟触摸事件
         if (circle_cnt  < 10){
             touch_auto_reporting();
         } else {
             break;
         }
         // 延时15ms
     }

     // 模拟触摸结束
    touch_reporting_end();


    // 打印测试结果
    printf("Touch test completed.\n");
}