#ifndef _STORAGE1_H_
#define _STORAGE1_H_

#include <stdint.h>


#define CFG_BACKLIGHT_RGB_CONFIG 700


//写入存储
void Storage_write(void);
//读取存储内容
void Storage_read(void);
// 重置存储
void Storage_reset(void);

#endif //_STORAGE_H_
