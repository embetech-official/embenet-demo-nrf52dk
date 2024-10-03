/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   ENMS service
@brief     ENMS Node service API
*/

#ifndef ENMS_NODE_H_
#define ENMS_NODE_H_

#include "enms_data.h"

#include <embenet_node.h>
#include <embenet_node_diag.h>
#include <embenet_udp.h>


#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup enms_node ENMS Node API
 *
 * @{ */

/// Possible ENMS Node function results
typedef enum {
    /// Success
    ENMS_NODE_RESULT_OK = 0,
    /// At least one of the input arguments to a function was invalid. The call was aborted.
    ENMS_NODE_RESULT_INVALID_INPUT_ARGUMENT = 1,
    /// The service failed to register an UDP socket for communication. The service was aborted.
    ENMS_NODE_RESULT_FAILED_TO_REGISTER_UDP_SOCKET = 2,
    /// The service failed to create a task. The service was aborted.
    ENMS_NODE_RESULT_FAILED_TO_CREATE_TASK = 3,
    /// The service failed to run a task. The service was aborted.
    ENMS_NODE_RESULT_FAILED_TO_RUN_TASK = 4,
    /// The service failed to register service
    ENMS_NODE_RESULT_FAILED_TO_REGISTER_SERVICE = 5,
    /// The service failed to register service because it is already registered
    ENMS_NODE_RESULT_SERVICE_ALREADY_REGISTERED = 6,
    /// The service failed to send an UDP packet
    ENMS_NODE_RESULT_FAILED_TO_SEND_PACKET = 7
} EnmsNodeResult;


#ifndef ENMS_NODE_MAX_SERVICES
#    define ENMS_NODE_MAX_SERVICES 10 ///< Maximum number of services that can be registered
#endif

/// Default UDP port number for the ENMS service
#define ENMS_DEFAULT_PORT UINT16_C(0xf0b1)

/** Structure describing a service */
typedef struct {
    /// Name of the service
    char name[ENMS_NODE_SERVICE_NAME_MAX_LENGTH];
    /// State of the service
    uint8_t state;
} EnmsService;

/** Structure describing indication sending policy */
typedef struct {
    /// After what time after ENMS service is started will the first BASIC-INFO.indication be sent
    uint32_t firstBasicInfoSendTimeSec;
    /// How often will the subsequent BASIC-INFO.indications be sent
    uint32_t basicInfoSendPeriodSec;
    /// After what time after ENMS service is started will the first STATUS.indication be sent
    uint32_t firstStatusSendTimeSec;
    /// How often will the subsequent STATUS.indications be sent
    uint32_t statusSendPeriodSec;
    /// After what time after ENMS service is started will the first SERVICE-INFO.indication be sent
    uint32_t firstServiceInfoSendTimeSec;
    /// How often will the subsequent SERVICE.indications be sent
    uint32_t serviceInfoSendPeriodSec;
    /// After what time after ENMS service is started will the first NEIGHBORHOOD.indication be sent
    uint32_t firstNeighborhoodSendTimeSec;
    /// How often will the subsequent NEIGHBORHOOD.indications be sent
    uint32_t neighborhoodSendPeriodSec;
    /// After what time after ENMS service is started will the first CELLS.indication be sent
    uint32_t firstCellsSendTimeSec;
    /// How often will the subsequent CELLS.indications be sent
    uint32_t cellsSendPeriodSec;
} EnmsIndicationPolicy;

/** Structure defining the ENMS Node service instance */
typedef struct {
    /// Time at which ENMS-BASIC-INFO.indication should be sent (0 if not scheduled)
    uint64_t basicInfoIndicationSendTime;
    /// Time at which ENMS-STATUS.indication should be sent (0 if not scheduled)
    uint64_t statusIndicationSendTime;
    /// Time at which ENMS-SERVICE-INFO.indication should be sent (0 if not scheduled)
    uint64_t serviceInfoIndicationSendTime;
    /// Time at which ENMS-NEIGHBORHOOD-INFO.indication should be sent (0 if not scheduled)
    uint64_t neighborhoodIndicationSendTime;
    /// Time at which ENMS-CELLS-INFO.indication should be sent (0 if not scheduled)
    uint64_t cellsIndicationSendTime;
    /// UDP socket descriptor
    EMBENET_UDP_SocketDescriptor udpSocket;
    /// gateway address
    EMBENET_IPV6 gatewayAddress;
    /// TaskId
    EMBENET_TaskId taskId;
    /// Hardware identifier. By default this is the MD5 hash of the hardware nameplate record.
    uint8_t hwId[16];
    /// Network stack version
    ENMS_StackVersion stackVersion;
    /// Indication policy
    EnmsIndicationPolicy indicationPolicy;
    /// Services
    EnmsService services[ENMS_NODE_MAX_SERVICES];
    /// number of services
    uint8_t serviceCount;
} EnmsNode;

/**
 * @brief Initializes the ENMS Node service.
 *
 * This function initializes the ENMS Node service. It registers a UDP socket for communication with the ENMS BR service running typically in the border router.
 * It also creates a task that is responsible for periodic sending of ENSM-BASIC-INFO messages.
 *
 * @param[in] enmsNode ENMS Node service instance
 * @param[in] port UDP port number over which the service will communicate, provide 0 to use default port value 0xF0B1
 * @param[in] hwId hardware identifier
 * @param[in] indicationPolicy indication policy defining when and how often the indications should be sent (NULL to use default policy)
 *
 * @retval ENMS_NODE_RESULT_OK if the service was initialized successfully
 * @retval ENMS_NODE_RESULT_INVALID_INPUT_ARGUMENT if at least one of the input arguments was invalid
 * @retval ENMS_NODE_RESULT_FAILED_TO_REGISTER_UDP_SOCKET if the service failed to register a UDP socket
 * @retval ENMS_NODE_RESULT_FAILED_TO_CREATE_TASK if the service failed to create a task
 */
EnmsNodeResult ENMS_NODE_Init(EnmsNode* enmsNode, uint16_t port, const uint8_t hwId[16], const EnmsIndicationPolicy* indicationPolicy);

/**
 * Deinitializes the ENMS Node service
 *
 * @param[in] enmsNode ENMS Node service instance
 *
 * @retval ENMS_NODE_RESULT_OK if the service was deinitialized successfully
 * @retval ENMS_NODE_RESULT_INVALID_INPUT_ARGUMENT if at least one of the input arguments was invalid
 */
EnmsNodeResult ENMS_NODE_Deinit(EnmsNode* enmsNode);

/**
 * @brief Starts the ENMS Node service.
 *
 * This function starts the ENMS Node service. It is typically called in reaction to the event that the node has joined a network.
 * It schedules a task that periodically sends ENSM-BASIC-INFO messages.
 *
 * @param[in] enmsNode ENMS Node service instance
 *
 * @retval ENMS_NODE_RESULT_OK if the service was started successfully
 * @retval ENMS_NODE_RESULT_INVALID_INPUT_ARGUMENT if at least one of the input arguments was invalid
 * @retval ENMS_NODE_RESULT_FAILED_TO_RUN_TASK if the service was unable to run a task
 */
EnmsNodeResult ENMS_NODE_Start(EnmsNode* enmsNode);

/**
 * @brief Stops the ENMS Node service.
 *
 * This function stops the ENMS Node service. It is typically called in reaction to the event that the node has left a network.
 * The service can be restarted by a call to @ref ENMS_NODE_Start.
 *
 * @param[in] enmsNode ENMS Node service instance
 *
 * @retval ENMS_NODE_RESULT_OK if the service was stopped successfully
 * @retval ENMS_NODE_RESULT_INVALID_INPUT_ARGUMENT if at least one of the input arguments was invalid
 */
EnmsNodeResult ENMS_NODE_Stop(EnmsNode* enmsNode);

/**
 * @brief Registers a service.
 *
 * This function is used to tell the ENSM NODE service that other service runs in the node.
 * Each service is described by:
 * - a name (up to 15 characters + trailing '/0')
 * - an unsigned integer (8 bit) that denotes the state of the service (0 usually means that the service is not active)
 * Once the service is registered, its state can be changed by a call to @ref ENMS_NODE_SetServiceState
 *
 * @retval ENMS_NODE_RESULT_OK if the service was registered
 * @retval ENMS_NODE_RESULT_INVALID_INPUT_ARGUMENT if at least one of the input arguments was invalid
 * @retval ENMS_NODE_RESULT_FAILED_TO_REGISTER_SERVICE if the service was not registered (probably exceeded the number of services that can be handled - see @ref ENMS_NODE_MAX_SERVICES)
 */
EnmsNodeResult ENMS_NODE_RegisterService(EnmsNode* enmsNode, const char* serviceName, uint8_t serviceState);

/**
 * @brief Sets service state.
 *
 * This function is used to update the state of a service, previously registered by a call to @ref ENMS_NODE_RegisterService.
 *
 * @retval ENMS_NODE_RESULT_OK if the service state was updated
 * @retval ENMS_NODE_RESULT_INVALID_INPUT_ARGUMENT if at least one of the input arguments was invalid
 */
EnmsNodeResult ENMS_NODE_SetServiceState(EnmsNode* enmsNode, const char* serviceName, uint8_t serviceState);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ENMS_NODE_H_ */
