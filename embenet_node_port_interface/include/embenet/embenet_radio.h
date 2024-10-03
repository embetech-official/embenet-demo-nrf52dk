/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   RADIO Transceiver Application Interface (API)
The IDLE state is implementation defined. The assumption is to balance speed and energy consumption in cross-platform manner.
Modern transceivers may not differentiate between SLEEP and STANDBY modes (or consumed power difference is miniscule), so we can prioritize minimizing energy consumption by assuming
that RADIO IDLE state is SLEEP.
@note	   Radio states:

UNINITIALIZED	State just after hardware reset.
IDLE 			In this state radio may remain in low power mode.
TX_READY		Prepared for immediate transmission trigger.
RX_READY		Prepared for immediate listening trigger.
LISTEN			Radio listens for frames.
TX				Radio is transmitting frame.
RX				Radio is receiving frame.
ACTIVE			After TX or RX radio remains in state ready for fast transition to TX_READY or RX_READY.


HardwareReset 						-> UNINITIALIZED
UNINITIALIZED(EMBENET_RADIO_Init) 	-> IDLE
IDLE(EMBENET_TxEnable) 				-> TX_READY 		at idleToTxReady
IDLE(EMBENET_RxEnable) 				-> RX_READY 		at idleToRxReady
IDLE(EMBENET_RADIO_Idle)			-> IDLE
RX_READY(EMBENET_RADIO_RxNow)		-> LISTEN
RX_READY(EMBENET_RADIO_Idle)		-> IDLE
TX_READY(EMBENET_RADIO_TxNow)		-> TX 				at txDelay
TX_READY(EMBENET_RADIO_Idle)		-> IDLE
LISTEN(start of frame)				-> RX 				at rxDelay
LISTEN(EMBENET_RADIO_Idle)			-> IDLE
RX(end of frame)					-> ACTIVE
RX(EMBENET_RADIO_Idle)				-> IDLE
ACTIVE(EMBENET_TxEnable) 			-> TX_READY			at activeToTxReady
ACTIVE(EMBENET_RxEnable)			-> RX_READY 		at activeToRxReady
ACTIVE(EMBENET_RADIO_Idle)			-> IDLE
*/

// clang-format off
/**
                                                              UNINITIALIZED (after hardware reset)
												                    |
												                    | EMBENET_RADIO_Init
												                    |
												                   \|/
 (from any state except UNINITIALIZED) EMBENET_RADIO_Idle ------> IDLE
												                    |
											 	 	                |
					                    	   EMBENET_TxEnable     |     EMBENET_RxEnable
                    -------> TX_READY <--------------------------------------------------------> RX_READY <-----------------------------------------------------
	                |		  	  |  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  |                                                        |
	                |		  	  |	EMBENET_RADIO_TxNow												  | EMBENET_RADIO_RxNow                                    |
	                |		  	  | (calls EMBENET_RADIO_CaptureCbt on start of TX frame) 	  	  	  |                                                        |
   EMBENET_TxEnable |		  	 \|/  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	 \|/                                                       |
	                |			 TX                                                                 LISTEN                                                     |
	                |	  	 	  |  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  |                                                        |
	                |		  	  | (calls EMBENET_RADIO_CaptureCbt on end of frame) 	  	  	  	  | (calls EMBENET_RADIO_CaptureCbt on start of RX frame)  |
	                |		  	  |                                                                   |                                                        |
	                |		  	 \|/  	  	(calls EMBENET_RADIO_CaptureCbt on end of RX frame)    	 \|/                                                       |
                    -----------ACTIVE <------------------------------------------------------------- RX                                                        |
                                  |                                                                                                                            |
                                  |                                           EMBENET_RxEnable                                                                 |
                                  ------------------------------------------------------------------------------------------------------------------------------
**/
// clang-format on


#ifndef EMBENET_NODE_PORT_INTERFACE_EMBENET_RADIO_H_
#define EMBENET_NODE_PORT_INTERFACE_EMBENET_RADIO_H_


