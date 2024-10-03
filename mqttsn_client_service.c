/**
@file
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET demo
@brief     Exemplary MQTT-SN client service
*/

#include "mqttsn_client.h"
#include "embenet_node.h"
#include "gpio_config.h"

#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

#include "nrf_gpio.h"

// MQTT-SN Gateway port number
const uint16_t gatewayPortNo = 1885;
// MQTT-SN Client port number
const uint16_t clientPortNo = gatewayPortNo;

// Enumeration describing the state of the MQTT-SN service
enum mqttsnServiceState {
    // In the process of connecting
    CONNECTING = 0,
    // In the process of registering an uptime topic
    REGISTER_UPTIME_TOPIC,
    // In the process of registering a button state topic
    REGISTER_BUTTON_STATE_TOPIC,
    // In the process of subscribing to topic
    SUBSCRIBE_TO_TOPIC,
    // Normal state of operation
    RUNNING
} serviceState = CONNECTING;

// Descriptor of the MQTT-SN client
static MQTTSNClient mqttsnClient;
// MQTT-SN service task id
static EMBENET_TaskId mqttsnTaskId = EMBENET_TASKID_INVALID;
// This will be the MQTT topic that the client publishes to, pushing uptime information
static char uptimeTopic[MQTTSN_MAX_TOPIC_NAME_LENGTH];
// This will be the MQTT topic that the client publishes to, pushing button state information
static char buttonTopic[MQTTSN_MAX_TOPIC_NAME_LENGTH];
// This will be the MQTT topic that the client subscribes to, in order to receive control messages
static const char* ledControlTopic = "ledcontrol";

// Prototypes of event handlers
static void onMQTTConnected(MQTTSNClient* client);
static void onMQTTDisconnected(MQTTSNClient* client);

// Structure holding event handlers
const MQTTSNClientEventHandlers mqttEventHandlers = {
    .onConnected = onMQTTConnected,
    .onDisconnected = onMQTTDisconnected,
    .onTopicRegisteredByGateway = NULL
};


/**
 * This callback function is called when the MQTT-SN client connects to the gateway.
 *
 * @param[in] client MQTT-SN client descriptor
 *
 */
static void onMQTTConnected(MQTTSNClient* client) {
    puts("MQTT-SN: Connected to gateway");
    // Move to another state and reschedule the service task
    serviceState = REGISTER_UPTIME_TOPIC;
    EMBENET_NODE_TaskSchedule(mqttsnTaskId, EMBENET_NODE_TIME_SOURCE_LOCAL, EMBENET_NODE_GetLocalTime());
}


/**
 * This callback function is called when the MQTT-SN client disconnects from the gateway.
 * It tries to reconnect over and over again.
 *
 * @param[in] client MQTT-SN client descriptor
 *
 */
static void onMQTTDisconnected(MQTTSNClient* client) {
    puts("MQTT-SN: Client disconnected. Will try to reconnect in 5s.");
    // Cancel the service task
    EMBENET_NODE_TaskCancel(mqttsnTaskId);
    // Re-initialize the client
    MQTTSN_CLIENT_Deinit(&mqttsnClient);
    // Use the UID of the node as part of the client ID
    char          clientId[32];
    sprintf(clientId, "Client%x%08x", (unsigned)(EMBENET_NODE_GetUID()>>32), (unsigned)(EMBENET_NODE_GetUID()));
    MQTTSN_CLIENT_Init(&mqttsnClient, clientPortNo, clientId, &mqttEventHandlers);
    // Re-initialize and restart the service task
    serviceState = CONNECTING;
    EMBENET_NODE_TaskSchedule(mqttsnTaskId, EMBENET_NODE_TIME_SOURCE_LOCAL, EMBENET_NODE_GetLocalTime() + 5000);
}


/**
 * This callback function is called when a topic is registered in the gateway.
 *
 * @param[in] client MQTT-SN client descriptor
 * @param[in] topicId id of the registered topic
 * @param[in] topicName registered topic (string)
 *
 */
