//
// Created by Hank on 2024/3/14.
//

#ifndef C_AES_EXAMPLE_H
#define C_AES_EXAMPLE_H
#include <stdint.h>
#define _PRODUCT_KB01104_DE    (0x00100020)
#define _CHIP_PAR2860      (0xC0000001)
#define  ProductModelCode    (_PRODUCT_KB01104_DE)  //产品型号代号

#define  ChipModelCode       (_CHIP_PAR2860)     //芯片型号代号

int asc_test();
uint8_t _OTA_Message_Encrypt(uint8_t* output, uint8_t output_len,uint8_t* input, uint8_t input_len,uint8_t number,uint32_t randomNumber);
uint8_t Send_EncryptedData(uint8_t command_code, uint8_t error_code, uint8_t number, uint32_t randomNumber,
                           uint8_t *encrypt_buf, uint8_t encrypt_len,uint8_t data_len);
#endif //C_AES_EXAMPLE_H
