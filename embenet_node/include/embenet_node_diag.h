/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET API
@brief     embeNET Node Diagnostic API

*/

#ifndef EMBENET_NODE_DIAG_H_
#define EMBENET_NODE_DIAG_H_


#include "embenet_defs.h"

#include <stdbool.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint64_t timeOverall; ///< Overall time of activity in microseconds
    uint64_t timeTx;      ///< Time spent on frame transmission in microseconds
    uint64_t timeRx;      ///< Time spent on listening or receiving in microseconds
    uint64_t timeActive;  ///< Time spent on active state in microseconds
} EMBENET_NODE_DIAG_DutyCycleRawData;

typedef struct {
    /* Number of times the packet reception queue reached the 'alert' condition during the last 1000 superframes.
     * When the number of packets in the packet reception queue reaches the alerting threshold value, the 'alert' condition
     * is signaled.Such a superframe is then counted.The value of queueAlertCnt is the number of such superframes in the
     * last 1000 superframes.It gives information about peak packet queue usage.
     */
    unsigned alert;
    /* Number of times the packet reception queue was overflown during the last 1000 superframes.
     * When the packet reception queue is overflown it cannot store any more packets and subsequent incoming packets are dropped.
     * When that happens in a given superframe, such a superframe is then counted. The value of queueOverflowCnt is the number
     * of such superframes in the last 1000 superframes. It gives information about packet queue failures.
     */
    unsigned overflow;
} EMBENET_NODE_DIAG_QueueCounters;

typedef enum {
    EMBENET_NODE_DIAG_NEIGHBOR_ROLE_PARENT    = 0,
    EMBENET_NODE_DIAG_NEIGHBOR_ROLE_CHILD     = 1,
    EMBENET_NODE_DIAG_NEIGHBOR_ROLE_UNRELATED = 2,
} EMBENET_NODE_DIAG_NeighborRole;

typedef struct {
    uint64_t                       eui;  ///< 0 if entry is inactive
    int8_t                         rssi; ///< 127 denotes, that rssi could not be obtained
    EMBENET_NODE_DIAG_NeighborRole role;
} EMBENET_NODE_DIAG_NeighborInfo;

typedef enum {
    EMBENET_NODE_DIAG_CELL_ROLE_NONE         = 0,
    EMBENET_NODE_DIAG_CELL_ROLE_ADV          = 1,
    EMBENET_NODE_DIAG_CELL_ROLE_AUTO_DOWN    = 2,
    EMBENET_NODE_DIAG_CELL_ROLE_AUTO_UP      = 3,
    EMBENET_NODE_DIAG_CELL_ROLE_AUTO_UP_DOWN = 4,
    EMBENET_NODE_DIAG_CELL_ROLE_MANAGED      = 5,
    EMBENET_NODE_DIAG_CELL_ROLE_APP          = 6,
} EMBENET_NODE_DIAG_CellRole;

typedef enum {
    EMBENET_NODE_DIAG_CELL_TYPE_NONE = 0,
    EMBENET_NODE_DIAG_CELL_TYPE_TX   = 1,
    EMBENET_NODE_DIAG_CELL_TYPE_RX   = 2,
    EMBENET_NODE_DIAG_CELL_TYPE_TXRX = 3,
} EMBENET_NODE_DIAG_CellType;

typedef struct {
    EMBENET_NODE_DIAG_CellRole role; ///< EMBENET_NODE_DIAG_CELL_ROLE_NONE if entry is inactive
    EMBENET_NODE_DIAG_CellType type; ///< EMBENET_NODE_DIAG_CELL_TYPE_NONE if entry is inactive
    uint16_t                   pdr;  ///< expressed in 0.01% unit
    uint8_t                    slotOffset;
    uint8_t                    channelOffset;
    uint64_t                   companionEui;
} EMBENET_NODE_DIAG_CellInfo;

/**
 * @retval true - node operates as root
 * @retval false - node does not operate as root
 */
bool EMBENET_NODE_DIAG_IsRoot(void);

/**
 * @return parent EUI
 */