static void onMQTTTopicRegistered(const struct MQTTSNClient* client, MQTTSNTopicId topicId, const char* topicName) {
    printf("MQTT-SN: Topic '%s' registered in gateway with id %d\n", topicName, (int)topicId);
    // Move to next state
    if (serviceState == REGISTER_UPTIME_TOPIC) {
        serviceState = REGISTER_BUTTON_STATE_TOPIC;
    } else {
        serviceState = SUBSCRIBE_TO_TOPIC;
    }
    // Reschedule immediately
    EMBENET_NODE_TaskSchedule(mqttsnTaskId, EMBENET_NODE_TIME_SOURCE_LOCAL, EMBENET_NODE_GetLocalTime());
}


/**
 * This callback function is called when message on the control topic is received.
 *
 * @param[in] client MQTT-SN client descriptor
 * @param[in] data message data
 * @param[in] dataSize message data size
 *
 */
static void onLedcontrolUpdate(struct MQTTSNClient* client, MQTTSNTopicId topicId, const void* data, size_t dataSize) {
    char* msg = (char*)data;

    printf("MQTT-SN: Got message on control topic: %s\n", msg);

    // Perform action based on received command
    if (0 == strncmp(msg, "led1on", 6)) {
        printf("MQTT-SN: Led 1 on\n");
        nrf_gpio_pin_clear(LED1);
    } else if (0 == strncmp(msg, "led1off", 7)) {
        printf("MQTT-SN: Led 1 off\n");
        nrf_gpio_pin_set(LED1);
    } else if (0 == strncmp(msg, "led2on", 6)) {
        printf("MQTT-SN: Led 2 on\n");
        nrf_gpio_pin_clear(LED2);
    } else if (0 == strncmp(msg, "led2off", 7)) {
        printf("MQTT-SN: Led 2 off\n");
        nrf_gpio_pin_set(LED2);
    } else if (0 == strncmp(msg, "led3on", 6)) {
        printf("MQTT-SN: Led 3 on\n");
        nrf_gpio_pin_clear(LED3);
    } else if (0 == strncmp(msg, "led3off", 7)) {
        printf("MQTT-SN: Led 3 off\n");
        nrf_gpio_pin_set(LED3);
    } else if (0 == strncmp(msg, "led4on", 6)) {
        printf("MQTT-SN: Led 4 on\n");
        nrf_gpio_pin_clear(LED4);
    } else if (0 == strncmp(msg, "led4off", 7)) {
        printf("MQTT-SN: Led 4 off\n");
        nrf_gpio_pin_set(LED4);
    } else {
        puts("MQTT-SN: Unknown command, ignoring");
    }
}


/**
 * Implementation of a service task, using a state machine.
 *
 * @param[in] taskId id of the embeNET Node task
 * @param[in] timeSource clock over which the task was scheduled
 * @param[in] t time at which the task was scheduled to run
 * @param[in] context user defined context
 */
static void mqttsnServiceTask(EMBENET_TaskId taskId, EMBENET_NODE_TimeSource timeSource, uint64_t t, void *context) {
    switch (serviceState) {
        case CONNECTING:
            puts("MQTT-SN: Connecting to gateway");
            // Get border router address. We assume that the MQTT-SN gateway runs on the border router IP.
            EMBENET_IPV6 addr;
            EMBENET_NODE_GetBorderRouterAddress(&addr);
            // Perform a clean connect - you can tweak the timings here
            MQTTSN_CLIENT_CleanConnect(&mqttsnClient, &addr, gatewayPortNo, 30, 10, NULL, NULL);
            // Reschedule the task to try again after 10s if failed to connect
            EMBENET_NODE_TaskSchedule(mqttsnTaskId, timeSource, t + 10000);
            break;
        case REGISTER_UPTIME_TOPIC:
            puts("MQTT-SN: Registering uptime topic");
            MQTTSN_CLIENT_RegisterTopic(&mqttsnClient, uptimeTopic, onMQTTTopicRegistered);
            break;
        case REGISTER_BUTTON_STATE_TOPIC:
            puts("MQTT-SN: Registering button state topic");
            MQTTSN_CLIENT_RegisterTopic(&mqttsnClient, buttonTopic, onMQTTTopicRegistered);
            break;
        case SUBSCRIBE_TO_TOPIC:
            printf("MQTT-SN: Subscribing to topic: '%s'\n", ledControlTopic);
            MQTTSN_CLIENT_Subscribe(&mqttsnClient, ledControlTopic, onLedcontrolUpdate);
            // Move to normal state of operation
            serviceState = RUNNING;
            // Reschedule the task
            EMBENET_NODE_TaskSchedule(mqttsnTaskId, timeSource, EMBENET_NODE_GetLocalTime() + 1000);
            break;
        case RUNNING: {
            // Get current local time
            uint64_t uptime = EMBENET_NODE_GetLocalTime();
            // Construct a simple JSON
            char uptimeStr[80];
            sprintf(uptimeStr, "{\"uptime\":%u}", (unsigned)(uptime / 1000U));
            // Publish the message
            printf("MQTT-SN: Publishing on topic '%s' message: %s\n", uptimeTopic, uptimeStr);
            MQTTSN_CLIENT_PublishMessage(&mqttsnClient, uptimeTopic, uptimeStr, strlen(uptimeStr));
            // Reschedule the task
            EMBENET_NODE_TaskSchedule(taskId, timeSource, t + 10000);
        } break;
        default:
            puts("MQTT-SN: Unknown service state");
            break;
    }
}


