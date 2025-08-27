//
// Created by hank on 2025/8/5.
//

#ifndef C_TOUCH_H
#define C_TOUCH_H

#include <stdint.h>

#define TOUCH_SLAVE_ID 								0x33 // 触摸板从机地址
#define INT_GPIO            						22 // 中断GPIO
#define	PTP_CERT_STATUS_SEGMENT_LEN					257
#define	TP_MAX_FINGER_COUNT							4 // 最大总触摸点数
#define	TP_MAX_CONTACT_COUNT						6 // 报告的最大contact 大小

#define BAYES_DEBOUNCE_CYCLE						2 // 贝叶斯防抖周期

#define TOUCHPAD_MAX_X 								2459 //0x800
#define TOUCHPAD_MAX_Y								1380 // 0x450

#pragma pack(push)
#pragma pack(1)
typedef struct hid_finger_data {
    uint8_t confidence : 1;
    uint8_t tip_switch : 1;
    uint8_t reserved   : 2;
    uint8_t contact_id : 4;
    uint8_t x_l8:8;
    uint8_t x_m4:4;
    uint8_t y_l4:4;
    uint8_t y_m8:8;
} hid_finger_data_t;

typedef struct PTP_touch_pad_data {
    uint8_t	Report_ID;
    hid_finger_data_t fingers[TP_MAX_FINGER_COUNT];
    uint16_t scan_time;
    uint8_t contact_count;
    uint8_t button   : 1;
    uint8_t reserved : 7;
} PTP_touch_pad_data_t;

typedef struct HIDReport
{
    uint8_t button;
    uint8_t x_l8:8;
    uint8_t x_m4:4;
    uint8_t y_l4:4;
    uint8_t y_m8:8;
    uint8_t wheel;
    uint8_t twheel;
}HID_Report;

typedef struct FingerReport
{
    uint8_t tip:1; // 是否离开表面开关
    uint8_t confidence:1; // 置信度
    uint8_t contactID:6; // 接触ID
    uint8_t x_l8;	// X坐标低8位
    uint8_t x_m4:4; // X坐标高4位
    uint8_t y_l4:4;	 //	Y坐标低4位
    uint8_t y_m8; //	Y坐标高8位
}Finger_Report;

typedef struct PTPReport
{
    uint8_t scantime_l8;
    uint8_t scantime_m8;
    uint8_t button:1;
    uint8_t button1:1;
    uint8_t button2:1;
    uint8_t Reserved:5;
    Finger_Report finger_rpt[TP_MAX_FINGER_COUNT];
    uint8_t contactCnt;
}PTP_Report;

typedef struct _CONTACT {
    uint8_t contact_id;
    uint8_t tip: 1;
    uint8_t confidence: 1;
    uint8_t : 6;
    uint16_t x;
    uint16_t y;
    uint16_t size;
} CONTACT, *CCONTACT;

// typedef struct _CONTACT_ATTR {
//     uint16_t size; // 大小
//     // uint16_t soi; // 物体强度
//     // uint8_t width; //  宽度
//     // uint8_t height; //  高度
//     // uint16_t force; // 物体的最⼤接触⼒
// } CONTACT_ATTR, *CCONTACT_ATTR;

typedef struct _TOUCHPAD_EVENT {
    uint8_t contact_count;// 手指总数    1
    CONTACT contacts[TP_MAX_FINGER_COUNT]; // 手指 x y 信息 6*4 = 24
    // CONTACT_ATTR contact_attrs[TP_MAX_FINGER_COUNT]; // 手指强度信息  8*4 = 32
    // uint8_t :3;
    // uint8_t button_status:1; // DRV0/BTN 状态指⽰位 0
    // uint8_t :4;
    // uint8_t gesture_type;//GESTURE_0_TYPE
} TOUCHPAD_EVENT, *TTOUCHPAD_EVENT;


typedef struct _OTHER_TOUCH_DATA {
    uint8_t is_new; // 是否新数据
    // int small_count; // 小触点连续保持次数 需要超过22，size为4，
    int valid; // 当前是否有效 0:默认   1：有效  -1：无效
    int confidence;  // 0:默认   1：有效  -1：无效
    // int valid_key_count; // 记录防误触增强后有效按键次数，（生命周期为按键后 一段时间内）
    // uint16_t total_count; // 连续触摸次数
    // uint16_t distance; // 两手指的距离
    // uint16_t size; // 触摸大小
    // uint8_t index;// 在数组的索引位置
    // int edge_valid;// 是否在边缘  0:默认   1：有效  -1：无效
    int distance_valid;// 默认距离是否有效 0:默认   1：有效  -1：无效
    uint8_t distance_valid_count;// 距离 连续有效次数
} OTHER_TOUCH_DATA,*OOTHER_TOUCH_DATA;

#pragma pack(pop)

void test_touch(void);

#endif //C_TOUCH_H
