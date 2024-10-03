/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   EMBENET NODE Port Interface
@brief     AES-128 interface for the EMBENET NODE Port
*/

#ifndef EMBENET_NODE_PORT_INTERFACE_EMBENET_AES128_H_
#define EMBENET_NODE_PORT_INTERFACE_EMBENET_AES128_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup embenet_node_port_aes AES-128 Interface
 *
 * This is interface to the 128-bit version of the Advanced Encryption Standard (AES) algorithm used by the embeNET Node.
 * This interface was declared to allow various implementations of the AES-128 algorithm, including hardware-accelerated support.
 * @{
 */

/**
 * @brief Initializes the AES-128 ciphering algorithm.
 *
 * The function will be called by the network stack BEFORE any other AES API functions will be invoked, allowing initialization of necessary resources (e.g. hardware accelerator or LUTs)
 */
void EMBENET_AES128_Init(void);

/**
 * @brief Deinitializes the AES-128 ciphering algorithm.
 *
 * This function is only called when the stack is being deinitialized.
 */
void EMBENET_AES128_Deinit(void);

/**
 * @brief Sets the AES-128 key to be used in further encryption and decryption operations
 *
 * The key set by this function should be used during the subsequent calls to @ref EMBENET_AES128_Encrypt and @ref EMBENET_AES128_Decrypt
 *
 * @param[in]        key 16 Bytes long secret key
 */
void EMBENET_AES128_SetKey(uint8_t const key[16U]);

/**
 * @brief Encrypts a 16 byte data chunk using AES-128 algorithm
 *
 * This function should encrypt the given 16 bytes of data overwriting the original plaintext data with the ciphered representation.
 *
 * @param[in,out] data 16 byte long plaintext data to be encrypted.
 *
 * @note The function shall overwrite the plaintext data in place
 */
void EMBENET_AES128_Encrypt(uint8_t data[16U]);

/**
 * @brief Decrypts a 16 byte data chunk using AES-128 algorithm
 *
 * This function should decrypt the given 16 bytes of data overwriting the original ciphered data with the plaintext representation.
 *
 * @param[in,out] data 16 byte long ciphered data to be decrypted.
 *
 * @note The function WILL overwrite encrypted data in place.
 */
void EMBENET_AES128_Decrypt(uint8_t data[16U]);

/** @} */

#ifdef __cplusplus
}
#endif

#endif // EMBENET_NODE_PORT_AES128_H_
