#ifndef ENMS_DATA_H_
#define ENMS_DATA_H_

#ifdef __cplusplus
#    include <cstdint>
extern "C" {
#else
#    include <assert.h>
#    include <stdbool.h>
#    include <stddef.h>
#    include <stdint.h>
#endif


#define ENMS_PACKED __attribute__((packed))


#ifndef static_assert
#    define ENMS_STATIC_ASSERT(...) struct ENMS_static_assert_inactive
#else
#    define ENMS_STATIC_ASSERT(...) static_assert(__VA_ARGS__)
#endif


enum {
    ENMS_NODE_SERVICE_NAME_MAX_LENGTH = 16 ///< Maximum allowed length of the provided service name (inlcuding terminating '\0\ character)
};

/** Structure defining network stack version */
typedef struct ENMS_PACKED {
    uint8_t  verHi;
    uint8_t  verLo;
    uint16_t revision;
} ENMS_StackVersion;

// Check structure packing
ENMS_STATIC_ASSERT(sizeof(ENMS_StackVersion) == 4, "sizeof(ENMS_StackVersion) should be 4");

/** Structure defining basic information about the node */
typedef struct ENMS_PACKED {
    /// Hardware identifier. By default this is the MD5 hash of the hardware nameplate record.
    uint8_t hwId[16];
    /// Network stack version
    ENMS_StackVersion stackVersion;
} ENMS_BasicNodeInfo;

// Check structure packing
ENMS_STATIC_ASSERT(sizeof(ENMS_BasicNodeInfo) == 20, "sizeof(ENMS_BasicNodeInfo) should be 20");

/** Information about the service */
typedef struct ENMS_PACKED {
    uint8_t serviceNo;                                      ///< Index of the service (0..serviceCount-1)
    char    serviceName[ENMS_NODE_SERVICE_NAME_MAX_LENGTH]; ///< Name of the service (NULL-terminated string)
    uint8_t serviceState;                                   ///< Service state: 0 - inactive, 1 - active
} ENMS_ServiceInfo;

// Check structure packing
ENMS_STATIC_ASSERT(sizeof(ENMS_ServiceInfo) == 18, "sizeof(ENMS_ServiceInfo) should be 18");

enum {
    ENMS_NODE_BATTERY_STATE_MAINS_POWERED = INT8_C(101), ///< Indicates that the node is Mains powered
    ENMS_NODE_BATTERY_STATE_ERROR         = INT8_C(-1),  ///< Indicates error
};

/// Structure holding status information about the node
typedef struct ENMS_PACKED {
    uint64_t parentEUI64;    ///< EUI64 of the parent node
    int8_t   parentRssi;     ///< RSSI of the parent node
    uint16_t parentPdr;      ///< Packet delivery rate expressed in 0.01% units (0..10000)
    uint16_t dagRank;        ///< Nodes DAG rank
    uint8_t  upCells;        ///< TX cells to parent
    uint16_t upPacketRate;   ///< Packet rate expressed as a mean count of packets sent in one superframe towards the border router
    uint8_t  downCells;      ///< RX cells from parent
    uint16_t downPacketRate; ///< Packet rate expressed as a mean count of packets sent downward in one superframe
    uint16_t dutyCycle;      ///< Duty cycle expressed in 0.01% units (0..10000)
    /**
     * Number of times the packet reception queue reached the 'alert' condition during the last 1000 superframes.
     * When the number of packets in the packet reception queue reaches the alerting threshold value, the 'alert' condition is signaled. Such a superframe is then counted.
     * The value of queueAlertCnt is the number of such superframes in the last 1000 superframes. It gives information about peak packet queue usage.
     */
    uint16_t queueAlertCnt;
    /**
     * Number of times the packet reception queue was overflown during the last 1000 superframes.
     * When the packet reception queue is overflown it cannot store any more packets and subsequent incoming packets are dropped. When that happens in a given superframe,
     * such a superframe is then counted.
     * The value of queueOverflowCnt is the number of such superframes in the last 1000 superframes. It gives information about packet queue failures.
     */
    uint16_t queueOverflowCnt;
    uint8_t  neighborCount; ///< Number of visible neighbors that are considered stable
    /**
     * Battery gauge indicator expressed in percentages (0..100).
     * Special value of 101 means that the device is mains powered.
     * Special value of -1 indicates a generic error, e.g. the battery state could not be estimated due to a hardware error
     */
    uint8_t  batteryState;
    uint16_t estimatedLife; ///< Estimated lifetime of the node expressed in hours (valid only for battery powered devices).
    uint32_t upTime;        ///< Node uptime in seconds. This is the time since the last node reset.
} ENMS_StatusInfo;

// Check structure packing
ENMS_STATIC_ASSERT(sizeof(ENMS_StatusInfo) == 33, "sizeof(ENMS_StatusInfo) should be 33");

typedef enum {
    ENMS_NEIGHBOR_ROLE_PARENT    = 0,
    ENMS_NEIGHBOR_ROLE_CHILD     = 1,
    ENMS_NEIGHBOR_ROLE_UNRELATED = 2,
    ENMS_NEIGHBOR_ROLE_GEEZER    = 3, ///< inactive for a very long time
} ENMS_NeighborRole;

/// Structure holding status information about the node
typedef struct ENMS_PACKED {
    /// role and neighbor field length (compression is unimplemented), @ref ENMS_NeighborRole
    uint8_t roleNLength;
    /// EUI64 of the parent node
    uint64_t neighborEUI64;
    /// RSSI of the parent node
    int8_t neighborRSSI;
} ENMS_NeighborInfo;

// Check structure packing
ENMS_STATIC_ASSERT(sizeof(ENMS_NeighborInfo) == 10, "sizeof(ENMS_NeighborInfo) should be 10");

typedef enum {
    ENMS_CELL_ROLE_NONE         = 0,
    ENMS_CELL_ROLE_ADV          = 1,
    ENMS_CELL_ROLE_AUTO_DOWN    = 2,
    ENMS_CELL_ROLE_AUTO_UP      = 3,
    ENMS_CELL_ROLE_AUTO_UP_DOWN = 4,
    ENMS_CELL_ROLE_MANAGED      = 5,
    ENMS_CELL_ROLE_APP          = 6,
} ENMS_CellRole;

typedef enum {
    ENMS_CELL_TYPE_NONE = 0,
    ENMS_CELL_TYPE_TX   = 1,
    ENMS_CELL_TYPE_RX   = 2,
    ENMS_CELL_TYPE_TXRX = 3,
} ENMS_CellType;

/// Structure holding status information about the node
typedef struct ENMS_PACKED {
    uint8_t  function;       ///< Denotes cell role and type
    uint16_t pdr;            ///< Packet delivery ratio expressed in 0.01% units (0..10000), valid only for CELL_TYPE_TX and CELL_TYPE_TXRX
    uint8_t  slotOffset;     ///< Slot offset
    uint8_t  channelOffset;  ///< Channel offset
    uint64_t companionEUI64; ///< EUI64 of the companion node
} ENMS_CellInfo;

// Check structure packing
ENMS_STATIC_ASSERT(sizeof(ENMS_CellInfo) == 13, "sizeof(ENMS_CellInfo) should be 13");


#ifdef __cplusplus
}
#endif

#endif /* ENMS_DATA_H_ */
