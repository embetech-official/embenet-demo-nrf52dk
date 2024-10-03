/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   AES-128 encryption and decryption
@brief     Utility wrapper for the actual AES-128 library
*/

#ifndef AES128_AES128_H_
#define AES128_AES128_H_

#include <stdbool.h>
#include <stdint.h>

/** @defgroup aes128 AES-128 encoding and decoding library
 *
 * This is a wrapper for the Tiny AES library (public domain code)
 * https://github.com/kokke/tiny-AES-c
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/// AES-128 key size (in number of bytes)
#define AES128_MAX_KEY_SIZE 16

/** Possible AES-128 modes of operation */
typedef enum {
    /// Cipher block chaining mode
    MODE_CBC = 0,
    /// Electronic codebook mode
    MODE_ECB = 1,
    /// Couter mode
    MODE_CTR = 2
} AES128_Mode;


/**
 * Encrypts given data using selected key, iv and encryption mode.
 *
 * @param mode - mode of encryption (CBC, ECB, CTR available)
 * @param key - address of the key used to encrypt data - may be null, default one is used then
 * @param keySize - length of the given key in bytes
 * @param iv - address of the initialization vector - may be null, default one is used then
 * @param data - address of data to encrypt - cannot be null
 * @param length - size of data in bytes
 */
bool AES128_Encrypt(AES128_Mode mode, uint8_t const* key, uint8_t keySize, uint8_t const* iv, uint8_t* data, uint16_t length);

/**
 * Decrypts given data using selected key, iv and decryption mode.
 *
 * @param mode - mode of decryption (CBC, ECB, CTR available)
 * @param key - address of the key used to decrypt data - may be null, default one is used then
 * @param keySize - length of the given key in bytes
 * @param iv - address of the initialization vector - may be null, default one is used then
 * @param data - address of data to decrypt - cannot be null
 * @param length - size of data in bytes
 */
bool AES128_Decrypt(AES128_Mode mode, uint8_t const* key, uint8_t keySize, uint8_t const* iv, uint8_t* data, uint16_t length);

#ifdef __cplusplus
}
#endif

/** @} */


#endif /* AES128_AES128_H_ */