void mqttsn_client_service_init(void) {
    // Prepare clientId - use the UID of the node as part of the client ID
    EMBENET_EUI64 uid = EMBENET_NODE_GetUID();
    char          clientId[32];
    sprintf(clientId, "Client%x%08x", (unsigned)(uid>>32), (unsigned)(uid));
    // Prepare topics
    sprintf(uptimeTopic, "uptime/%x%08x", (unsigned)(uid>>32), (unsigned)(uid));
    sprintf(buttonTopic, "button/%x%08x", (unsigned)(uid>>32), (unsigned)(uid));
    // Initialize the MQTT-SN client
    if (MQTTSN_CLIENT_RESULT_OK == MQTTSN_CLIENT_Init(&mqttsnClient, clientPortNo, clientId, &mqttEventHandlers)) {
        // Create the service task
        mqttsnTaskId = EMBENET_NODE_TaskCreate(mqttsnServiceTask, &mqttsnClient);
        if (EMBENET_TASKID_INVALID == mqttsnTaskId) {
            MQTTSN_CLIENT_Deinit(&mqttsnClient);
            puts("MQTT-SN: Unable to create task. Service aborted.");
        } else {
            printf("MQTT-SN: Service initialized with clientId: %s\n", clientId);
        }
    } else {
        puts("MQTT-SN: Unable to initialize MQTT-SN client. Service aborted.");
    }
}


void mqttsn_client_service_start(void) {
    puts("MQTT-SN: Starting service");
    serviceState = CONNECTING;
    EMBENET_NODE_TaskSchedule(mqttsnTaskId, EMBENET_NODE_TIME_SOURCE_LOCAL, EMBENET_NODE_GetLocalTime());
}


void mqttsn_client_service_stop(void) {
    puts("MQTT-SN: Stopping service");
    // Cancel the service task
    EMBENET_NODE_TaskCancel(mqttsnTaskId);
    // Re-initialize the client
    MQTTSN_CLIENT_Deinit(&mqttsnClient);
    // Use the UID of the node as part of the client ID
    char          clientId[32];
    sprintf(clientId, "Client%x%08x", (unsigned)(EMBENET_NODE_GetUID()>>32), (unsigned)(EMBENET_NODE_GetUID()));
    MQTTSN_CLIENT_Init(&mqttsnClient, clientPortNo, clientId, &mqttEventHandlers);
}


void mqttsn_client_service_proc(void) {
    // Holds the last timestamp at which gateway was notified
    static uint64_t lastTimestamp;
    // Holds the number of button presses
    static int buttonPressCounter;

    // Check if service is running
    if (serviceState == RUNNING) {
        // Check if button is pressed
		if (!nrf_gpio_pin_read(BUTTON1)) {
            // Check if sufficient time passed from the last time we notified the gateway
            if (lastTimestamp + 1000 < EMBENET_NODE_GetLocalTime()) {
                // Prepare message to be published
                char payloadStr[80];
                sprintf(payloadStr, "{\"button\":%d}", ++buttonPressCounter);
                // Publish message
                printf("MQTT-SN: Publishing on topic '%s' message: %s\n", buttonTopic, payloadStr);
                MQTTSN_CLIENT_PublishMessage(&mqttsnClient, buttonTopic, payloadStr, strlen(payloadStr));
                // Save timestamp
                lastTimestamp = EMBENET_NODE_GetLocalTime();
            }
        }
    }
}