EMBENET_EUI64 EMBENET_NODE_DIAG_GetParentEUI64(void);

/**
 * @return parent RSSI (127 if RSSI is not available)
 */
int8_t EMBENET_NODE_DIAG_GetParentRSSI(void);

/**
 * @return current Packet Delivery Rate to parent expressed in 0.01% units (0..10000)
 */
uint16_t EMBENET_NODE_DIAG_GetParentPDR(void);

/**
 * @return node's DAGRank
 */
uint16_t EMBENET_NODE_DIAG_GetDAGRank(void);

/**
 * @note Returns sum of managed TX cells to parent node.
 *
 * @return up cells count
 */
unsigned EMBENET_NODE_DIAG_GetUpCells(void);

/**
 * @note Packet rate is averaged through all managed cells to parent node.
 * @note It is defined as the rate of used cells to all passed cells to parent node.
 *
 * @return up packet rate
 */
uint16_t EMBENET_NODE_DIAG_GetUpPacketRate(void);

/**
 * @note Returns sum of RX cells from child nodes.
 *
 * @return down cells count
 */
unsigned EMBENET_NODE_DIAG_GetDownCells(void);

/**
 * @note Packet rate is averaged through all downlink autonomous cells (autonomous cells to listen for packets from parent).
 * @note It is defined as the rate of used cells to all passed cells from parent node.
 *
 * @return up packet rate in 0.01% unit
 */
uint16_t EMBENET_NODE_DIAG_GetDownPacketRate(void);

/**
 * @return radio READY state duty cycle in 0.01% unit.
 */
uint16_t EMBENET_NODE_DIAG_GetRadioReadyDutyCycle(void);

/**
 * @return radio TX state duty cycle in 0.01% unit
 */
uint16_t EMBENET_NODE_DIAG_GetRadioTxDutyCycle(void);

/**
 * @return radio RX state duty cycle in 0.01% unit
 */
uint16_t EMBENET_NODE_DIAG_GetRadioRxDutyCycle(void);

/**
 * @return @ref EMBENET_NODE_DIAG_DutyCycleRawData
 */
EMBENET_NODE_DIAG_DutyCycleRawData EMBENET_NODE_DIAG_GetRadioDutyCycleRaw(void);

/**
 * @return @ref EMBENET_NODE_DIAG_QueueCounters
 */
EMBENET_NODE_DIAG_QueueCounters EMBENET_NODE_DIAG_GetQueueThresholdAndOverflowCounters(void);

/**
 * @return neighbors count that are active
 */
unsigned EMBENET_NODE_DIAG_GetNeighborCount(void);

/**
 * @param[in] index - should be in range of <0, EMBENET_NODE_DIAG_GetNeighborCount()-1>
 * @return @ref EMBENET_NODE_DIAG_NeighborInfo
 */
EMBENET_NODE_DIAG_NeighborInfo EMBENET_NODE_DIAG_GetNeighborInfo(unsigned index);

/**
 * @return active cells count
 */
unsigned EMBENET_NODE_DIAG_GetCellsCount(void);

/**
 * @param[in] index - should be in range of <0, EMBENET_NODE_DIAG_GetCellsCount()-1>
 * @return @ref EMBENET_NODE_DIAG_CellInfo
 */
EMBENET_NODE_DIAG_CellInfo EMBENET_NODE_DIAG_GetCellInfo(unsigned index);

/**
 * @note Multiply @ref EMBENET_NODE_DIAG_GetSlotframeLength() by @ref EMBENET_NODE_DIAG_GetSlotDurationUs() to get slotframe length.
 * @return slots count in superframe
 */
unsigned EMBENET_NODE_DIAG_GetSlotframeLength(void);

/**
 * @note Multiply @ref EMBENET_NODE_DIAG_GetSlotframeLength() by @ref EMBENET_NODE_DIAG_GetSlotDurationUs() to get slotframe length.
 * @return slot duration
 */
unsigned EMBENET_NODE_DIAG_GetSlotDurationUs(void);

#ifdef __cplusplus
}
#endif

#endif /* EMBENET_NODE_DIAG_H_ */
