/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET UDP API
@brief     embeNET UDP API
*/

#ifndef EMBENET_UDP_H_
#define EMBENET_UDP_H_

#include "embenet_defs.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @page embenet_udp_using_sockets Using UDP sockets
 * @tableofcontents
 *
 * Introduction
 * ============
 *
 * The main data transmission protocol used in embeNET is UDP. Most application level communication is done using this protocol.
 * In order to transmit and receive UDP data, the Nodes as well as the Border Router are expected to register UDP sockets, through
 * which the communication will be carried. A socket is a logical endpoint in the networked device, that allows to easily dispatch network
 * traffic to various services running in the device. We may think of the socket as a filter that selects only particular network traffic and
 * feeds it into a software component (service) that owns the socket. Typically various services running in the node will use different ports
 * and thus will register their own sockets. Such sockets will be used for transmitting and receiving data.
 * In embeNET each socket is associated with a port number and may have three types of behavior concerning the way the receiving (destination)
 * device is addressed.
 *
 * Understanding types of sockets, ports and addressing
 * ====================================================
 *
 * In order to understand types of sockets in embeNET we first need to understand the IPv6 addressing principles employed in embeNET.
 * There are basically two types of network traffic supported:
 * - unicast traffic, where we send the data to one particular destination node which is described by its own unicast address
 * - multicast traffic, where we send the data to a group of nodes using group addressing
 * (Note, that each node may belong to none, one or more than one group, and the groups it belongs to may vary from node to node)
 *
 * Apart from that, the traffic is directed to a given port number in the destination device.
 * Thus the destination IPv6 address (unicast or multicast) and destination port identify the actual data recipient.
 *
 * In embeNET we can register sockets that:
 * - receive only unicast traffic on the given port (@ref EMBENET_UDP_TRAFFIC_UNICAST)
 * - receive only multicast (group) traffic on the given port for the single specific group (@ref EMBENET_UDP_TRAFFIC_MULTICAST)
 * - receive all traffic (unicast or multicast) on the given port for all joined groups (@ref EMBENET_UDP_TRAFFIC_ALL)
 *
 * This type of socket behavior is defined during socket registration (see @ref EMBENET_UDP_RegisterSocket). There is no way to change that
 * behavior other than unregister the given socket and register another one.
 *
 * Example
 * -------
 *
 * Let's assume that we have a network with the network prefix set to 2001:0db8:0000:0000.
 * Within that network there is a node with UID = 0000:0000:1234:5678
 * This node belongs to two groups number 12 and 15
 *
 * In order to address this node using unicast addressing we should use an address constructed in the following way:
 * @code
 * // Assemble an IPv6 unicast address
 * EMBENET_IPV6 unicastAddr = EMBENET_AssembleUnicastIpv6(0x20010db800000000, 0x0000000012345678);
 * @endcode
 *
 * In order to address this node as a member of the group number 12 we should use an address constructed in the following way:
 * @code
 * // Assemble an IPv6 multicast address to group 12
 * EMBENET_IPV6 multicastAddr12 = EMBENET_AssembleMulticastIpv6(0x20010db800000000, 12);
 * // Assemble an IPv6 multicast address to group 15
 * EMBENET_IPV6 multicastAddr15 = EMBENET_AssembleMulticastIpv6(0x20010db800000000, 15);
 * @endcode
 *
 * Let's register three sockets in this node - all of them on the same port number = 2222, so that:
 * - socket *U* will handle only unicast traffic (@ref EMBENET_UDP_TRAFFIC_UNICAST)
 * - socket *M* will handle only multicast traffic for group 12 (@ref EMBENET_UDP_TRAFFIC_MULTICAST)
 * - socket *A* will handle both unicast traffic and multicast traffic for all joined groups (@ref EMBENET_UDP_TRAFFIC_ALL)
 *
 * So now:
 * - if we send data to port 2222 using unicast addressing (unicastAddr - from the above example), our node will receive the same data on socket *U* and *A*
 * - if we send data to port 2222 using multicast addressing to group 12 (multicastAddr12 - from the above example), our node will receive the same data on socket *M* and *A*
 * - if we send data to port 2222 using multicast addressing to group 15 (multicastAddr15 - from the above example), our node will receive the same data only on socket *A*
 *
 * Now let's explore some corner cases:
 * - if we register socket *N* handling multicast traffic to group 13 the socket will not receive any data until the node joins this group
 * - similarly, when the node will leave group 13, socket *N* will stop receiving data
 * - if there are more sockets registered on the same port, they also will receive data (data will be "duplicated" and fed to each socket)
 *
 * Registering a socket
 * ====================
 *
 * Before using a socket we must register it through a call to @ref EMBENET_UDP_RegisterSocket. This call takes a pointer to a @ref EMBENET_UDP_SocketDescriptor
 * structure that describes the socket. The following example shows how to register a socket that accepts all types of traffic.
 *
 * @code
 * EMBENET_UDP_SocketDescriptor socket;
 *
 * socket.port = 1234;
 * socket.socketType  = EMBENET_UDP_TRAFFIC_ALL;
 * socket.rxHandler   = receptionHandler;
 * socket.groupId     = 0; // not used for this type of socket type (used only for multicast sockets)
 * socket.userContext = NULL;
 * if (EMBENET_RESULT_OK == EMBENET_UDP_RegisterSocket(&socket)) {
 *     // Socket registered successfully
 * } else {
 *     // Failed to register a socket
 * }
 * @endcode
 *
 * Receiving data
 * ==============
 *
 * In the above example the socket is created with a data reception function called 'receptionHandler'. This function may look like this:
 *
 * @code
 * void receptionHandler(EMBENET_UDP_SocketDescriptor const* socket, EMBENET_IPV6 const* address, void const* data, size_t dataLength)
 * {
 *     printf("Got %z bytes\n", dataLenght);
 * }
 * @endcode
 *
 * This function will be called every time a UDP packet through a given socket.
 *
 * Sending data
 * ============
 *
 * In order to send some data through a registered socket a call to @ref EMBENET_UDP_Send has to be made. The following example illustrates
 * the data sending mechanism by sending 5 bytes ('Hello') through a registered socket to a destination port 2000 in the destination node
 * described by an IPv6 address.
 *
 * @code
 * // prepare destination IPv6 address
 * EMBENET_IPV6 destinationAddress = EMBENET_AssembleUnicastIpv6(0x20010db800000000, 0x0000000012345678);
 *
 * if (EMBENET_RESULT_OK == EMBENET_UDP_Send(&socket, &destinationAddress, 2000, "Hello", 5) {
 *     // Data was scheduled to be send
 * } else {
 *     // Unable to send the data
 * }
 * @endcode
 *
 * Determining the amount of data that can be sent
 * -----------------------------------------------
 *
 * The @ref EMBENET_UDP_Send schedules a single UDP datagram to be sent. This call does not provide any fragmentation capabilities.
 * The maximum amount of data that can be sent by a single call to @ref EMBENET_UDP_Send can be determined using @ref EMBENET_UDP_GetMaxDataSize
 * function:
 *
 * @code
 * size_t maxUDPDataSize = EMBENET_UDP_GetMaxDatagramSize();
 * @endcode
 *
 * Unregistering the socket
 * ========================
 *
 * Once the socket is not needed anymore it can be unregistered by a call to @ref EMBENET_UDP_UnregisterSocket.
 * The following code illustrates this idea:
 *
 * @code
 * EMBENET_UDP_UnregisterSocket(&socket);
 * @endcode
 *
 * User context
 * ============
 *
 * Each socket can be assigned with a user defined pointer called userContext. This variable can point to any user-defined data to you want to bind with
 * the socket. The intended use case is to pass a user context to the data reception handler.
 * Below is an example. Let's assume that we want the reception handler to store the received value in a buffer, that will be set during socket registration:
 *
 * @code
 * char userBuffer[64];
 * @endcode
 *
 * We can register the socket with a user context pointing to that buffer
 * @code
 * EMBENET_UDP_SocketDescriptor socket;
 *
 * socket.port = 1234;
 * socket.socketType  = EMBENET_UDP_TRAFFIC_ALL;
 * socket.rxHandler   = receptionHandler;
 * socket.groupId     = 0;
 * socket.userContext = userBuffer;
 * if (EMBENET_RESULT_OK == EMBENET_UDP_RegisterSocket(&socket)) {
 *     // Socket registered successfully
 * } else {
 *     // Failed to register a socket
 * }
 * @endcode
 *
 * Now, when the receptionHandler is called, we can retrieve the buffer and use it:
 *
 * @code
 * void receptionHandler(EMBENET_UDP_SocketDescriptor const* socket, EMBENET_IPV6 const* address, void const* data, size_t dataLength)
 * {
 *     printf("Got %z bytes\n", dataLenght);
 *     memcpy(socket.userContext, data, dataLength);
 * }
 * @endcode
 *
 *
 */

/** @addtogroup embenet_udp_api embeNET UDP C API
 *
 * This page documents the embeNET UDP C API. The API consists of the following parts:
 *
 * Socket management                    ||
 * -------------------------------------|------------------------------------------------------
 * @ref EMBENET_UDP_RegisterSocket      | Registers an UDP socket.
 * @ref EMBENET_UDP_UnregisterSocket    | Unregisters an UDP socket.
 *
 * Sending data                         ||
 * -------------------------------------|------------------------------------------------------
 * @ref EMBENET_UDP_GetMaxDataSize      | Gets the maximum amount of data that can be sent in a single UDP datagram.
 * @ref EMBENET_UDP_Send                | Sends a single UDP datagram through a registered UDP socket.
 *
 * To get more information on how to use this API refer to: @ref embenet_udp_using_sockets
 *
 * @{
 */


/** Convenience alias for @ref EMBENET_UDP_SocketDescriptor that SHALL be used in user's code */
typedef struct EMBENET_UDP_SocketDescriptor EMBENET_UDP_SocketDescriptor;

/**
 * @brief Data reception handler that is called every time some data on a certain registered port is received.
 *
 * During socket registration (see @ref EMBENET_UDP_RegisterSocket) the user is expected to pass the callback function that will be called
 * each time a UDP datagram is received through this socket. This typedef defines the format of such function.
 *
 * @note This handler callback is called in non-privileged mode. It is called from a thread or main loop and not from an interrupt service routine.
 *
 * @param[in] socket descriptor of the socket that received the data
 * @param[in] sourceAddress IPv6 address of the sender
 * @param[in] sourcePort source port number that was used in the sender to send the data
 * @param[in] data pointer to memory region that holds the received data
 * @param[in] dataSize size of the received data (in number of bytes)
 */
typedef void (*EMBENET_UDP_RxDataHandler)(EMBENET_UDP_SocketDescriptor const* socket, EMBENET_IPV6 const* sourceAddress, uint16_t sourcePort, void const* data, size_t dataSize);

/**
 * Possible types of an UDP socket that describe what is the socket's listening IPv6 address.
 */
typedef enum {
    EMBENET_UDP_TRAFFIC_UNICAST   = 0, ///< The socket only listens on node's unicast address
    EMBENET_UDP_TRAFFIC_MULTICAST = 1, ///< The socket only listens on multicast address of given group
    EMBENET_UDP_TRAFFIC_ALL       = 2, ///< The socket listens both on nodes unicast address or any matching multicast address - equivalent to IPv6 [::]
} EMBENET_UDP_Traffic;

/**
 * @brief Structure describing an UDP Socket.
 *
 * This structure describes a single registered UDP socket. When registering a socket through a call to @ref EMBENET_UDP_RegisterSocket
 * the user is expected to fill all the structure fields EXCEPT the 'next' field, which is used internally by the embeNET stack.
 */
struct EMBENET_UDP_SocketDescriptor {
    /**
     * UDP port number to bind to. This parameter may be in the range of 1 to 65535.
     */
    uint16_t port;
    /**
     * Traffic that will be handled by the socket. Possible options:
     *  - @ref EMBENET_UDP_TRAFFIC_UNICAST - the socket will only receive data sent to the node using unicast destination address
     *  - @ref EMBENET_UDP_TRAFFIC_MULTICAST - the socket will only receive data sent to the node using matching multicast group address
     *  - @ref EMBENET_UDP_TRAFFIC_ALL - the socket will receive data sent to the node using unicast or matching multicast address
     */
    EMBENET_UDP_Traffic handledTraffic : 8;
    /**
     * Multicast group identifier used only when socketType is set to @ref EMBENET_UDP_TRAFFIC_MULTICAST.
     * In such case this identifier determines the multicast group address that the socket will listen to.
     * In other cases this field is ignored and should be set to 0.
     */
    EMBENET_GroupId groupId;
    /**
     * A mandatory user-defined callback function that will be called every time an UDP datagram is received on the socket.
     * This field must not be NULL.
     */
    EMBENET_UDP_RxDataHandler rxDataHandler;
    /**
     * An optional user-defined context pointer that will be passed to the rxHandler when it is called.
     */
    void* userContext;
    /**
     * Field reserved for the network stack.
     * When registering new socket set this field to NULL. Once the socket is registered this field MUST NOT be changed.
     */
    EMBENET_UDP_SocketDescriptor* next;
};


/**
 * @brief Registers a new UDP socket.
 *
 * This function is used to register a new UDP socket, thus enabling data reception on the resulting address/port combination.
 *
 * @param[in] socket pointer to a valid socket descriptor structure
 *
 * @note The provided socket descriptor instance MUST be valid and accessible until EMBENET_UDP_UnregisterSocket. In particular, the descriptor SHALL NOT have automatic storage duration.
 * @note The user MAY register socket with EMBENET_UDP_SOCKET_TYPE_MULTICAST==socketType and group not joined by the node. The user will not receive messages on this socket until the node joines the
 * required group
 * @return EMBENET_RESULT_OK if socket was properly registered, or error status when the registration failed
 */
EMBENET_Result EMBENET_UDP_RegisterSocket(EMBENET_UDP_SocketDescriptor* socket);

/**
 * @brief	Unregisters socket from interface
 * @param[in] socket pointer to valid socket descriptor instance
 * @retval EMBENET_RESULT_OK if socket was successfully unregistered
 * @retval EMBENET_RESULT_UDP_SOCKET_UNREGISTERED if the given socket has not been registered
 */
EMBENET_Result EMBENET_UDP_UnregisterSocket(EMBENET_UDP_SocketDescriptor* socket);

/**
 * @brief Gets the maximum size of a single UDP payload size
 *
 * This function returns the maximum size of data that can be sent in a single UDP datagram. This is the maximum allowed size of the data to be sent
 * by a call to @ref EMBENET_UDP_Send
 *
 * @param[in] socket pointer to valid socket descriptor instance
 * @return  maximum size of data that can be sent in a single UDP datagram (in number of bytes)
 */
size_t EMBENET_UDP_GetMaxDataSize(EMBENET_UDP_SocketDescriptor const* socket);

/**
 * @brief	Sends UDP datagram from the given socket
 *
 * This function schedules an UDP datagram to be sent.
 *
 * @note An UDP datagram can be sent only from a registered port.
 * @note The source address of the resulting IPv6 Packet will always resolve to Node's UNICAST address
 * @param[in]	socket pointer to valid socket descriptor instance from which the data will be sent
 * @param[in]	destinationAddress IPv6 destination address
 * @param[in]	destinationPort UDP destination port number
 * @param[in]	data pointer to memory address storing UDP payload data
 * @param[in]	dataSize size of data in bytes (see @ref EMBENET_UDP_GetMaxDataSize)
 * @retval EMBENET_RESULT_OK if datagram was scheduled properly for sending
 * @retval EMBENET_RESULT_INVALID_ARGUMENT if at least one of the input arguments was invalid
 * @retval EMBENET_RESULT_UDP_MAX_DATA_SIZE_EXCEEDED if the size of the data to be sent is too large (see @ref EMBENET_UDP_GetMaxDataSize)
 * @retval EMBENET_RESULT_UDP_PACKET_QUEUE_FULL if there is not enough space to buffer the data to send
 * @retval EMBENET_RESULT_UDP_SOCKET_UNREGISTERED if the given socket has not been registered
 */
EMBENET_Result EMBENET_UDP_Send(EMBENET_UDP_SocketDescriptor const* socket, EMBENET_IPV6 const* destinationAddress, uint16_t destinationPort, const void* data, size_t dataSize);

/** @} */


#ifdef __cplusplus
}
#endif

#endif /* EMBENET_UDP_H_ */
