/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET API
@brief     embeNET Node Trace API

*/

#ifndef EMBENET_NODE_TRACE_H_
#define EMBENET_NODE_TRACE_H_

#include <embetech/compiler_support.h>


/**
 * @defgroup embenet_node_port_api_TRACE Trace functions
 * Provides interface for user-defined trace handles on important network events
 * @{
 */

#ifdef __cplusplus
extern "C" {
#    include <cinttypes>
#    include <cstddef>
#else
#    include <inttypes.h>
#    include <stdbool.h>
#    include <stddef.h>
#endif

typedef enum {
    EMBENET_TRACE_CELL_EVENT_TX,
    EMBENET_TRACE_CELL_EVENT_RX,
} EMBENET_TRACE_CellEvent;

typedef enum {
    EMBENET_TRACE_CELL_ROLE_ADV,
    EMBENET_TRACE_CELL_ROLE_AUTO_UP,
    EMBENET_TRACE_CELL_ROLE_AUTO_DOWN,
    EMBENET_TRACE_CELL_ROLE_AUTO_UPDOWN,
    EMBENET_TRACE_CELL_ROLE_AUTO_MANAGED,
} EMBENET_TRACE_CellRole;

typedef enum {
    EMBENET_TRACE_CELL_TYPE_TX,
    EMBENET_TRACE_CELL_TYPE_RX,
    EMBENET_TRACE_CELL_TYPE_TXRX,
} EMBENET_TRACE_CellType;

typedef enum {
    EMBENET_TRACE_FRAME_TYPE_BEACON,
    EMBENET_TRACE_FRAME_TYPE_DATA,
    EMBENET_TRACE_FRAME_TYPE_ACK,
} EMBENET_TRACE_FrameType;

typedef struct {
    EMBENET_TRACE_CellEvent cellEvent;
    EMBENET_TRACE_CellRole  cellRole;
    EMBENET_TRACE_FrameType frameType;
    unsigned                channelOffset;
    unsigned                slotOffset;
    int8_t                  rssiOrTxPower;
    unsigned                length;
    uint64_t                asn;
    uint64_t                src;
    uint64_t                dst;
    uint64_t                node;
} EMBENET_TRACE_LinkLayerTelemetry;

/**
 * @brief Called immediately stack started.
 * @note Called in non privileged mode.
 * @param[in] eui of node
 */
typedef void (*EMBENET_TRACE_Started)(uint64_t eui);

/**
 * @brief Called immediately after synchronization event.
 * @note Called in privileged mode.
 * @param[in] panid of the network, node has joined to.
 */
typedef void (*EMBENET_TRACE_Synchronized)(uint16_t panid);

/**
 * @brief Called immediately after desynchronization event.
 * @note Called in privileged mode.
 */
typedef void (*EMBENET_TRACE_Desynchronized)(void);

/**
 * @brief Called when transmission was not successful.
 * @note Called in privileged mode.
 * @param[in] linkLocalDestinationEui
 * @param[in] destinationEui
 * @param[in] attempt that was made
 */
typedef void (*EMBENET_TRACE_PacketNoAck)(uint64_t linkLocalDestinationEui, uint64_t destinationEui, uint8_t attempt);

/**
 * @brief Called when managed transmission was not successful.
 * @note Called in privileged mode.
 * @param[in] linkLocalDestinationEui
 */
typedef void (*EMBENET_TRACE_ManagedPacketNoAck)(uint64_t linkLocalDestinationEui);

/**
 * @brief Called when all transmission attempts were unsuccessful and packet was discarded.
 * @note Called in privileged mode.
 * @param[in] linkLocalDestinationEui
 * @param[in] destinationEui
 */
typedef void (*EMBENET_TRACE_PacketNotDelivered)(uint64_t linkLocalDestinationEui, uint64_t destinationEui);

/**
 * @brief Called after connection to target parent is established:
 *      - neighbor was selected
 *	 	- autonomous cells was scheduled
 *	 	- mandatory managed cells were successfully negotiated and added
 * @note Called in non-privileged mode.
 * @param[in] parentEui
 */
typedef void (*EMBENET_TRACE_Joined)(uint64_t parentEui);

/**
 * @brief Called every time node synchronizes to it's time source.
 * @param[in] 	us - time correction expressed in microseconds
 */
typedef void (*EMBENET_TRACE_SyncCorrection)(int32_t us);

/**
 * @brief Called after parent is selected as new parent.
 * @note Called in non-privileged mode.
 * @param[in] parentEui (native endianess)
 */
typedef void (*EMBENET_TRACE_ParentSelected)(uint64_t parentEui);

/**
 * @brief Called after parent is considered lost:
 * @note Called in non-privileged mode.
 * @param[in] parentEui
 */
typedef void (*EMBENET_TRACE_ParentLost)(uint64_t parentEui);

/**
 * @brief Called every time a new neighbor is added to node's registry.
 * @note Called in non-privileged mode.
 * @param[in] neighborEui
 * @param[in] rssi averaged
 */
typedef void (*EMBENET_TRACE_NeighborAdded)(uint64_t neighborEui, int8_t rssi);

/**
 * @brief Called every time a new neighbor is removed from node's registry. Neighbor may be removed due to long inactivity time or lack of space in registry.
 * @note Called in non-privileged mode.
 * @param[in] neighborEui
 */
typedef void (*EMBENET_TRACE_NeighborRemoved)(uint64_t neighborEui);

/**
 * @brief Called every time node's rank in changed.
 * @note Called in non-privileged mode.
 * @param[in] rank value.
 */
typedef void (*EMBENET_TRACE_RankUpdate)(uint16_t rank);

/**
 * @brief Called every time a packet count in packet buffer is changed.
 * @note May be called in privileged mode or non-privileged mode.
 * @param[in] length
 */
typedef void (*EMBENET_TRACE_QueueLength)(size_t length);

/**
 * @brief Called every time the ENSM Status Indication Packet was sent.
 * @note Called in non-privileged mode.
 */
typedef void (*EMBENET_TRACE_EnmsStatusSent)(void);

/**
 * @brief Called every time radio is toggled from sleep to active state.
 * @note May be called in ISR.
 *
 * @param 	active - true if radio is turned on, otherwise false
 */
typedef void (*EMBENET_TRACE_RadioActivity)(bool active);

/**
 * @brief Called every time radio is toggled from sleep to active state.
 * @note May be called in ISR.
 */
typedef void (*EMBENET_TRACE_SlotStart)(void);

/**
 * @brief Called when frame is TXed or RXed.
 * @note Is called in ISR.
 */
typedef void (*EMBENET_TRACE_LinkLayerEvent)(const EMBENET_TRACE_LinkLayerTelemetry* linkLayerTelemetry);

/**
 * @brief Event handler that is called every time stack has free time.
 * @param[in] asn - of the free timeslot from which this free time starts
 * @param[in] startNwkTime - timestamp on which the free cell starts
 * @param[in] durationUs - how long the free period lasts
 */
typedef void (*EMBENET_TRACE_OnFreeSlots)(uint64_t asn, uint64_t startNwkTime, uint32_t durationUs);

/**
 * @brief Event called when there is an active slot (only when node is synchronized). Handler is called on entry and when slot is finished.
 * @note Is called in ISR.
 *
 * @param[in] enters - true if slot starts, false if slot ends
 */
typedef void (*EMBENET_TRACE_OnSlotStartEnd)(bool enters);

/**
 * @brief Event handler called every time MAC layer enters and leaves its routines.
 * @note Radio ISRs are not included.
 * @note Is called in ISR.
 *
 * @param[in] enters - true if enters routine, false if leaves
 */
typedef void (*EMBENET_TRACE_OnMacRoutine)(bool enters);

/**
 * @brief Event handler called every time RADIO API is used.
 * @note Is called in ISR.
 * @note Covers only TxEnable, RxEnable, TxNow and RxNow functions.
 *
 * @param[in] enters - true if enters RADIO function, false if leaves.
 */
typedef void (*EMBENET_TRACE_OnRadioApiUsed)(bool enters);

/**
 * @brief Event handler called every time MCU enters of leaves radio ISR.
 * @note Is called in ISR.
 *
 * @param[in] enters - true if enters, false if leaves
 */
typedef void (*EMBENET_TRACE_OnRadioIsr)(bool enters);

/// structure describing all callbacks, set to NULL to deactivate
typedef struct {
    EMBENET_TRACE_Started            onStarted;
    EMBENET_TRACE_Synchronized       onSynchronized;
    EMBENET_TRACE_Desynchronized     onDesynchronized;
    EMBENET_TRACE_PacketNoAck        onPacketNoAck;
    EMBENET_TRACE_ManagedPacketNoAck onManagedPacketNoAck;
    EMBENET_TRACE_PacketNotDelivered onPacketNotDelivered;
    EMBENET_TRACE_Joined             onJoined;
    EMBENET_TRACE_SyncCorrection     onSyncCorrection;
    EMBENET_TRACE_ParentSelected     onParentSelected;
    EMBENET_TRACE_ParentLost         onParentLost;
    EMBENET_TRACE_NeighborAdded      onNeighborAdded;
    EMBENET_TRACE_NeighborRemoved    onNeighborRemoved;
    EMBENET_TRACE_RankUpdate         onRankUpdate;
    EMBENET_TRACE_QueueLength        onQueueLength;
    EMBENET_TRACE_EnmsStatusSent     onEnmsStatusSent;
    EMBENET_TRACE_LinkLayerEvent     onLinkLayerEvent;
    EMBENET_TRACE_OnFreeSlots        onFreeSlots;

    EMBENET_TRACE_OnSlotStartEnd onSlotStartEnd;
    EMBENET_TRACE_OnMacRoutine   onMacRoutine;
    EMBENET_TRACE_OnRadioApiUsed onRadioApiUsed;
    EMBENET_TRACE_OnRadioIsr     onRadioIsr;
} EMBENET_NODE_TraceHandlers;

/**
 * @brief Connects trace handlers.
 * @param traceHandlers @ref EMBENET_NODE_TraceHandlers
 */
void EMBENET_NODE_SetTraceHandlers(const EMBENET_NODE_TraceHandlers* traceHandlers) EMBEUTILS_NONNULL();

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* EMBENET_NODE_TRACE_H_ */
