/**
@file
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET demo
@brief     Exemplary UDP-based client service
*/

#include "custom_service.h"
#include "embenet_node.h"
#include "enms_node.h"
#include "gpio_config.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "nrf_gpio.h"


/// Socket descriptor for exemplary, user-defined custom service
static EMBENET_UDP_SocketDescriptor customServiceSocket;
/// Id of the task running the custom service
static EMBENET_TaskId customServiceTaskId = EMBENET_TASKID_INVALID;

/**
 * @brief User-defined function that will be invoked as a periodically scheduled task
 *
 * @param[in] taskId id of the task
 * @param[in] timeSource time source (local time or network time)
 * @param[in] t time at which the task was scheduled to run
 * @param[in] context generic, user-defined context
 */
static void customServiceTask(EMBENET_TaskId taskId, EMBENET_NODE_TimeSource timeSource, uint64_t t, void *context) {
    static int counter;

    // make a simple message with counter
    char message[32];
    int messageLength = snprintf(message, sizeof(message), "Custom message no %d", counter++);
    // get border router address
    EMBENET_IPV6 borderRouterAddress;
    EMBENET_NODE_GetBorderRouterAddress(&borderRouterAddress);
    // send UDP packet using port 1234
    if (EMBENET_RESULT_OK != EMBENET_UDP_Send(&customServiceSocket, &borderRouterAddress, 1234, message, messageLength)) {
        printf("CUSTOM_SERVICE: Failed to send UDP packet\n");
    }

    // Schedule next invocation of this task after 5 seconds
    EMBENET_NODE_TaskSchedule(taskId, timeSource, t + 5000);
}

/**
 * @brief User-defined function that will be invoked upon datagram reception on customServiceSocket
 *
 * @param[in] socket pointer to socket descriptor (facilitates binding same reception handler with multiple sockets; also socket descriptor stores user context)
 * @param[in] sourceAddress IPv6 Address of the packet originator
 * @param[in] sourcePort UDP source port
 * @param[in] data pointer to datagram's payload
 * @param[in] dataSize size of datagram's payload
 */
static void customServiceReceptionHandler(EMBENET_UDP_SocketDescriptor const *socket, EMBENET_IPV6 const *sourceAddress, uint16_t sourcePort, void const *data, size_t dataSize) {
    // Retrieve command from datagram's payload
    char const *msg = (char const*) data;

    if (dataSize >= 6) {
        // Perform action based on received command
        if (0 == strncmp(msg, "led1on", 6)) {
            printf("CUSTOM_SERVICE: Led 1 on\n");
            nrf_gpio_pin_clear(LED1);
        } else if (0 == strncmp(msg, "led1off", 7)) {
            printf("CUSTOM_SERVICE: Led 1 off\n");
            nrf_gpio_pin_set(LED1);
        } else if (0 == strncmp(msg, "led2on", 6)) {
            printf("CUSTOM_SERVICE: Led 2 on\n");
            nrf_gpio_pin_clear(LED2);
        } else if (0 == strncmp(msg, "led2off", 7)) {
            printf("CUSTOM_SERVICE: Led 2 off\n");
            nrf_gpio_pin_set(LED2);
        } else if (0 == strncmp(msg, "led3on", 6)) {
            printf("CUSTOM_SERVICE: Led 3 on\n");
            nrf_gpio_pin_clear(LED3);
        } else if (0 == strncmp(msg, "led3off", 7)) {
            printf("CUSTOM_SERVICE: Led 3 off\n");
            nrf_gpio_pin_set(LED3);
        } else {
            printf("CUSTOM_SERVICE: Unrecognized command with size: %d\n", (int)dataSize);
        }
    } else {
        printf("CUSTOM_SERVICE: Unrecognized command with size: %d\n", (int)dataSize);
    }
}

void custom_service_init(void) {
    // Initialize UDP socket on node's unicast address, and port 1234. Upon datagram reception, customServiceReceptionHandler will be called)
    customServiceSocket = (EMBENET_UDP_SocketDescriptor ) {
        .port = 1234,
        .groupId = 0, // GroupId is ignored, when using EMBENET_UDP_TRAFFIC_UNICAST
        .handledTraffic = EMBENET_UDP_TRAFFIC_UNICAST,
        .rxDataHandler = customServiceReceptionHandler,
        .userContext = NULL // userContext is not needed in this example, however user may pass it to callback invocation
    };

    // Register UDP socket. Registering socket enables datagram reception/transmission
    EMBENET_Result customServiceSocketRegistrationStatus = EMBENET_UDP_RegisterSocket(&customServiceSocket);
    if (EMBENET_RESULT_OK == customServiceSocketRegistrationStatus) {
        printf("CUSTOM_SERVICE: Socket %d registered successfully\n", (int)customServiceSocket.port);
        // Create a task using built-in scheduler
        customServiceTaskId = EMBENET_NODE_TaskCreate(customServiceTask, NULL);
        if (EMBENET_TASKID_INVALID == customServiceTaskId) {
            printf("CUSTOM_SERVICE: Unable to create task\n");
        } else {
            printf("CUSTOM_SERVICE: Service initialized\n");
        }
    } else {
        printf("CUSTOM_SERVICE: Registering socket failed with status %d\n", (int)customServiceSocketRegistrationStatus);
    }
}

void custom_service_start(void) {
    printf("CUSTOM_SERVICE: Starting service\n");
    // Get current time
    uint64_t current_time = EMBENET_NODE_GetLocalTime();
    // Schedule task using node's local time (invoke it after 2 seconds)
    EMBENET_NODE_TaskSchedule(customServiceTaskId, EMBENET_NODE_TIME_SOURCE_LOCAL, current_time + 2000);
}

void custom_service_stop(void) {
    printf("CUSTOM_SERVICE: Stopping service\n");
    // Cancel scheduled task
    EMBENET_NODE_TaskCancel(customServiceTaskId);
}
