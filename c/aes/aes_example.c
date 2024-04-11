//
// Created by Hank on 2024/3/14.
//

#include <malloc.h>
#include <memory.h>
#include "aes_example.h"
#include "aes.h"
#include <stdio.h>
#include <stdint.h>
#include "AES128.h"
#include "crc16.h"

#define len_DATA 16

int asc_test() {
    unsigned char IN_DATA[16] = {0x12, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
                                 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};//待加密数据
    unsigned char *pInputBuf = (unsigned char *) malloc(len_DATA);//存放加密数据，len_DATA是待加密数据长度
    unsigned char *pOutputBuf = (unsigned char *) malloc(len_DATA);//存放解密数据，len_DATA是待解密数据长度
    unsigned char Key[32] = {
            223, 22, 220, 201, 152, 26, 130, 7, 167, 62, 199,
            255, 137, 237, 43, 136, 123, 97, 230, 63, 224,
            16, 54, 140, 129, 61, 178, 160, 238, 189, 18, 176}; // 秘钥
    unsigned char chainBlock[16] = {0};
    memcpy(AES_Key_Table, Key, 32);

    aesEncInit();
//    加密
    for (int i = 0; i < len_DATA / 16; i++) {
        memcpy(&pInputBuf[16 * i], &IN_DATA[16 * i], 16);
        aesEncrypt(&pInputBuf[16 * i], chainBlock);
    }

    for (int i = 0; i < sizeof(chainBlock); i++) {
        printf("0x%X,", chainBlock[i]);
    }

    printf("\n");
//    -------------------------***********-------------------------
    aesDecInit();
    unsigned char encrypt_data[] = {0xB, 0x5C, 0x52, 0x2F, 0x50, 0xEC, 0xA8, 0x42, 0x4,
                                    0x19, 0xB4, 0x7, 0x6D, 0xFA, 0x33, 0xA3};
    unsigned char chainCipherBlock[16] = {0};
    //    解密
    for (int i = 0; i < (len_DATA / 16); i++) {
        memcpy(&pOutputBuf[16 * i], &encrypt_data[16 * i], 16);
        aesDecrypt(&pOutputBuf[16 * i], chainCipherBlock);//AES解密
    }

    for (int i = 0; i < sizeof(chainCipherBlock); i++) {
        printf("0x%X,", chainCipherBlock[i]);
    }
    printf("\n");
}

uint32_t rtc = 0;

/// @brief 加密数据打包
/// @param command_code 命令码
/// @param error_code 错误码
/// @param number 秘钥ID
/// @param randomNumber 随机数
/// @param encrypt_buf 加密数据
/// @param encrypt_len 加密包长度
/// @param data_len 数据包长度
/// @return 表示打包是否成功
uint8_t Send_EncryptedData(uint8_t command_code, uint8_t error_code, uint8_t number, uint32_t randomNumber,
                           uint8_t *encrypt_buf, uint8_t encrypt_len,uint8_t data_len) {
    uint8_t buf[64], buf_len = 0, i = 0;
    uint16_t crc = 0;

    crc = GetQuickCRC16(encrypt_buf, encrypt_len);//计算CRC
    printf("crc:%d\n", crc);
    buf_len = 0;

    buf[buf_len++] = command_code;//命令码
    buf[buf_len++] = error_code;//命令码

    buf[buf_len++] = data_len;//长度

    buf[buf_len++] = HI_UINT16(crc);//CRC16
    buf[buf_len++] = LO_UINT16(crc);

    buf[buf_len++] = number;//秘钥ID

    buf[buf_len++] = UINT32_BYTE3(randomNumber);//随机数
    buf[buf_len++] = UINT32_BYTE2(randomNumber);
    buf[buf_len++] = UINT32_BYTE1(randomNumber);
    buf[buf_len++] = UINT32_BYTE0(randomNumber);

    //复制加密数据
    for (i = 0; i < encrypt_len; i++) {
        buf[buf_len++] = encrypt_buf[i];
    }
    printf("\r\nSend_EncryptedData :{");
    for (i = 0; i < buf_len; i++) {
        printf("0x%02x,", buf[i]);
    }
    printf("}\r\n");

//    while ( ble_tx_data(INATECK_OTA_READ_WRITE_IDX,buf_len, buf) == 0)
//        ;
    return 1;
}


/// @brief 消息加密
/// @param output 输出的数据
/// @param output_len 输出的数据长度
/// @param input 输入的数据
/// @param input_len 输入的数据长度
/// @param number 秘钥ID
/// @param randomNumber 随机数
/// @return 加密后的长度，为0时表示加密失败
uint8_t _OTA_Message_Encrypt(uint8_t *output, uint8_t output_len, uint8_t *input, uint8_t input_len, uint8_t number,
                             uint32_t randomNumber) {
    uint8_t buf[64] = {0};
    uint8_t buf_len = input_len;

    if (input_len == 0) {
        return 0;
    }

    buf_len = buf_len + 4;
    // len 不足16的倍数，补长度
    if (buf_len % 16 != 0) {
        buf_len = buf_len + (16 - buf_len % 16);
    }
    printf("\nbuf_len:%d\n", buf_len);
    if (output_len < buf_len) {
        return 0;
    }

    memcpy(buf, &rtc, 4);
    memcpy(buf + 4, input, input_len);
    printf("--------------------buf-------------------\n");
    for (int i = 0; i < buf_len; i++) {
        printf("0x%02x,", buf[i]);
    }
    printf("\nbuf_len:%d\n",buf_len);
    AES_CBC_encrypt(output, buf, buf_len, randomNumber, number);

    return buf_len;
}


/// @brief 设备加密认证
/// @param buf 写入的数据
/// @param len 写入的数据长度
/// @return 数据的长度
static uint8_t _OTA_AUTH(uint8_t *auth_buf, uint8_t auth_len) {
    uint8_t buf[64] = {0};
    uint8_t number = 0; // 随机数 用于选择密钥对
    uint32_t randomNumber = 0x123456789; // 随机数

    uint8_t encrypt_len = _OTA_Message_Encrypt(buf, 64, auth_buf, auth_len, number, randomNumber);
    printf("encrypt_len:%d\n", encrypt_len);
    Send_EncryptedData(0x2C + 1, 0, number, randomNumber, buf, encrypt_len,auth_len + 4);
    return 1;
}



