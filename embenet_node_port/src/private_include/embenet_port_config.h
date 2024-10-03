/**
 @file
 @license   Commercial
 @copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
 @version   1.1.4417
 @purpose   Board specific definitions.
 @brief     Configures BSP modules.

 */

#ifndef EMBENET_PORT_CONFIG_H_
#define EMBENET_PORT_CONFIG_H_

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wcast-align"
#include "nrf52.h"
#pragma GCC diagnostic pop

#include <inttypes.h>

// ###################################################################################################
//  BRT
// ###################################################################################################

#define EMBENET_BRT_MAX_FRAME_SIZE 200 // EMBENET_BRT_MAX_DATA_SIZE + HDLC markings + CRC

// ###################################################################################################
//  Timer
// ###################################################################################################

#define EMBENET_PORT_TIMER             NRF_RTC1
#define EMBENET_PORT_TIMER_IRQn        RTC1_IRQn
#define EMBENET_PORT_TIMER_IRQ_HANDLER RTC1_IRQHandler

#endif // EMBENET_PORT_CONFIG_H_ included
