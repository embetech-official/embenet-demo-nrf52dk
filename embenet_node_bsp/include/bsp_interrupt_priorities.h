/**
@file
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   Critical section.
@brief     Needed by toolchain_defs.h.

*/


#ifndef INTERRUPT_PRIORITIES_H
#define INTERRUPT_PRIORITIES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Interrupt priority order.
 */
enum {
    INTERRUPT_PRIORITY_HDLC_UART   = 2,
    INTERRUPT_PRIORITY_BUTTONS     = 7,
    INTERRUPT_PRIORITY_DEBUG_TIMER = 7,
    INTERRUPT_PRIORITY_DEBUG_UART  = 8,
};

/**
 * @brief Interrupt subpriority order.
 */
enum {
    INTERRUPT_SUBPRIORITY_HDLC_UART   = 0,
    INTERRUPT_SUBPRIORITY_BUTTONS     = 0,
    INTERRUPT_SUBPRIORITY_DEBUG_TIMER = 0,
    INTERRUPT_SUBPRIORITY_DEBUG_UART  = 0,
};

#ifdef __cplusplus
}
#endif

#endif /* INTERRUPT_PRIORITIES_H */
