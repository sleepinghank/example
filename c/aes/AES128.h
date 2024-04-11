#ifndef __AES128_H__
#define __AES128_H__

#include <stdint.h>


#define AES128 1
//#define AES192 1
//#define AES256 1

//---------------------------------------------------------快捷操作---------------------------------------------------------//

#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)
#define HI_UINT8(a)  (((a) >> 4) & 0x0F)
#define LO_UINT8(a)  ((a) & 0x0F)

#define BUILD_UINT16(loByte, hiByte) ((uint16_t)(((loByte) & 0x00FF)|(((hiByte) & 0x00FF)<<8)))

#define BUILD_UINT32(Byte0, Byte1, Byte2, Byte3)   ((uint32_t)(((uint32_t)(Byte0) & 0x00FF) + (((uint32_t)(Byte1) & 0x00FF) << 8) \
                                                  + (((uint32_t)(Byte2) & 0x00FF) << 16) + (((uint32_t)(Byte3) & 0x00FF) << 24)))

#define UINT32_BYTE0(dwTemp) ((uint8_t)( dwTemp        & 0x000000ff )) /*!< uint32_t 数据拆分 byte0 */
#define UINT32_BYTE1(dwTemp) ((uint8_t)((dwTemp >> 8)  & 0x000000ff )) /*!< uint32_t 数据拆分 byte1 */
#define UINT32_BYTE2(dwTemp) ((uint8_t)((dwTemp >> 16) & 0x000000ff )) /*!< uint32_t 数据拆分 byte2 */
#define UINT32_BYTE3(dwTemp) ((uint8_t)((dwTemp >> 24) & 0x000000ff )) /*!< uint32_t 数据拆分 byte3 */

//------------------------------------------------------------END------------------------------------------------------------//


/// @brief AES秘钥初始化（通过设备地址以及设备信息计算新的秘钥跟iv）
/// @param *addr mac地址
/// @param productModelCode 产品型号code
/// @param chipModelCode 芯片型号code
/// @return 成功与否
uint8_t AES_Keyinitialization(uint8_t *addr,uint32_t productModelCode, uint32_t chipModelCode);
/// @brief AES加密
/// @param output 输出加密后的加密数据
/// @param input    输入加密前的原始数据
/// @param length   数据长度
/// @param randomNumber 随机数 用于秘钥加盐
/// @param number 随机数 用于选择密钥对
void AES_CBC_encrypt(uint8_t* output, uint8_t* input, uint32_t length, uint32_t randomNumber, uint8_t number);
/// @brief AES解密
/// @param output 输出解密后的数据
/// @param input    输入加密后的加密数据
/// @param length   数据长度
/// @param randomNumber 随机数 用于秘钥加盐
/// @param number 随机数 用于选择密钥对
void AES_CBC_decrypt(uint8_t* output, uint8_t* input, uint32_t length, uint32_t randomNumber, uint8_t number);

#endif
