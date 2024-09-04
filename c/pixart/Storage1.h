#ifndef _STORAGE1_H_
#define _STORAGE1_H_

#include <stdint.h>


#define CFG_BACKLIGHT_RGB_CONFIG 700

typedef enum {
    Fn_Lock = 0,
    Backlight_Color,
    Backlight_Brightness,
    Touch_Flag,
    System_Type,
    Device_Index,
    STORAGE_END,
    IDX_END,
} STORAGE_IDX;

/* EasyFlash error code */
typedef enum {
    EF_NO_ERR,
    EF_ERASE_ERR,
    EF_READ_ERR,
    EF_WRITE_ERR,
    EF_ENV_NAME_ERR,
    EF_ENV_NAME_EXIST,
    EF_ENV_FULL,
    EF_ENV_INIT_FAILED,
} EfErrCode;

/**
  * @brief  设置值
  * @param  STORAGE_IDX storage_idx: 存储索引
  * @param  const uint8_t *buf: 存储缓冲
  * @param  size_t size: 存储大小
  * @return EfErrCode:错误码
  */
EfErrCode Storage_Set(STORAGE_IDX storage_idx, const uint8_t *buf, size_t size);
/**
  * @brief  设置读取值
  * @param  STORAGE_IDX storage_idx: 存储索引
  * @param  uint8_t *buf: 读取缓冲区
  * @param  size_t size: 读取长度
  * @return EfErrCode:错误码
  */
EfErrCode Storage_Get(STORAGE_IDX storage_idx, uint8_t *buf, size_t size);


void Storage_Set_U8(STORAGE_IDX storage_idx, uint8_t val);

uint8_t Storage_Get_U8(STORAGE_IDX storage_idx);

//写入存储
void Storage_write(void);
//读取存储内容
void Storage_read(void);
// 重置存储
void Storage_reset(void);

#endif //_STORAGE_H_
