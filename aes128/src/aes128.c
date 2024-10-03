/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   AES-128 encryption and decryption
@brief     Utility wrapper for the actual AES-128 library
*/

#include "aes128.h"

#include "aes.h"

#include <string.h>

#define CBC 1
#define CTR 1
#define ECB 1

/** Structure describing the AES-128 encoder and decoder */
typedef struct {
    /// mode of operation
    AES128_Mode mode;
    /// actual cipher
    struct AES_ctx ctx;
    /// key
    uint8_t key[AES128_MAX_KEY_SIZE];
    /// key size
    uint8_t keySize;
    /// initialization vector
    uint8_t iv[AES128_MAX_KEY_SIZE];
} AES128_Descriptor;


// default key and iv used, if user didn't passed other one
#define DEFAULT_KEY \
    { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c }
#define DEFAULT_IV \
    { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f }

static AES128_Descriptor aes128Desc;

static bool AES128_Init(AES128_Mode mode, uint8_t const* key, uint8_t keySize, uint8_t const* iv);

static bool AES128_Init(AES128_Mode mode, uint8_t const* key, uint8_t keySize, uint8_t const* iv) {
    bool result     = false;
    aes128Desc.mode = mode;
    if (keySize <= sizeof(aes128Desc.key)) {
        aes128Desc.keySize = keySize;
        switch (mode) {
            case MODE_CBC: {
                if (iv == NULL) {
                    uint8_t ivBuffer[] = DEFAULT_IV;
                    memcpy(aes128Desc.iv, ivBuffer, keySize);
                } else {
                    memcpy(aes128Desc.iv, iv, keySize);
                }

                if (key == NULL) {
                    uint8_t keyBuffer[] = DEFAULT_KEY;
                    memcpy(aes128Desc.key, keyBuffer, keySize);
                } else {
                    memcpy(aes128Desc.key, key, keySize);
                }

                AES_init_ctx_iv(&aes128Desc.ctx, aes128Desc.key, aes128Desc.iv);
                result = true;
            } break;
            case MODE_ECB:
                (void)iv;

                if (key == NULL) {
                    uint8_t keyBuffer[] = DEFAULT_KEY;
                    memcpy(aes128Desc.key, keyBuffer, keySize);
                } else {
                    memcpy(aes128Desc.key, key, keySize);
                }

                AES_init_ctx(&aes128Desc.ctx, aes128Desc.key);
                result = true;
                break;
            case MODE_CTR:
                if (iv == NULL) {
                    uint8_t ivBuffer[] = DEFAULT_IV;
                    memcpy(aes128Desc.iv, ivBuffer, keySize);
                } else {
                    memcpy(aes128Desc.iv, iv, keySize);
                }

                if (key == NULL) {
                    uint8_t keyBuffer[] = DEFAULT_KEY;
                    memcpy(aes128Desc.key, keyBuffer, keySize);
                } else {
                    memcpy(aes128Desc.key, key, keySize);
                }

                AES_init_ctx_iv(&aes128Desc.ctx, aes128Desc.key, aes128Desc.iv);
                result = true;
                break;
            default: break;
        }
    }
    return result;
}

bool AES128_Encrypt(AES128_Mode mode, uint8_t const* key, uint8_t keySize, uint8_t const* iv, uint8_t* data, uint16_t length) {
    bool result = false;
    AES128_Init(mode, key, keySize, iv);
    if (length % AES_BLOCKLEN != 0) {
        return false;
    }
    if (mode == aes128Desc.mode && data != NULL) {
        switch (mode) {
            case MODE_CBC: {
                AES_CBC_encrypt_buffer(&aes128Desc.ctx, data, length);
                result = true;
            } break;
            case MODE_ECB: {
                size_t bytesElapsed = length;
                size_t offset       = 0;
                while (bytesElapsed) {
                    AES_ECB_encrypt(&aes128Desc.ctx, data + offset);
                    offset += AES_BLOCKLEN;
                    bytesElapsed -= AES_BLOCKLEN;
                }
                result = true;
            } break;
            case MODE_CTR: {
                AES_CTR_xcrypt_buffer(&aes128Desc.ctx, data, length);
                result = true;
            } break;
            default: break;
        }
    }
    return result;
}

bool AES128_Decrypt(AES128_Mode mode, uint8_t const* key, uint8_t keySize, uint8_t const* iv, uint8_t* data, uint16_t length) {
    bool result = false;
    AES128_Init(mode, key, keySize, iv);
    if (length % AES_BLOCKLEN != 0) {
        return false;
    }
    if (mode == aes128Desc.mode && data != NULL) {
        switch (mode) {
            case MODE_CBC: {
                AES_CBC_decrypt_buffer(&aes128Desc.ctx, data, length);
                result = true;
            } break;
            case MODE_ECB: {
                size_t bytesElapsed = length;
                size_t offset       = 0;
                while (bytesElapsed) {
                    AES_ECB_decrypt(&aes128Desc.ctx, data + offset);
                    offset += AES_BLOCKLEN;
                    bytesElapsed -= AES_BLOCKLEN;
                }
                result = true;
            } break;
            case MODE_CTR: {
                AES_CTR_xcrypt_buffer(&aes128Desc.ctx, data, length);
                result = true;
            } break;
            default: break;
        }
    }
    return result;
}
