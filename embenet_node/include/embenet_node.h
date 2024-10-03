/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET API
@brief     embeNET Node API

Acronyms:
                - PANID		Personal Area Network Identifier, it is assigned by the Border Router, it differentiates logically wireless networks.
                - JRC		Join Registrar Coordinator.
                - CJ		Constrained Join.
                - K1		Pre-shared 16B authentication key which is the same among all nodes in the same network. Its scope can vary from a single network
                            identified by PANID to all nodes in multiple networks. K1 is assigned by application.
                - K2		Key obtained during Constrained Join process, assigned automatically by JRC. Ensures network-level security.
                - PSK		Pre-shared Constrained Join key. Should be unique and shared with destined Join Registrar Coordinator.
*/

#ifndef EMBENET_NODE_H_
#define EMBENET_NODE_H_

#include "embenet_defs.h"
#include "embenet_node_event_handlers.h"
#include "embenet_udp.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @addtogroup embenet_node_api embeNET Node API
 *
 * This page documents the embeNET Node C API. The API consists of the following parts:
 *
 * Stack management                         ||
 * -----------------------------------------|------------------------------------------------------
 * @ref EMBENET_NODE_Init                   | Initializes the node network process.
 * @ref EMBENET_NODE_Deinit                 | Deinitializes the node network process.
 * @ref EMBENET_NODE_Proc                   | Advances (runs) the node network process.
 *
 * Network management                       ||
 * -----------------------------------------|------------------------------------------------------
 * @ref EMBENET_NODE_Join                   | Makes the node join the network.
 * @ref EMBENET_NODE_QuickJoin              | Makes the join the network using previously negotiated credentials for quicker join.
 * @ref EMBENET_NODE_Leave                  | Makes the node leave the network.
 * @ref EMBENET_NODE_GetUID                 | Gets own unique identifier.
 * @ref EMBENET_NODE_SetUID                 | Sets own unique identifier.
 * @ref EMBENET_NODE_GetBorderRouterAddress | Gets the IPv6 address of the border router.
 * @ref EMBENET_NODE_GetParentAddress       | Gets the IPv6 link-local address of the parent node.
 * @ref EMBENET_NODE_ForceParentChange      | Forces parent change.
 * @ref EMBENET_NODE_RootStart              | Starts operation as a root node.
 *
 * Group management                         ||
 * -----------------------------------------|------------------------------------------------------
 * @ref EMBENET_NODE_JoinGroup              | Makes the node join a specific group.
 * @ref EMBENET_NODE_LeaveGroup             | Makes the node leave a specific group.
 * @ref EMBENET_NODE_GetGroupCount          | Gets the number of groups the node belongs to.
 * @ref EMBENET_NODE_GetGroupByIndex        | Gets the group number by index.
 *
 * Task management                          ||
 * -----------------------------------------|------------------------------------------------------
 * @ref EMBENET_NODE_TaskCreate             | Creates a task that can be scheduled.
 * @ref EMBENET_NODE_TaskDestroy            | Destroys a previously created task.
 * @ref EMBENET_NODE_TaskSchedule           | Schedules a task.
 * @ref EMBENET_NODE_TaskCancel             | Cancels a scheduled task.
 * @ref EMBENET_NODE_GetLocalTime           | Gets current local node time.
 * @ref EMBENET_NODE_GetNetworkTime         | Gets current network time.
 * @ref EMBENET_NODE_GetNetworkAsn          | Gets current Absolute Slot Offset
 *
 * Utility functions                        ||
 * -----------------------------------------|------------------------------------------------------
 * @ref EMBENET_NODE_GetRandomValue         | Gets a random value
 * @ref EMBENET_NODE_GetVersion             | Gets embeNET stack version
 *
 * Refer to @ref embenet_node_user_guide for information on how to use the API.
 *
 * @{
 */

/** Structure defining the network configuration */
typedef struct {
    EMBENET_K1  k1;  ///< Common network key. This key must be the same for all nodes and border router joining the same network.
    EMBENET_PSK psk; ///< Pre-shared key
} EMBENET_NODE_Config;


typedef size_t EMBENET_TaskId; ///< Identifier of an application-level task running within the stack


#define EMBENET_TASKID_INVALID SIZE_MAX /// Special value of EMBENET_TaskId that informs that the task is invalid

/** Possible time sources for task scheduling */
typedef enum {
    /// Local node time
    EMBENET_NODE_TIME_SOURCE_LOCAL = 0,
    /// Network time
    EMBENET_NODE_TIME_SOURCE_NETWORK = 1
} EMBENET_NODE_TimeSource;

/**
 * Prototype of user task function that can be scheduled in either local, or network time.
 *
 * @param[in] taskId id of the running task
 * @param[in] timeSource time source for the scheduled task
 * @param[in] t time at which the task was expected to run
 * @param[in] context context as provided when the task was created
 */
typedef void (*EMBENET_NODE_TaskFunction)(EMBENET_TaskId taskId, EMBENET_NODE_TimeSource timeSource, uint64_t t, void* context);

/**
 * @brief Initializes the embeNET networking stack for node.
 *
 * This function initializes the embeNET networking stack in the node. It reserves and initializes the resources needed for the stack operation.
 * It also initializes the required hardware (BSP).
 * This call starts the local clock in node so that the time returned through a call to @ref EMBENET_NODE_GetLocalTime starts to flow.
 *
 * @note After initialization @ref EMBENET_NODE_Proc should be called periodically.
 *
 * For more information refer to @ref embenet_node_stack_handling
 *
 * @param[in] eventHandlers structure, desired handlers may be set
 *
 * @retval EMBENET_RESULT_OK if initialization completed successfully
 * @retval EMBENET_RESULT_INVALID_ARGUMENT if at least one of the input arguments was invalid
 */
EMBENET_Result EMBENET_NODE_Init(const EMBENET_NODE_EventHandlers* eventHandlers);

/**
 * @brief Deinitializes the embeNET networking stack.
 *
 * This function deinitializes the embeNET networking stack. Once called, all activities within the stack are stopped and all dynamically
 * allocated resources (if any) are freed. In order to use the stack again, one must call @ref EMBENET_NODE_Init.
 *
 * For more information refer to @ref embenet_node_stack_handling
 */
void EMBENET_NODE_Deinit(void);

/**
 * @brief Runs the networking process of the embeNET stack for node.
 *
 * This function should be called periodically within the main loop of a program (or a thread) after a call to @ref EMBENET_NODE_Init.
 * It advances the networking process that is running in the stack which is responsible for all networking activities.
 * In particular, many event callbacks registered in the stack are called from within the context of this function.
 * This function performs time-invariant operations that may be processed in non-ISR, non-privileged MCU routine.
 *
 * For more information refer to @ref embenet_node_stack_handling
 */
void EMBENET_NODE_Proc(void);

/**
 * @brief Starts the network joining process as a node.
 *
 * This functions starts the process of joining the node to the network. To join the network the application must provide the
 * configuration structure (see @ref EMBENET_NODE_Config) containing:
 * - k1 - common network key
 * - psk - pre-shared device-specific key
 *
 * For more information on joining the network refer to @ref embenet_node_network_handling
 *
 * @param[in] config network configuration (see @ref EMBENET_NODE_Config)
 *
 * @retval EMBENET_RESULT_OK if the joining process has begun successfully
 * @retval EMBENET_RESULT_INVALID_ARGUMENT if at least one of the input arguments was invalid
 */
EMBENET_Result EMBENET_NODE_Join(const EMBENET_NODE_Config* config);

/**
 * @brief Starts the network joining process as a node using a previously stored @ref EMBENET_NODE_QuickJoinCredentials for quicker join.
 *
 * This functions starts the process of joining the node to the network using the credentials (@ref EMBENET_NODE_QuickJoinCredentials),
 * that were established during the previous join. Re-using these credentials allows the node to speed up the joining process.
 *
 * For more information on joining the network refer to @ref embenet_node_network_handling and specifically: @ref embenet_node_network_quick_join
 *
 * @param[in] quickJoinCredentials credentials returned by the @ref EMBENET_NODE_OnJoined callback
 *
 * @retval EMBENET_RESULT_OK if the quick joining process has begun successfully
 * @retval EMBENET_RESULT_INVALID_ARGUMENT if at least one of the input arguments was invalid
 * @retval EMBENET_RESULT_INVALID_CREDENTIALS if the provided credentials were invalid
 */
EMBENET_Result EMBENET_NODE_QuickJoin(const EMBENET_NODE_QuickJoinCredentials* quickJoinCredentials);

/**
 * @brief Disconnects the node from the network.
 *
 * This function causes the node to go back to the initialized state, stopping all network activity.
 * If the node was joined to the network then all the tasks that were scheduled in network time get canceled.
 * This call can also be used to stop the on-going joining process.
 *
 * @return This call currently always returns EMBENET_RESULT_OK.
 */
EMBENET_Result EMBENET_NODE_Leave(void);

/**
 * @brief Starts operation as a root node.
 *
 * This function causes the node to act as a root node in the network. Once started, such a node is controlled
 * exclusively by an external entity called border router.
 *
 * During network formation and operation the root node can broadcast additional data to all nodes wishing to join the network.
 * This data can be set using the panData and panDataSize arguments. This data is available in the joining nodes through the
 * @ref EMBENET_NODE_OnJoinAttempt event handler.
 *
 * @note This call is only available if the embeNET Node library was build with ROOT_CAPABILITIES enabled
 *
 * @param[in] panData additional network-wide data to pass to each node wishing to join. The network is capable of sending max. 16 Bytes od PAN data
 * @param[in] panDataSize size of the additional network-wide data.
 *
 * @retval EMBENET_RESULT_OK if the root operation has started successfully
 * @retval EMBENET_RESULT_INVALID_ARGUMENT if at least one of the input arguments was invalid
 * @retval EMBENET_RESULT_ROOT_CAPABILITIES_DISABLED if the library was built without support for root capabilities
 */
EMBENET_Result EMBENET_NODE_RootStart(void const* panData, size_t panDataSize);

/**
 * @brief Makes the node join the given multicast group.
 *
 * The node can belong to many multicast groups. This call is used to join the group with a specific group identifier.
 *
 * @param[in] groupId identifier of the group that the node should join
 *
 * @retval true if the node successfully joined the group
 * @retval false otherwise
 */
bool EMBENET_NODE_JoinGroup(EMBENET_GroupId groupId);

/**
 * @brief Makes the node leave the given multicast group.
 *
 * The node can belong to many multicast groups. This call is used to leave the group with a specific group identifier.
 *
 * @param[in] groupId identifier of the group that the node should leave
 */
void EMBENET_NODE_LeaveGroup(EMBENET_GroupId groupId);

/**
 * @brief 		Gets the number of groups the node belongs to.
 *
 * The node can belong to many multicast groups. Each time the @ref EMBENET_NODE_JoinGroup or @ref EMBENET_NODE_JoinGroup
 * is called, the number of groups the node belongs to may change. This call allows to get the current number of groups
 * the node belongs to. After that the user code may poll each group using the @ref EMBENET_NODE_GetGroupByIndex
 *
 * @return 		current number of groups the node belongs to
 */
size_t EMBENET_NODE_GetGroupCount(void);

/**
 * @brief Gets the groups the node belongs to by their index.
 *
 * @param[in] index group index ranging from 0 to @ref EMBENET_NODE_GetGroupCount - 1
 *
 * @return group identifier or @ref EMBENET_GROUPID_INVALID if the index is outside the valid range
 */
EMBENET_GroupId EMBENET_NODE_GetGroupByIndex(size_t index);

/**
 * @brief Registers an application-level task.
 *
 * This function registers a new application-level task within the networking stack.
 *
 * @param[in] taskFunction pointer to a function that will be run as a task
 * @param[in] userContext optional context that will be passed to the taskFunction once it is called
 *
 * @return task identifier or EMBENET_TASKID_INVALID if the task has not been created
 */
EMBENET_TaskId EMBENET_NODE_TaskCreate(EMBENET_NODE_TaskFunction taskFunction, void* userContext);

/**
 * @brief Destroys a task
 *
 * @param[in] taskId task identifier (as returned by @ref EMBENET_NODE_TaskCreate)
 */
void EMBENET_NODE_TaskDestroy(EMBENET_TaskId taskId);

/**
 * @brief Schedules task in time, reschedules if task was already scheduled.
 *
 * @param[in] taskId task identifier (as returned by @ref EMBENET_NODE_TaskCreate)
 * @param[in] timeSource time source for the scheduled task
 * @param[in] t time expressed in ms
 *
 * @retval EMBENET_RESULT_OK if scheduled properly.
 * @retval EMBENET_RESULT_INVALID_ARGUMENT if at least one of the input arguments was invalid
 * @retval EMBENET_RESULT_UNABLE_TO_SCHEDULE_IN_THE_PAST if the schedule time is in the past
 * @retval EMBENET_RESULT_NOT_SYNCHRONIZED if schedule was requested in network time but the node is not synchronized to the network
 */
EMBENET_Result EMBENET_NODE_TaskSchedule(EMBENET_TaskId taskId, EMBENET_NODE_TimeSource timeSource, uint64_t t);

/**
 * @brief Cancels the previously scheduled task.
 *
 * @note May be safely called even when task was not scheduled. In such case there is no effect.
 *
 * @param[in] taskId task identifier (as returned by @ref EMBENET_NODE_TaskCreate)
 *
 * @retval EMBENET_RESULT_OK if the task canceled properly
 * @retval EMBENET_RESULT_INVALID_ARGUMENT if at least one of the input arguments was invalid
 */
EMBENET_Result EMBENET_NODE_TaskCancel(EMBENET_TaskId taskId);

/**
 * @brief Gets own UID which is an EUI64 address.
 * @returns UID or @ref EMBENET_EUI64_INVALID if the UID was not set neither by the underlying hardware nor by the user
 */
EMBENET_EUI64 EMBENET_NODE_GetUID(void);


/**
 * @brief Sets own UID which is an EUI64 address.
 *
 * This function sets the UID which is used to identify the node in the network.
 * In most cases calling this function directly from application code is not needed, as the UID of the node is normally taken
 * form the underlying hardware platform.
 *
 * In rare cases however the application may want to control how the UIDs are generated.
 * In such scenarios extreme care should be taken to ensure that the UIDs in nodes are truly unique across all available inventory.
 *
 * This function should not be called once the networking is engaged (after a call to @ref EMBENET_NODE_Join or @ref EMBENET_NODE_QuickJoin).
 */
EMBENET_Result EMBENET_NODE_SetUID(EMBENET_EUI64 uid);

/**
 * @brief Forces parent change.
 *
 * This function should be used when QoS highly outreaches the expected level. Eg. when multicast traffic does not work despite all other
 * services works properly.
 */
void EMBENET_NODE_ForceParentChange(void);

/**
 * @brief Gets the IPv6 address of the border router, if it is reachable.
 *
 * This function gets the IPv6 address of the border router that started the network that the node has joined.
 * In vast majority of cases the border router address becomes valid immediately after join.
 *
 * @param[out] ipv6 pointer the location where the IPv6 address will be stored, must not be NULL.
 *
 * @retval EMBENET_RESULT_OK if the border router IPv6 address was stored successfully
 * @retval EMBENET_RESULT_INVALID_ARGUMENT if at least one of the input arguments was invalid
 * @retval EMBENET_RESULT_UNSPECIFIED_ERROR if the IPv6 was not stored
 */
EMBENET_Result EMBENET_NODE_GetBorderRouterAddress(EMBENET_IPV6* ipv6);

/**
 * @brief Gets the link-local IPv6 address of the parent node.
 *
 * In vast majority of cases the parent address will be valid after node joins network. Parent address may change over time.
 *
 * @param[out] ipv6 pointer the location where the IPv6 address will be stored, must not be NULL.
 *
 * @retval EMBENET_RESULT_OK if parent's IPv6 address was stored successfully
 * @retval EMBENET_RESULT_INVALID_ARGUMENT if at least one of the input arguments was invalid
 * @retval EMBENET_RESULT_UNSPECIFIED_ERROR if the IPv6 was not stored
 */
EMBENET_Result EMBENET_NODE_GetParentAddress(EMBENET_IPV6* ipv6);

/**
 * @brief Gets the current local time since the networking stack was initialized.
 *
 * This function returns the value of a monotonic clock, that is started from value 0 when @ref EMBENET_NODE_Init is called.
 * The clock is stopped when EMBENET_NODE_Deinit is invoked.
 * The clock measures time in milliseconds. It is a local node clock and it is NOT synchronized with other nodes in the network.
 * By using a 64-bit value range it is assumed, that the time overflow should not be concerned.
 *
 * @return Local time expressed in ms.
 */
uint64_t EMBENET_NODE_GetLocalTime(void);

/**
 * @brief Gets the current network time.
 *
 * This time has no particular start point and may begin from a very large value.
 *
 * @note Network time is a current duration since ASN no 0. Every EmbeNET node holds the same time notion.
 * @return Time expressed in ms. 0 if not synchronized.
 */
uint64_t EMBENET_NODE_GetNetworkTime(void);

/**
 * @brief Gets the current network time expressed in Absolute Slot Number.
 *
 * This time has no particular start point and may begin from a very large value.
 *
 * @return Time expressed in ASN. 0 if not synchronized.
 */
uint64_t EMBENET_NODE_GetNetworkAsn(void);

/**
 * @brief Gets a random unsigned integer value from a given range.
 * @note The value of stop must be greater or equal the value of start.
 * @param[in] start lowest number in the range
 * @param[in] stop highest number in the range
 * @return random unsigned integer value in the range of start to stop.
 */
uint32_t EMBENET_NODE_GetRandomValue(uint32_t start, uint32_t stop);

/**
 * @brief Gets the embeNET stack version
 *
 * @return structure describing the version
 */
EMBENET_Version EMBENET_NODE_GetVersion(void);

/** @} */


#ifdef __cplusplus
}
#endif

#endif // EMBENET_NODE_H_
