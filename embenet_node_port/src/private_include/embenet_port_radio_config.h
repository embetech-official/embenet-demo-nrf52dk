/**
 @file
 @license   Commercial
 @copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
 @version   1.1.4417
 @purpose   Board specific definitions.
 @brief     Configures Radio.

 */

#ifndef EMBENET_PORT_RADIO_CONFIG_H_
#define EMBENET_PORT_RADIO_CONFIG_H_

#include <inttypes.h>
#include <string.h>


//###################################################################################################
// RADIO CAPABILITIES
//###################################################################################################

// RADIO_TxNow -> TX ISR 85us, (45us of radio preparing and power ramping, 40us of preamble and address field transmission)

// IDLE current 5uA
// TX current 17,5mA
// RX curretn 13mA
// LISTEN current 13mA
// preparation energy tx 2nAh (8mA przez 450us + 600us)
// preparation energy rx 500pAh (7,8mA przez 560us)
// preparation energy tx no frame 750pAh (7,8mA przez 330us)

#define EMBENET_RADIO_IdleToTxReady   (400)
#define EMBENET_RADIO_IdleToRxReady   (400)
#define EMBENET_RADIO_ActiveToTxReady (20)
#define EMBENET_RADIO_ActiveToRxReady (20)
#define EMBENET_RADIO_TxDelay         (45)
#define EMBENET_RADIO_RxDelay         EMBENET_RADIO_TxDelay // between GO signal and start listening - cannot measure but could be the same as delayTx


#define EMBENET_RADIO_TX_TX_START_CORRECTION (40) // time difference between start of frame ISR and actual first bit being sent by radio
#define EMBENET_RADIO_TX_RX_END_CORRECTION   (10) // time difference between end of frame ISR on receiver and transmitter side (receiver - sender)
#define EMBENET_RADIO_TX_RX_START_CORRECTION (50) // additional 10us takes the transceiver to trigger start of frame ISR on receiver side

//###################################################################################################
// PHY SETTINGS
//###################################################################################################
#ifndef EMBENET_RADIO_MAX_OUTPUT_POWER_REDUCTION
#    define EMBENET_RADIO_MAX_OUTPUT_POWER_REDUCTION 0
#endif
#define EMBENET_RADIO_SENSITIVITY      (-100) /**< [dBm], does not consider as neighbor if RSSI will be lower */
#define EMBENET_RADIO_MIN_CHANNEL      0
#define EMBENET_RADIO_MAX_CHANNEL      39
#define EMBENET_RADIO_CHANNEL_COUNT    40
#define EMBENET_RADIO_MIN_OUTPUT_POWER (-40)
#define EMBENET_RADIO_MAX_OUTPUT_POWER (4)
#define EMBENET_RADIO_POWER_OFFSET_LP  2
#define EMBENET_RADIO_POWER_OFFSET_HP  2
#define EMBENET_RADIO_POWER_OFFSET     2

#endif /* EMBENET_PORT_RADIO_CONFIG_H_ */
