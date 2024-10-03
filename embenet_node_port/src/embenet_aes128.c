/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET Node port for nRF52832
@brief     Implementation of the AES128 encoding/decoding
 */

#include "embenet_aes128.h"

#include <embetech/aes128.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wconversion"
#include "nrf52.h"
#include "nrf_ecb.h"
#pragma GCC diagnostic pop

#include <stdint.h>
#include <string.h>

// static CRYP_HandleTypeDef hcryp;
static uint8_t _key[16];

void EMBENET_AES128_Init(void) {
    nrf_ecb_init();
}

void EMBENET_AES128_Deinit(void) {
    // do nothing
    nrf_ecb_task_trigger(NRF_ECB, NRF_ECB_TASK_STOPECB);
}

void EMBENET_AES128_SetKey(uint8_t const key[16U]) {
    nrf_ecb_set_key(key);
    memcpy(_key, key, 16);
}

void EMBENET_AES128_Encrypt(uint8_t data[16U]) {
    uint8_t tmp[16];
    nrf_ecb_crypt(tmp, data);
    memcpy(data, tmp, 16);
}

void EMBENET_AES128_Decrypt(uint8_t data[16U]) {
    AES128_Decrypt(MODE_ECB, _key, 16, NULL, data, 16);
}
