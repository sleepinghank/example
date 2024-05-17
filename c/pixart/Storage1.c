#include "Storage1.h"
#include <stdio.h>
#include <string.h>
//#define CONFIG_ENABLE_TOUCH 1;
#define FN_LOCK 1;

#ifdef CONFIG_ENABLE_TOUCH
 uint8_t touch_flag;
#endif
uint8_t backlight_color = 3;//颜色
uint8_t backlight_brightness = 43;//亮度
 uint8_t _device_idx = 0;//通道
uint8_t SystemType[3];//系统类型
uint8_t Storage_buffer[7] = {0xff};
uint8_t FN_Lock_flag= 1;
#ifdef CONFIG_ENABLE_TOUCH
uint8_t touch_en = 1;
#endif

uint8_t temp[11] = {0};

uint8_t Reset_flag =1;
//写入存储
void Storage_write(void)
{
    uint8_t buf[11] = {0};  // 将数组大小增加到11
    uint8_t len = 0;
    uint8_t err = 0;
    uint8_t i = 0,sum = 0;
  //  if(_device_idx != 0 && _device_idx != 1 && _device_idx != 2){
      //  _device_idx = 0;
  //  }
 //   if((Storage_buffer[0] != backlight_color) || (Storage_buffer[1] != backlight_brightness) 
//#ifdef CONFIG_ENABLE_TOUCH
//    || (Storage_buffer[2] != touch_flag)
//#endif
      //  || (Storage_buffer[3] != SystemType[0]) || (Storage_buffer[4] != SystemType[1])|| (Storage_buffer[5] != SystemType[2]) || (Storage_buffer[6] != _device_idx)||(Reset_flag == 1) )  // 检查_device_idx是否改变
  //  {
        
        printf("write --backlight_color = %d backlight_brightness = %d \r\n", backlight_color, backlight_brightness);
        #ifdef CONFIG_ENABLE_TOUCH
        printf("touch_flag = %x  touch_en = %x \r\n", 1,touch_en);
        #endif
        printf("SystemType= [%x][%x][%x]\r\n", SystemType[0],SystemType[1],SystemType[2]);

        buf[len++] = 0x5a;
        buf[len++] = 0;
        buf[len++] = backlight_color;
        buf[len++] = backlight_brightness;
#ifdef CONFIG_ENABLE_TOUCH
        buf[len++] = 1;
#endif
        buf[len++] = SystemType[0];
        buf[len++] = SystemType[1];
        buf[len++] = SystemType[2];
        buf[len++] = _device_idx;  // 将_device_idx写入到buf数组
        #ifdef FN_LOCK
        buf[len++] = FN_Lock_flag;
        #endif
    printf("len = %d\r\n", len);
        for ( i = 2; i < len; i++)
        {
            sum += buf[i];  // 计算校验和
            printf("buf[%d] = %x\r\n", i, buf[i]);
        }

        buf[1] = len;
        buf[len++] = sum;
//    printf("len = %d\r\n", buf[1]);
        // buf[8] = buf[0] + buf[1] + buf[2] + buf[3]+ buf[4] + buf[5] + buf[6] + buf[7];  // 更新校验和
        printf("buf [%d] = {",len);
        for ( i = 0; i < len; i++)
        {
            printf("%02X ",buf[i]);
        }
         printf("}\r\n");
    
//       err = pxi_gap_s_profile_data_write(CFG_BACKLIGHT_RGB_CONFIG,10, &buf[0]);  // 更新写入的数据长度
        memcpy(temp, buf, 11);
        printf("err = %d\r\n", err);
        printf("sun = %d\r\n",  sum);
//         Storage_buffer[0] = backlight_color;
//         Storage_buffer[1] = backlight_brightness;
// #ifdef CONFIG_ENABLE_TOUCH
//         Storage_buffer[2] = touch_flag;
// #endif
//         Storage_buffer[3] = SystemType[0];
//         Storage_buffer[4] = SystemType[1];
//         Storage_buffer[5] = SystemType[2];
//         Storage_buffer[6] = _device_idx;  // 将_device_idx写入到Storage_buffer数组

        Reset_flag = 0;
 //   }
    printf("FN_Lock_flag = %d\r\n", FN_Lock_flag);
}


//读取存储内容
void Storage_read(void)
{
    uint8_t buf[15] = {0};  // 将数组大小增加到11
    uint8_t sum = 0,len,i;
    uint8_t *p_buf;
    memcpy(buf, temp, 11);
//    pxi_gap_s_profile_data_read(CFG_BACKLIGHT_RGB_CONFIG, 14, &buf[0]);  // 更新读取的数据长度

        printf("Storage_read [%d] = {",buf[1]);
        for ( i = 0; i < 14; i++)
        {
            printf("%02X ",buf[i]);
        }
    printf("}\r\n");

    p_buf = &buf[0];

    if (*p_buf != 0x5a)
    {
        printf("head error !!!\r\n");
        return;
    }
p_buf++;

   len =  *p_buf;
    printf("len = %d\r\n", len);
   p_buf++;

    for ( i = 0; i < len -2; i++)
    {
        sum += p_buf[i];  // 计算校验和
        printf("p_buf[%d] = %x\r\n", i, p_buf[i]);
    }


printf("sum = %x / %x\r\n",  sum, buf[len]);
     if (sum != buf[len])
     {

         printf("Verification error !!!   \r\n");
        
        backlight_color =  0;
        backlight_brightness = 76;
        #ifdef CONFIG_ENABLE_TOUCH
        touch_flag = 1;
        #endif
        SystemType[0] = 0;
        SystemType[1] = 0;
        SystemType[2] = 0;
        FN_Lock_flag = 1;
        _device_idx = 0;
        return;
     }
    

    backlight_color = *p_buf;
    p_buf++;

    backlight_brightness = *p_buf;
    p_buf++;

#ifdef CONFIG_ENABLE_TOUCH
    touch_flag = *p_buf;
    p_buf++;
#endif

    SystemType[0] = *p_buf;
    p_buf++;

    SystemType[1] = *p_buf;
    p_buf++;
    SystemType[2] = *p_buf;
    p_buf++;
    _device_idx = *p_buf;  // 从buf数组读取_device_idx
    p_buf++;

    #ifdef FN_LOCK
    FN_Lock_flag = *p_buf;
    p_buf++;
    #endif


    printf("read --backlight_color = %d backlight_brightness = %d \r\n", backlight_color, backlight_brightness);
    #ifdef CONFIG_ENABLE_TOUCH
        printf("touch_flag = %x  touch_en = %x\r\n", touch_flag,touch_en);
        #endif
    printf("SystemType= [%x][%x][%x]\r\n", SystemType[0],SystemType[1],SystemType[2]);
    printf("read --_device_idx = %d \r\n", _device_idx);
    printf("FN_Lock_flag = %d\r\n", FN_Lock_flag);
}
