/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   EMBENET NODE EUI64 port API
@brief     Capabilities of the EMBENET NODE Port
*/

#ifndef EMBENET_NODE_PORT_INTERFACE_EMBENET_PORT_CAPABILITIES_H_
#define EMBENET_NODE_PORT_INTERFACE_EMBENET_PORT_CAPABILITIES_H_

#ifdef __cplusplus
#    include <cstdint>
#    include <cstdlib>
#else
#    include <stdbool.h>
#    include <stdint.h>
#    include <stdlib.h>
#endif

#include "embenet_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup embenet_node_port_capabilities Port Capabilities
 *
 * This interface allows to fetch platform-related port capabilities that the embeNET Node stack uses.
 *
 * @{
 */

/**
 * @brief MAC timings
 * @note MAC timings must conform to the embeNET radio timing requirements, otherwise runtime error will be thrown.
 */
typedef struct {
    // time-slot related
    EMBENET_TimeUs TsTxOffsetUs;     ///< Delay from start of slot to the start of data frame transmission
    EMBENET_TimeUs TsTxAckDelayUs;   ///< Delay from the end of reception of a data frame to the start of ACK transmission
    EMBENET_TimeUs TsLongGTUs;       ///< Half margin of frame reception (will be applied as +- to the expected time of reception)
    EMBENET_TimeUs TsShortGTUs;      ///< Half margin of ACK frame reception
    EMBENET_TimeUs TsSlotDurationUs; ///< Length of slot
                                     // radio watchdog
    EMBENET_TimeUs wdRadioTxUs;      ///< Transmission trigger watchdog (guards whether start of frame after transmission trigger occurs)
    EMBENET_TimeUs wdDataDurationUs; ///< Data frame transmission watchdog (guards data transmission it its time does not go out of bounds)
    EMBENET_TimeUs wdAckDurationUs;  ///< ACK frame transmission watchdog (guard ACK transmission if its time does not go out of bounds)
} EMBENET_MAC_Timings;


extern const EMBENET_MAC_Timings embenetMacTimings;

extern const size_t  embenetMacChannelListSize; ///< The count of non-advertisement channels TSCH operates on
extern const uint8_t embenetMacChannelList[];   ///< non-advertisement TSCH channel map, MAC always selects channels in range of <0,embenetMacChannelListSize)

extern const size_t  embenetMacAdvChannelListSize; ///< The count of advertisement channels TSCH operates on
extern const uint8_t embenetMacAdvChannelList[];   ///< non-advertisement TSCH channel map, MAC always selects channels in range of <0,embenetMacAdvChannelListSize)

extern const uint32_t embenetMacKaPeriodSlots; ///< Default KA period


/********************** CONSIDER REMOVING THE DECLARATIONS BELOW **********************/

extern const EMBENET_TimeUs embenetMacTimeCorrectionGuardUs; ///< If time correction within synchronizations will overlap given time guard node will print warn log
extern const uint32_t       embenetMacDesyncTimeoutSlots;    ///< Synchronization timeout, if no synchronization event happens to this time, node considers itself as desynchronized


typedef struct {
    uint64_t n1; ///< First node of connected pair
    uint64_t n2; ///< Second node of connected pair
} EMBENET_MAC_TopologyEntry;


extern const bool                      embenetMacTopologyActive;           ///< true if topology is forced, otherwise false
extern const EMBENET_MAC_TopologyEntry embenetMacTopologyList[];           ///< list of pairs with mutual connection
extern const size_t                    embenetMacTopologyListEntriesCount; ///< embenetMacTopologyList entries count

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* EMBENET_NODE_PORT_INTERFACE_EMBENET_PORT_CAPABILITIES_H_ */
