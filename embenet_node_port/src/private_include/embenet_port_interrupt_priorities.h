/**
@file
@license
@copyright
@version   1.1.4417
@purpose   Critical section.
@brief     Needed by toolchain_defs.h.

*/


#ifndef EMBENET_INTERRUPT_PRIORITIES_H
#define EMBENET_INTERRUPT_PRIORITIES_H

/**
 * @brief Interrupt priority order.
 */
enum {
    INTERRUPT_PRIORITY_EMBENET_TIMER = 3,
    INTERRUPT_PRIORITY_RADIO         = 3,
};

/**
 * @brief Interrupt subpriority order.
 */
enum {
    INTERRUPT_SUBPRIORITY_RADIO         = 0,
    INTERRUPT_SUBPRIORITY_EMBENET_TIMER = 1,
};


#endif /* EMBENET_INTERRUPT_PRIORITIES_H */
