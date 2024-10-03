/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   EMBENET NODE Port Interface
@brief     Timer interface for the EMBENET NODE Port
*/

#ifndef EMBENET_NODE_PORT_INTERFACE_EMBENET_TIMER_H_
#define EMBENET_NODE_PORT_INTERFACE_EMBENET_TIMER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup embenet_node_port_timer Timer Interface
 *
 * Provides interface for the hardware timer that the embeNET Node stack uses to schedule various activities.
 *
 * @{
 */

typedef uint32_t EMBENET_TimeUs;           ///< Type to store time in microseconds
typedef int64_t  EMBENET_TimeDifferenceUs; ///< Type to store time difference in microseconds


/**
 * @brief EMBENET timer callback function type
 * @param[in] context general-purpose context associated with the timer
 */
typedef void (*EMBENET_TIMER_CompareCallback)(void* context);


/**
 * @brief Initializes and starts the timer
 *
 * This function should enable the timer to start counting from 0 upwards. It should also configure the callback function that will be called when the compare event is triggered.
 *
 * @param[in] compareCallback function that MUST be invoked upon reaching the time, set by the @ref EMBENET_TIMER_SetCompare function
 * @param[in] context general-purpose context that will be passed to the compareCallback when it is invoked
 */
void EMBENET_TIMER_Init(EMBENET_TIMER_CompareCallback compareCallback, void* context);


/**
 * @brief Deinitializes timer
 *
 * The deinitialization should stop the timer, and disable interrupts
 */
void EMBENET_TIMER_Deinit(void);


/**
 * @brief Sets compare time in us, note that timer wraps around EMBENET_TimeUs.
 *
 * When duration between current timer value and compare value exceeds value returned by EMBENET_TIMER_GetMaxCompareDuration, the event is considered to be in the past, and MUST be triggered
 * instantaneously. Otherwise, the compare interrupt MUST be scheduled. On low performace systems the implementation may introduce guard time (e.g. 20 us). the compareValue in range
 * [current time - guard, current time + guard] SHALL be treated as it would be in the past.
 * @param[in] compareValue value to compare in us.
 */
void EMBENET_TIMER_SetCompare(EMBENET_TimeUs compareValue);


/**
 * @brief Reads current time value in us. The returned value represents monotonically increasing time expressed in microseconds modulo 2^32.
 * The returned value MUST be in range [0, 2^32).
 * @return Timer value in us.
 */
EMBENET_TimeUs EMBENET_TIMER_ReadCounter(void);


/**
 * @brief Returns maximum duration that is considered by the timer as the future.
 *
 * When scheduling compare interrupt, user may set arbitrary compare value. However, the underlying timer may not be monotonic (wraps on 2^32 us),
 * so required compare value MAY be lower than current timer value. This would create confusing situation, where it is not known whether the user wanted the event to trigger in the past, or distant
 * future. When duration between current timer value and compare value exceeds value returned by EMBENET_TIMER_GetMaxCompareDuration, the event is considered to be in the past, and MUST be triggered
 * instantaneously. The returned value should be less or equal the period of the underlying timer.
 * In most implementations, it is sufficient to set this value to {timer period} / 2
 * @return Maximum compare duration expressed in us.
 */
EMBENET_TimeUs EMBENET_TIMER_GetMaxCompareDuration(void);

/** @} */


#ifdef __cplusplus
}
#endif

#endif
