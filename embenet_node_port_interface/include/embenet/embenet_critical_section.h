/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   EMBENET NODE Port Interface
@brief     Critical section interface for the EMBENET NODE Port
*/

#ifndef EMBENET_NODE_PORT_INTERFACE_EMBENET_CRITICAL_SECTION_H_
#define EMBENET_NODE_PORT_INTERFACE_EMBENET_CRITICAL_SECTION_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup embenet_node_port_critical_section Critical Section Interface
 *
 * This interface provides implementation of a critical section.
 * @{
 */

/**
 * @brief Enters critical section
 * @note In most implementations resolves to disabling interrupt handling.
 */
void EMBENET_CRITICAL_SECTION_Enter(void);


/**
 * @brief Exits critical section
 * @note In most implementations resolves to re-enabling interrupt handling, provided that handling was enabled before entering critical section.
 */
void EMBENET_CRITICAL_SECTION_Exit(void);

/** @} */

#ifdef __cplusplus
}
#endif


#endif
