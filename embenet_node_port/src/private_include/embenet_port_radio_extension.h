/**
 @file
 @license   Commercial
 @copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
 @version   1.1.4417
 @purpose   Radio extensions functions.
 @brief     Additional radio funcitonalities or configuration options.

 */

#ifndef EMBENET_PORT_RADIO_EXTENSION_H_
#define EMBENET_PORT_RADIO_EXTENSION_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Locks HFXO. Radio will not turn it off when going to idle state. Enables HFXO if it is not enabled.
 * @note If it happens that HFXO is not enabled, this function can block for ~300us while waiting for HFXO to start.
 */
void EMBENET_RADIO_LockHfxo(void);

/**
 * @brief Unlocks HFXO. Radio will turn the HFXO off when going idle.
 * @note This function does not disable HFXO.
 */
void EMBENET_RADIO_UnlockHfxo(void);

#ifdef __cplusplus
}
#endif

#endif /* EMBENET_PORT_RADIO_EXTENSION_H_ */