#include "embenet_timer.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup embenet_node_port_radio Radio Interface
 *
 * Provides interface to the radio transceiver
 *
 * @{
 */

typedef int8_t  EMBENET_RADIO_SetParamType; ///< radio implementation specific parameter type
typedef int8_t  EMBENET_RADIO_GetParamType; ///< radio implementation specific parameter type
typedef uint8_t EMBENET_RADIO_Channel;      ///< radio channel
typedef int8_t  EMBENET_RADIO_Power;        ///< radio power in dBm

///< radio timings structure, holds execution and radio configuration time values
typedef struct {
    EMBENET_TimeUs idleToTxReady;   ///< maximum time needed to become TX ready from idle state (EMBENET_RADIO_Idle() -> EMBENET_RADIO_TxReady())
    EMBENET_TimeUs idleToRxReady;   ///< maximum time needed to become RX ready from idle state (EMBENET_RADIO_Idle() -> EMBENET_RADIO_RxReady())
    EMBENET_TimeUs activeToTxReady; ///< maximum time needed to become TX ready from active state ('end of frame callback' -> EMBENET_RADIO_TxReady())
    EMBENET_TimeUs activeToRxReady; ///< maximum time needed to become RX ready from active state ('end of frame callback' -> EMBENET_RADIO_RxReady())
    EMBENET_TimeUs txDelay;         ///< maximum time needed from TX ready to appearance of radio signal on output port
    EMBENET_TimeUs rxDelay;         ///< maximum time needed to switch from RX ready to listening
    EMBENET_TimeUs txRxStartDelay;  ///< the time between the appearance of first bit of preamble and a call of startOfFrame callback

    EMBENET_RADIO_Power sensitivity;    ///< input power level below which PER significantly grows up eg. from 0% to 1% for packet length of 30B
    EMBENET_RADIO_Power maxOutputPower; ///< maximum output power radio can set
    EMBENET_RADIO_Power minOutputPower; ///< minimum output power radio can set
} EMBENET_RADIO_Capabilities;

///< Defines RADIO layer operation status
typedef enum {
    EMBENET_RADIO_STATUS_SUCCESS             = 0,
    EMBENET_RADIO_STATUS_GENERAL_ERROR       = -1,
    EMBENET_RADIO_STATUS_COMMUNICATION_ERROR = -2,
    EMBENET_RADIO_STATUS_WRONG_STATE         = -3,

    EMBENET_RADIO_STATUS_PARAMETER_NOT_IMPLEMENTED    = -30,
    EMBENET_RADIO_STATUS_PARAMETER_ARGS_WRONG_NUMBER  = -31,
    EMBENET_RADIO_STATUS_PARAMETER_ARG1_OUT_OF_BOUNDS = -32,
    EMBENET_RADIO_STATUS_PARAMETER_ARG2_OUT_OF_BOUNDS = -33,
    EMBENET_RADIO_STATUS_PARAMETER_ARG3_OUT_OF_BOUNDS = -34,
    EMBENET_RADIO_STATUS_PARAMETER_ARGS_OUT_OF_BOUNDS = -35
} EMBENET_RADIO_Status;


enum {
    EMBENET_RADIO_MAX_PSDU_LENGTH = 128,
    EMBENET_RADIO_MIN_PSDU_LENGTH = 1,
};


/**
 * @brief Radio callback handler.
 *
 * @param[in] timestamp callback function to get time in ns.
 */
typedef void (*EMBENET_RADIO_CaptureCbt)(void* context, EMBENET_TimeUs timestamp);


/// Type used to store Received packet information
typedef struct {
    EMBENET_RADIO_Power rssi;
    uint8_t             lqi;
    bool                crcValid; ///< Must be invalid if mpduLength < EMBENET_RADIO_MIN_PSDU_LENGTH or mpduLength > EMBENET_RADIO_MAX_PSDU_LENGTH
    size_t              mpduLength;
} EMBENET_RADIO_RxInfo;


/// Type defining continuous TX mode
typedef enum {
    EMBENET_RADIO_CONTINUOUS_TX_MODE_PN9,
    EMBENET_RADIO_CONTINUOUS_TX_MODE_CARRIER,
} EMBENET_RADIO_ContinuousTxMode;


/**
 * @brief Initializes and sets transceiver state to IDLE
 * @retval EMBENET_RADIO_STATUS_SUCCESS on success
 * @retval EMBENET_RADIO_STATUS_COMMUNICATION_ERROR on error
 */
EMBENET_RADIO_Status EMBENET_RADIO_Init(void);


/**
 * @brief Sets transceiver callbacks.
 * @param[in] onStartFrame - called when sync word is detected with a drawn back timestamp indicating the start of frame transmission, NULL is considered as no callback.
 * @param[in] onEndFrame - called when the last bit of frame was received, NULL is considered as no callback.
 * @param[in] cbtContext - argument with which the hanlder will be called.
 */
void EMBENET_RADIO_SetCallbacks(EMBENET_RADIO_CaptureCbt onStartFrame, EMBENET_RADIO_CaptureCbt onEndFrame, void* cbtContext);


/**
 * Deinitializes and puts transceiver in lowest energy consumption mode possible
 */
void EMBENET_RADIO_Deinit(void);


/**
 * @brief Aborts any pending reception or transmission, clears internal buffers and puts radio in IDLE state
 * @retval EMBENET_RADIO_STATUS_SUCCESS on success
 * @retval EMBENET_RADIO_STATUS_COMMUNICATION_ERROR on error
 */
EMBENET_RADIO_Status EMBENET_RADIO_Idle(void);


/**
 * @brief Wakes transceiver from IDLE state.
 *        Prepares transceiver for transmission. Load new data (packet) to receiver's buffer/fifo.
 * @param[in] channel - Channel number.
 * @param[in] txp transmit power in dBm
 * @param[in] psdu - data pointer, must not be NULL
 * @param[in] psduLen - data length in bytes (must be in range EMBENET_RADIO_MIN_PSDU_LENGTH to EMBENET_RADIO_MAX_PSDU_LENGTH)
 * @retval EMBENET_RADIO_STATUS_SUCCESS on success
 * @retval EMBENET_RADIO_STATUS_PARAMETER_ARGS_OUT_OF_BOUNDS when psduLen < EMBENET_RADIO_MIN_PSDU_LENGTH or psduLen > EMBENET_RADIO_MAX_PSDU_LENGTH
 * @retval EMBENET_RADIO_STATUS_COMMUNICATION_ERROR on error
 */
EMBENET_RADIO_Status EMBENET_RADIO_TxEnable(EMBENET_RADIO_Channel channel, EMBENET_RADIO_Power txp, uint8_t const* psdu, size_t psduLen);


/**
 * @brief Triggers transmission.
 * @retval EMBENET_RADIO_STATUS_SUCCESS on success
 * @retval EMBENET_RADIO_STATUS_COMMUNICATION_ERROR on error
 */
EMBENET_RADIO_Status EMBENET_RADIO_TxNow(void);


/**
 * @brief Wakes transceiver from IDLE state.
 *        Prepares transceiver for listening state.
 * @param[in] channel - Channel number.
 * @retval EMBENET_RADIO_STATUS_SUCCESS if no error occurred
 * @retval EMBENET_RADIO_STATUS_COMMUNICATION_ERROR if radio is not responding
 * @retval EMBENET_RADIO_STATUS_WRONG_STATE if funtion called in wrong state of radio
 */
EMBENET_RADIO_Status EMBENET_RADIO_RxEnable(EMBENET_RADIO_Channel channel);


/**
 * @brief Triggers listening state.
 * @retval EMBENET_RADIO_STATUS_SUCCESS if no error occurred
 * @retval EMBENET_RADIO_STATUS_COMMUNICATION_ERROR if radio is not responding
 */
EMBENET_RADIO_Status EMBENET_RADIO_RxNow(void);


/**
 * @brief Gets received frame.
 * @note Should be called after onEndFrame occurs.
 *
 * @param[out] buffer - data storage buffer pointer.
 * @param[out] bufferLength
 * @return @ref EMBENET_RADIO_RxInfo
 */
EMBENET_RADIO_RxInfo EMBENET_RADIO_GetReceivedFrame(uint8_t* buffer, size_t bufferLength);


/**
 * @brief Gets radio timings.
 * @note Returned values are evaluated empirically.
 *
 * @return radio timings @ref EMBENET_RADIO_Capabilities.
 */
EMBENET_RADIO_Capabilities const* EMBENET_RADIO_GetCapabilities(void);


/**
 * @brief Starts continuous transmission.
 * @param[in] mode Continuous TX mode
 * @param[in] channel TX channel
 * @param[in] txp TX power in dBm
 * @retval EMBENET_RADIO_STATUS_SUCCESS on success
 * @retval EMBENET_RADIO_STATUS_PARAMETER_ARGS_OUT_OF_BOUNDS when psduLen < EMBENET_RADIO_MIN_PSDU_LENGTH or psduLen > EMBENET_RADIO_MAX_PSDU_LENGTH
 * @retval EMBENET_RADIO_STATUS_COMMUNICATION_ERROR on error
 */
EMBENET_RADIO_Status EMBENET_RADIO_StartContinuousTx(EMBENET_RADIO_ContinuousTxMode mode, EMBENET_RADIO_Channel channel, EMBENET_RADIO_Power txp);

/** @} */

#ifdef __cplusplus
}
#endif

#endif // EMBENET_RADIO_H_ included
