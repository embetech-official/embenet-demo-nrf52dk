/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET API
@brief     Common API definitions
*/

#ifndef EMBENET_DEFS_H_
#define EMBENET_DEFS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup embenet_common embeNET common definitions
 *  @{
 */

/// embeNET version number - high number
#define EMBENET_VER_HI 1
/// embeNET version number - low number
#define EMBENET_VER_LO 1
/// embeNET version number - revision number
#define EMBENET_REVISION 4417

/// Possible result codes
typedef enum {
    EMBENET_RESULT_OK = 0,
    EMBENET_RESULT_UNSPECIFIED_ERROR,              ///< Unspecified error
    EMBENET_RESULT_INVALID_ARGUMENT,               ///< Function arguments are invalid
    EMBENET_RESULT_CALLED_OUTSIDE_A_TASK,          ///< The called function was called outside of a running task
    EMBENET_RESULT_NOT_SYNCHRONIZED,               ///< The node is not synchronized to the network
    EMBENET_RESULT_UNABLE_TO_SCHEDULE_IN_THE_PAST, ///< The task was not scheduled because the requested time is in the past
    EMBENET_RESULT_INVALID_CREDENTIALS,            ///< The provided credentials were invalid
    EMBENET_RESULT_ROOT_CAPABILITIES_DISABLED,     ///< The requested operation requires root capabilities, which were not built in
    // UDP specific
    EMBENET_RESULT_UDP_MAX_DATA_SIZE_EXCEEDED, ///< Data size beyond limit
    EMBENET_RESULT_UDP_PACKET_QUEUE_FULL,      ///< Device's packet queue is full
    EMBENET_RESULT_UDP_FORWARDING_ERROR,       ///< Forwarding error
    EMBENET_RESULT_UDP_SOCKET_UNREGISTERED,    ///< UDP socket was not registered
    // JoinRules specific
    EMBENET_RESULT_JOIN_RULE_ALREADY_EXISTS, ///< Adding rule failed - the rule already exists
    EMBENET_RESULT_JOIN_RULE_NOT_FOUND,      ///< Rule not found in the register
    EMBENET_RESULT_JOIN_RULE_REGISTER_FULL,  // Adding rule failed - there is no more space to store the rule
} EMBENET_Result;


/* Network types */
typedef uint64_t EMBENET_NetworkPrefix; ///< Network prefix - common first 8 bytes of nodes IPv6 Address

typedef uint64_t EMBENET_EUI64;           ///< Unique identifier of the node
#define EMBENET_EUI64_INVALID UINT64_C(0) ///< Invalid value of EMBENET_GroupId used as an error indication

typedef uint16_t EMBENET_GroupId;                     ///< Multicast group ID
#define EMBENET_GROUPID_INVALID (EMBENET_GroupId)(0U) ///< Invalid value of EMBENET_GroupId used as an error indication

typedef uint16_t EMBENET_PANID; ///< IEEE802.15.4e PAN ID

/// IPv6 address
typedef struct {
    uint8_t val[16]; ///< Stored value
} EMBENET_IPV6;

/* Address conversion helper API */

/**
 * @brief Assembles Network prefix and Node's UID into IPv6 Address
 * @param nwkPrefix Network prefix
 * @param uid Unique Identifier (UID) of the node
 * @return IPv6 address
 */
EMBENET_IPV6 EMBENET_AssembleUnicastIpv6(EMBENET_NetworkPrefix nwkPrefix, EMBENET_EUI64 uid);

/**
 * @brief Assembles Network prefix and Multicast group ID into IPv6 Address
 * @param nwkPrefix Network prefix
 * @param gid Multicast group ID
 * @return IPv6 address
 */
EMBENET_IPV6 EMBENET_AssembleMulticastIpv6(EMBENET_NetworkPrefix nwkPrefix, EMBENET_GroupId gid);

/**
 * @brief Extracts UID from IPv6
 * @param ipv6 IPv6 address
 * @return UID
 */
EMBENET_EUI64 EMBENET_GetUidFromIpv6(const EMBENET_IPV6* ipv6);

/// Possible addressing modes
typedef enum {
    EMBENET_ADDRESSING_MODE_SINGLE, ///< Unicast addressing mode - the recipient is a single node
    EMBENET_ADDRESSING_MODE_GROUP,  ///< Multicast addressing mode - the recipients are all nodes within a group
} EMBENET_AddressingMode;


/* Network Encryption types */

typedef uint64_t EMBENET_RandomSeed; ///< Initial value of internal random number generators. USING NOT A RANDOM NUMBER WILL CAUSE A SERIOUS SECURITY VUNERABILITY

/// 128 bit pre-shared key used to authenticate Beacons
typedef struct {
    uint8_t val[16]; ///< Stored value
} EMBENET_K1;


/// Constrained join 128-bit pre-shared key. This key should be unique and shared with JRC
typedef struct {
    uint8_t val[16]; ///< Stored value
} EMBENET_PSK;


/// Structure describing the data necessary to perform a quick network rejoin
typedef struct {
    uint8_t reserved[52];
} EMBENET_NODE_QuickJoinCredentials;


/// Structure describing the embeNET version
typedef struct {
    uint8_t  hi;  ///< High version number
    uint8_t  lo;  ///< Low version number
    uint16_t rev; ///< Revision
} EMBENET_Version;

/** @} */


#ifdef __cplusplus
}
#endif

#endif // EMBENET_DEFS_H_
