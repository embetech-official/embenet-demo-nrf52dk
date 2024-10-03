/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET API
@brief     Definition of event handlers for node
*/


#ifndef EMBENET_NODE_EVENT_HANDLERS_H_
#define EMBENET_NODE_EVENT_HANDLERS_H_

#include "embenet_defs.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup embenet_node_api
 * @{
 */


/**
 * @brief Event hander that is called when the node joins a given network.
 *
 * This event handler is called when the node joins a network. The joining process could have been started by either a call to
 * @ref EMBENET_NODE_Join or @ref EMBENET_NODE_QuickJoin. Once this event handler is called, the node is connected to the
 * network identified by the given PANID (Personal Area Network IDentifier).
 * This handler also provides a set of credentials that can be used for quick join (see @ref embenet_node_network_quick_join).
 *
 * @param[in] panId PAN ID of network to which the node has joined.
 * @param[in] quickJoinCredentials credentials negotiated with the border router that can be used for quick rejoin next time
 *
 */
typedef void (*EMBENET_NODE_OnJoined)(EMBENET_PANID panId, const EMBENET_NODE_QuickJoinCredentials* quickJoinCredentials);

/**
 * @brief Event hander that is called when the node leaves the network.
 *
 * This event handler is called when the node leaves a network. This may happen due to the @ref EMBENET_NODE_Leave being called
 * or due to the fact that the network was lost. Either way, if the node wishes to join the network again, it should go
 * through the join or quick join procedure.
 */
typedef void (*EMBENET_NODE_OnLeft)(void);

/**
 * @brief Event hander that is called when the node tries to join a network.
 *
 * This event handler is called when an attempt is made in the stack to join a particular network, identified by the PAN ID identifier.
 * This is mostly for information purposes.
 *
 * @param[in] panId PAN ID of network to which the node tries to join.
 * @param[in] panData data that is propagated by root node in the network, may be NULL
 * @param[in] panDataSize size of that data (0 if data pointer is NULL)
 */
typedef void (*EMBENET_NODE_OnJoinAttempt)(EMBENET_PANID panId, const void* panData, size_t panDataSize);

/**
 * @brief Event hander that is called when the quick join credentials become obsolete
 *
 * This event handler is called when the stack detects that the credentials used in the quick join procedure became obsolete.
 * This indicates that these credentials should probably be forgotten and not used anymore.
 */
typedef void (*EMBENET_NODE_OnQuickJoinCredentialsObsolete)(void);

/**
 * @brief Event hander that is called when a UDP datagram was received on an unregistered port.
 *
 * This event handler is called when a UDP datagram is received but there was no socket associated with the incident port.
 *
 * @param[in] port port number on which the unexpected datagram was received
 */
typedef void (*EMBENET_NODE_DataOnUnregisteredPort)(uint16_t port);

/// Structure holding embeNET Node stack event handlers
typedef struct {
    /// Event hander that is called when the node joins a given network
    EMBENET_NODE_OnJoined onJoined;
    /// Event hander that is called when the node leaves the network
    EMBENET_NODE_OnLeft onLeft;
    /// Event hander that is called when the node tries to join a network
    EMBENET_NODE_OnJoinAttempt onJoinAttempt;
    /// Event hander that is called when the quick join credentials become obsolete
    EMBENET_NODE_OnQuickJoinCredentialsObsolete onQuickJoinCredentialsObsolete;
    /// Event hander that is called when a UDP datagram was received on an unregistered port
    EMBENET_NODE_DataOnUnregisteredPort onDataOnUnregisteredPort;
} EMBENET_NODE_EventHandlers;

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* EMBENET_NODE_EVENT_HANDLERS_H_ */
