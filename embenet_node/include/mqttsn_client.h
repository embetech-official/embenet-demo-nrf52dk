/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET MQTT-SN client
@brief     MQTT-SN client API declaration

This implementation of MQTT-SN client is based on the MQTT-SN specification version 1.2.
*/

#ifndef MQTTSNCLIENT_H_
#define MQTTSNCLIENT_H_

#include <embenet_defs.h>
#include <embenet_node.h>
#include <embenet_udp.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup mqttsn_client MQTT-SN Client
 *
 *  This module implements the MQTT-SN client according to the MQTT-SN specification version 1.2.
 *  MQTT-SN, or Message Queuing Telemetry Transport for Sensor Networks, is a lightweight and efficient messaging protocol
 *  designed for constrained environments, particularly suited for low-power, low-bandwidth, and high-latency networks.
 *  It is an extension of the widely adopted MQTT protocol, optimized to meet the specific requirements of sensor networks
 *  and other resource-constrained devices.
 *
 *  One of the most important technical aspects of MQTT-SN is that it uses the UDP transport protocol instead of TCP
 *  used in MQTT. It also uses topic IDs instead of topic strings allowing for shorter UDP datagrams, at the expense
 *  of necessity to first register the topic in the gateway prior any usage.
 *
 *  This MQTT-SN Client service is a UDP based service running over the embeNET wireless communication protocol.
 *  The MQTT-SN protocol is a version of the popular Message Queuing Telemetry Transport (MQTT) protocol
 *  but made suitable for Sensor Networks that utilize UDP transport protocol instead of TCP.
 *
 *  This client supports the following functions:
 *   - connecting and disconnecting to the gateway
 *   - registering topics
 *   - subscribing to topics and receiving messages on that topic
 *   - publishing messages on topics
 *
 *  @{
 */

/// Possible states of the MQTT-SN client
typedef enum {
    MQTTSN_CLIENT_STATE_DISCONNECTED,            ///< Client is disconnected from the gateway
    MQTTSN_CLIENT_STATE_CONNECTING,              ///< Client is awaiting CONNACK from the gateway
    MQTTSN_CLIENT_STATE_AWAITING_WILL_TOPIC_REQ, ///< Client is awaiting WILL TOPIC REQUEST from the gateway
    MQTTSN_CLIENT_STATE_AWAITING_WILL_MSG_REQ,   ///< Client is awaiting WILL MESSAGE REQUEST from the gateway
    MQTTSN_CLIENT_STATE_CONNECTED,               ///< Client is connected to the gateway
    MQTTSN_CLIENT_STATE_DISCONNECTING            ///< Client is disconnecting from the gateway
} MQTTSNClientState;

/// Possible results of the MQTT-SN client API calls
typedef enum {
    MQTTSN_CLIENT_RESULT_OK = 0,
    MQTTSN_CLIENT_RESULT_INVALID_INPUT_ARGUMENT,
    MQTTSN_CLIENT_RESULT_FAILED_TO_REGISTER_UDP_SOCKET,
    MQTTSN_CLIENT_RESULT_FAILED_TO_CREATE_TASK,
    MQTTSN_CLIENT_RESULT_FAILED_TO_SEND_PACKET,
    MQTTSN_CLIENT_RESULT_CONNECT_SERIALIZATION_ERR,
    MQTTSN_CLIENT_RESULT_BUFFER_OR_CLIENT_IS_NULL,
    MQTTSN_CLIENT_CLIENTID_FORBIDDEN_LENGTH,
    MQTTSN_CLIENT_UNEXPECTED_ACK_RECEIVED,
    MQTTSN_CLIENT_TOPIC_NOT_IN_REGISTRATION,
    MQTTSN_CLIENT_REGACK_DESERIALIZATION_FAILED,
    MQTTSN_CLIENT_PUBLISH_DESERIALIZATION_FAILED,
    MQTTSN_CLIENT_SUBACK_DESERIALIZATION_FAILED,
    MQTTSN_CLIENT_GW_ERR_CONGESTION,
    MQTTSN_CLIENT_GW_INVALID_TOPIC_ID,
    MQTTSN_CLIENT_GW_ERR_NOT_SUPPORTED,
    MQTTSN_CLIENT_UNKNOWN_ERROR,
    MQTTSN_CLIENT_WILL_TOPIC_SERIALIZATION_ERROR,
    MQTTSN_CLIENT_UNEXPECTED_WILL_TOPIC_REQ_RECEIVED,
    MQTTSN_CLIENT_UNEXPECTED_WILL_MSG_REQ_RECEIVED,
    MQTTSN_CLIENT_WILL_MSG_SERIALIZATION_ERROR,
    MQTTSN_CLIENT_PING_SERIALIZATION_ERROR,
    MQTTSN_CLIENT_TXTOPICS_BUFFER_FULL,
    MQTTSN_CLIENT_REGISTER_SERIALIZATION_ERROR,
    MQTTSN_CLIENT_PUBLISH_SERIALIZATION_ERROR,
    MQTTSN_CLIENT_TOPIC_NOT_REGISTERED,
    MQTTSN_CLIENT_SUBSCRIBE_SERIALIZATION_ERROR,
    MQTTSN_CLIENT_RECEIVED_BROKEN_PACKET,
    MQTTSN_CLIENT_CLIENT_NOT_CONNECTED,
    MQTTSN_CLIENT_CLIENT_NOT_DISCONNECTED,
    MQTTSN_CLIENT_DISCONNECT_SERIALIZATION_ERROR,
    MQTTSN_CLIENT_RESULT_TOPIC_NOT_FOUND,
    MQTTSN_CLIENT_RESULT_TOPIC_NOT_PENDING_REGISTRATION,
    MQTTSN_CLIENT_RESULT_DISCONNECT_SERIALIZATION_ERROR,
    MQTTSN_CLIENT_SUBSCRIBE_BUFFER_FULL,
    MQTTSN_CLIENT_TOPIC_EXCEEDS_SET_MAX_LEN
} MQTTSNClientResult;

/// Maximum length of the MQTT-SN topic name
#define MQTTSN_MAX_TOPIC_NAME_LENGTH 38
/// Maximum length of the MQTT-SN message data
#define MQTTSN_MAX_MESSAGE_DATA 32
/// Maximum length of the MQTT-SN client id
#define MQTTSN_MAX_CLIENT_ID_LENGTH 23

/// Gateway response timeout in milliseconds, after which the client assumes the gateway is not responding
#define MQTTSN_CLIENT_GATEWAY_RESPONSE_TIMEOUT_MS 5000
/// Maximum number of topics that the client can subscribe to
#define MQTTSN_CLIENT_MAX_TOPICS_TO_SUBSCRIBE 10
/// Maximum number of topics that the client can publish to
#define MQTTSN_CLIENT_MAX_TOPICS_TO_PUBLISH 10

/// Type describing topic id
typedef uint16_t MQTTSNTopicId;

// Forward declaration
struct MQTTSNClient;

/**
 * @brief Callback function type describing a function that is called when the MQTT-SN client connects to the gateway.
 *
 * This callback is called to signal the event that the client successfully connected to the gateway.
 * Once the client is connected, it is ready to publish and receive messages.
 *
 * @param[in] client pointer to the MQTT-SN client description structure
 *
 */
typedef void (*MQTTSNOnClientConnected)(struct MQTTSNClient* client);

/**
 * @brief Callback function type describing a function that is called when the MQTT-SN client gets disconnected from the gateway.
 *
 * This callback is called when the client detects that there is no response from the gateway it was connected to.
 *
 * @param[in] client pointer to the MQTT-SN client description structure
 *
 */
typedef void (*MQTTSNOnClientDisconnected)(struct MQTTSNClient* client);

/**
 * @brief Callback function type describing a function that is called when a topic is successfully registered by the client in the gateway.
 *
 * This callback is called as a result of client trying to register a topic.
 * It is called when the gateway acknowledges the topic registration assigning a topic id to the topic name.
 *
 * @param[in] client pointer to the MQTT-SN client description structure
 * @param[in] topicId topic id assigned by the gateway to the topic name
 * @param[in] topicName topic name that was registered
 *
 */
typedef void (*MQTTSNOnTopicRegisteredByClient)(const struct MQTTSNClient* client, MQTTSNTopicId topicId, const char* topicName);

/**
 * @brief Callback function type describing a function that is called when a topic is successfully registered by the gateway.
 *
 * This callback is called as a result of gateway informing the client about a topic.
 * It is called when the gateway acknowledges the topic registration assigning a topic id to the topic name.
 *
 * @param[in] client pointer to the MQTT-SN client description structure
 * @param[in] topicId topic id assigned by the gateway to the topic name
 * @param[in] topicName topic name that was registered
 */
typedef void (*MQTTSNOnTopicRegisteredByGateway)(struct MQTTSNClient* client, MQTTSNTopicId topicId, const char* topicName);

/**
 * @brief Callback function type describing a function that is called when a message is received on a topic.
 *
 * This callback is called when a message is received on a topic that the client subscribed to.
 *
 * @param[in] client pointer to the MQTT-SN client description structure
 * @param[in] topicId topic id assigned by the gateway to the topic name
 * @param[in] data pointer to the message data
 * @param[in] dataSize length of the message data
 */
typedef void (*MQTTSNOnPublishReceived)(struct MQTTSNClient* client, MQTTSNTopicId topicId, const void* data, size_t dataSize);


/// Structure grouping event callbacks for MQTT-SN client
typedef struct {
    /// Callback that is called when the client connects to the gateway
    MQTTSNOnClientConnected onConnected;
    /// Callback that is called when the client disconnects from the gateway
    MQTTSNOnClientDisconnected onDisconnected;
    /// Callback that is called when the gateway registers a topic
    MQTTSNOnTopicRegisteredByGateway onTopicRegisteredByGateway;
} MQTTSNClientEventHandlers;

/// Structure describing a single MQTT-SN topic
typedef struct MQTTSNTopicDescriptor {
    /// Id of the REGISTER message that was sent during topic registration
    uint16_t registerMessageId;
    /// MQTT-SN topic id
    MQTTSNTopicId topicId;
    /// MQTT-SN topic name
    char topicName[MQTTSN_MAX_TOPIC_NAME_LENGTH];
    /// MQTT-SN topic name length
    size_t topicNameLen;
    /// Callback that will be called when this topic is registered by client
    MQTTSNOnTopicRegisteredByClient onTopicRegisteredByClient;
    /// Callback that will be called when a published message is received on this topic
    MQTTSNOnPublishReceived onPublishReceived;
} MQTTSNTopicDescriptor;

/// MQTT-SN session flags
typedef struct {
    unsigned int topicIdType : 2;
    unsigned int cleanSession : 1;
    unsigned int will : 1;
    unsigned int retain : 1;
    unsigned int QoS : 2;
    unsigned int dup : 1;
} MQTTSNSessionFlags;

/// Structure describing the MQTT-SN Client
typedef struct MQTTSNClient {
    /// Variable length string that contains a 1-23 character long string that uniquely identifies the client to the server. Must be unique!
    char clientId[MQTTSN_MAX_CLIENT_ID_LENGTH + 1];
    /// State of the MQTT-SN client
    MQTTSNClientState state;
    /// IPv6 address of the MQTT-SN gateway
    EMBENET_IPV6 gatewayAddress;
    /// Port number at the MQTT-SN gateway
    uint16_t gatewayPort;
    /// Will topic name
    char willTopicName[MQTTSN_MAX_TOPIC_NAME_LENGTH];
    /// Will message data
    char willMsg[MQTTSN_MAX_MESSAGE_DATA];
    /// Id of the keep alive task, by which the task is referecned.
    EMBENET_TaskId pingTaskId;
    /// Id of the timeout task, by which the task is referenced.
    EMBENET_TaskId timeoutTaskId;
    /// Time (in seconds) after which the gateway assumes the client is disconnected, if no message from client in that time is received
    uint16_t keepAliveTime;
    /// Minimum time between messages sent from the client. When no user messages are sent, PING should be sent instead. This value should be less than keepAliveTime
    uint16_t pingPeriod;
    /// Last time the client received packet from server
    uint64_t lastPacketReceptionTime;
    /// Last time at which the client sent packet to server
    uint64_t lastPacketSendTime;
    /// Sequential packet counter (note that 0 is forbidden)
    uint16_t packetId;
    /// Array of topics that the client subscribed to
    MQTTSNTopicDescriptor subscribeTopics[MQTTSN_CLIENT_MAX_TOPICS_TO_SUBSCRIBE];
    /// Array of topics that the client registered to publish to
    MQTTSNTopicDescriptor publishTopics[MQTTSN_CLIENT_MAX_TOPICS_TO_PUBLISH];
    /// MQTT-SN client session flags (options)
    MQTTSNSessionFlags flags;
    /// UDP socket descriptor
    EMBENET_UDP_SocketDescriptor udpSocket;
    /// Definition of callback handlers on client events
    MQTTSNClientEventHandlers eventHandlers;
} MQTTSNClient;


/**
 * @brief Initializes the MQTT-SN client.
 *
 * This function initializes the MQTT-SN client descriptor structure to a sane state using the provided setting.
 * It also registers embeNET tasks required for maintenance.
 * Once the client is initialized, it is ready to establish connection with the gateway.
 *
 * @note This function must be called before any other function of the MQTT-SN client API.
 *
 * @param[in] client pointer to the MQTT-SN client description structure
 * @param[in] port UDP port number to use on the client side
 * @param[in] clientId a 1 to 23 character long unique string that identifies the MQTT-SN client
 * @param[in] eventHandlers structure gathering user defined callbacks for handling client specific events
 *
 * @retval MQTTSN_CLIENT_RESULT_OK if the client was initialized successfully
 * @retval MQTTSN_CLIENT_RESULT_INVALID_INPUT_ARGUMENT if one or more arguments are invalid
 * @retval MQTTSN_CLIENT_RESULT_FAILED_TO_CREATE_TASK if one or more tasks failed to be created
 */
MQTTSNClientResult MQTTSN_CLIENT_Init(MQTTSNClient* client, uint16_t port, const char* clientId, const MQTTSNClientEventHandlers* eventHandlers);

/**
 * @brief Deinitializes the MQTT-SN client.
 *
 * This function deinitializes the MQTTSNClient, closing the UDP socket (if open) and destroying all internal tasks.
 *
 * @param[in] client pointer to the MQTT-SN client description structure
 */
void MQTTSN_CLIENT_Deinit(MQTTSNClient* client);

/**
 * @brief Makes the client try to connect to a gateway.
 *
 * This function opens the UDP socket that will be used for communication with the gateway and makes the client start the connection procedure
 * against a given gateway using the provided settings.
 * Once the connection is established, the MQTTSNClientEventHandlers.onConnected callback is called (as provided in @ref MQTTSN_CLIENT_Init).
 * If the connection procedure fails, the MQTTSNClientEventHandlers.onDisconnected callback is called (as provided in @ref MQTTSN_CLIENT_Init).
 *
 * @param[in] client pointer to the MQTT-SN client description structure
 * @param[in] gatewayAddress IPv6 address of the MQTT Gateway
 * @param[in] gatewayPort UDP port number of the MQTT Gateway
 * @param[in] keepAliveTime time (in seconds) after which the gateway assumes the client is disconnected, if no message from client in that time is received
 * @param[in] pingPeriod minimum time between messages sent from the client. When no user messages are sent, PING should be sent instead. This value should be less than keepAliveTime
 * @param[in] willTopic topic to which will shall be published if gateway deems the client lost. Nullable if will is not used.
 * @param[in] willMsg message that shall be published if gateway deems the client lost. Nullable if will is not used.
 *
 * @retval MQTTSN_CLIENT_RESULT_OK if the connection procedure started successfully
 * @retval MQTTSN_CLIENT_RESULT_INVALID_INPUT_ARGUMENT if one or more arguments are invalid
 * @retval MQTTSN_CLIENT_CLIENT_NOT_DISCONNECTED if the client is already connected or in the process of connecting or disconnecting
 * @retval MQTTSN_CLIENT_RESULT_FAILED_TO_REGISTER_UDP_SOCKET if the UDP socket could not be opened
 * @retval MQTTSN_CLIENT_RESULT_CONNECT_SERIALIZATION_ERR if the connection request could not be serialized
 * @retval MQTTSN_CLIENT_RESULT_FAILED_TO_SEND_PACKET if the connection request could not be sent via UDP socket
 */
MQTTSNClientResult MQTTSN_CLIENT_Connect(MQTTSNClient*       client,
                                         EMBENET_IPV6 const* gatewayAddress,
                                         uint16_t            gatewayPort,
                                         uint16_t            keepAliveTime,
                                         uint16_t            pingPeriod,
                                         const char*         willTopic,
                                         const uint8_t*      willMsg);


/**
 * @brief Makes the client try to connect to a gateway using clean connection.

 * This function opens the UDP socket that will be used for communication with the gateway and makes the client start the connection procedure
 * against a given gateway using the provided settings.
 * Once the connection is established, the MQTTSNClientEventHandlers.onConnected callback is called (as provided in @ref MQTTSN_CLIENT_Init).
 * If the connection procedure fails, the MQTTSNClientEventHandlers.onDisconnected callback is called (as provided in @ref MQTTSN_CLIENT_Init).
 *
 * @param[in] client pointer to the MQTT-SN client description structure
 * @param[in] gatewayAddress IPv6 address of the MQTT Gateway
 * @param[in] gatewayPort UDP port number of the MQTT Gateway
 * @param[in] keepAliveTime time (in seconds) after which the gateway assumes the client is disconnected, if no message from client in that time is received
 * @param[in] pingPeriod minimum time between messages sent from the client. When no user messages are sent, PING should be sent instead. This value should be less than keepAliveTime
 * @param[in] willTopic topic to which will shall be published if gateway deems the client lost. Nullable if will is not used.
 * @param[in] willMsg message that shall be published if gateway deems the client lost. Nullable if will is not used.
 *
 * @retval MQTTSN_CLIENT_RESULT_OK if the connection procedure started successfully
 * @retval MQTTSN_CLIENT_RESULT_INVALID_INPUT_ARGUMENT if one or more arguments are invalid
 * @retval MQTTSN_CLIENT_CLIENT_NOT_DISCONNECTED if the client is already connected or in the process of connecting or disconnecting
 * @retval MQTTSN_CLIENT_RESULT_FAILED_TO_REGISTER_UDP_SOCKET if the UDP socket could not be opened
 * @retval MQTTSN_CLIENT_RESULT_CONNECT_SERIALIZATION_ERR if the connection request could not be serialized
 * @retval MQTTSN_CLIENT_RESULT_FAILED_TO_SEND_PACKET if the connection request could not be sent via UDP socket
 */
MQTTSNClientResult MQTTSN_CLIENT_CleanConnect(MQTTSNClient*       client,
                                              EMBENET_IPV6 const* gatewayAddress,
                                              uint16_t            gatewayPort,
                                              uint16_t            keepAliveTime,
                                              uint16_t            pingPeriod,
                                              const char*         willTopic,
                                              const uint8_t*      willMsg);


/**
 * @brief Disconnects the client from the gateway.
 *
 * This function starts the disconnection procedure.
 * Once completed, the MQTTSNClientEventHandlers.onDisconnected callback is called (as provided in @ref MQTTSN_CLIENT_Init).
 *
 * @retval MQTTSN_CLIENT_RESULT_OK if the connection procedure started successfully
 * @retval MQTTSN_CLIENT_RESULT_INVALID_INPUT_ARGUMENT if one or more arguments are invalid
 * @retval MQTTSN_CLIENT_CLIENT_NOT_CONNECTED if the client is not connected to the gateway
 * @retval MQTTSN_CLIENT_RESULT_DISCONNECT_SERIALIZATION_ERROR if the disconnection request could not be serialized
 * @retval MQTTSN_CLIENT_RESULT_FAILED_TO_SEND_PACKET if the disconnection request could not be sent via UDP socket
 */
MQTTSNClientResult MQTTSN_CLIENT_Disconnect(MQTTSNClient* client);


/**
 * @brief Registers a topic in the gateway.
 *
 * Sends REGISTER packet to the gateway with provided topic string starting registration procedure. Once completed the provided
 * onTopicRegisteredCallback is called.
 *
 * @param[in] client pointer to the MQTT-SN client description structure
 * @param[in] topic topic string to register
 * @param[in] onTopicRegisteredCallback callback that will be called when the topic is registered
 *
 * @retval MQTTSN_CLIENT_RESULT_OK if the topic registration procedure started successfully
 * @retval MQTTSN_CLIENT_RESULT_INVALID_INPUT_ARGUMENT if one or more arguments are invalid
 * @retval MQTTSN_CLIENT_CLIENT_NOT_CONNECTED if the client is not connected to the gateway
 * @retval MQTTSN_CLIENT_TXTOPICS_BUFFER_FULL if the client has no more space to register new topics
 * @retval MQTTSN_CLIENT_REGISTER_SERIALIZATION_ERROR if the registration request could not be serialized
 * @retval MQTTSN_CLIENT_RESULT_FAILED_TO_SEND_PACKET if the registration request could not be sent via UDP socket
 */
MQTTSNClientResult MQTTSN_CLIENT_RegisterTopic(MQTTSNClient* client, const char* topic, MQTTSNOnTopicRegisteredByClient onTopicRegisteredCallback);


/**
 * @brief Gets the id of the registered topic.
 *
 * @param[in] client pointer to the MQTT-SN client description structure
 * @param[in] topic topic string to search for
 *
 * @returns topic id or 0 if the provided topic could not be found
 */
MQTTSNTopicId MQTTSN_CLIENT_GetTopicId(MQTTSNClient* client, const char* topic);


/**
 * @brief Publishes a message on a topic given the topic string.
 *
 * Publishes a message using PUBLISH packet to a provided topic.
 * Currently only supports QoS0
 *
 * @param[in] client pointer to the MQTT-SN client description structure
 * @param[in] topic string containing regular topic name
 * @param[in] message string of characters to be used as a payload
 * @param[in] messageLen length of message
 * @retval MQTTSN_CLIENT_RESULT_OK if the message was published successfully
 * @retval MQTTSN_CLIENT_RESULT_INVALID_INPUT_ARGUMENT if one or more arguments are invalid
 * @retval MQTTSN_CLIENT_CLIENT_NOT_CONNECTED if the client is not connected to the gateway
 */
MQTTSNClientResult MQTTSN_CLIENT_PublishMessage(MQTTSNClient* client, const char* topic, const char* message, size_t messageLen);


/**
 * @brief Publishes a message on a topic given the topic id.
 *
 * Publishes a message using PUBLISH packet to a provided topic.
 * Currently only supports QoS0
 *
 * @param[in] client pointer to the MQTT-SN client description structure
 * @param[in] topicId id of a target topic
 * @param[in] message string of characters to be used as a payload
 * @param[in] messageLen length of message
 *
 * @retval MQTTSN_CLIENT_RESULT_OK if the message was published successfully
 * @retval MQTTSN_CLIENT_RESULT_INVALID_INPUT_ARGUMENT if one or more arguments are invalid
 * @retval MQTTSN_CLIENT_PUBLISH_SERIALIZATION_ERROR if the publish request could not be serialized
 * @retval MQTTSN_CLIENT_RESULT_FAILED_TO_SEND_PACKET if the publish request could not be sent via UDP socket
 */
MQTTSNClientResult MQTTSN_CLIENT_PublishMessageById(MQTTSNClient* client, MQTTSNTopicId topicId, const char* message, size_t messageLen);


/**
 * @brief Subscribes to the topic.
 *
 * Subscribes to the topic by a regular topic name via SUBSCRIBE packet.
 * Once subscribed, the client will receive messages published on the topic through the provided callback.
 *
 * @param[in] client pointer to the MQTT-SN client description structure
 * @param[in] topic string containing regular topic name
 * @param[in] onPublishReceivedCallback callback that will be called when a message is received on the topic
 *
 * @retval MQTTSN_CLIENT_RESULT_OK if the client subscribed successfully
 * @retval MQTTSN_CLIENT_RESULT_INVALID_INPUT_ARGUMENT if one or more arguments are invalid
 * @retval MQTTSN_CLIENT_CLIENT_NOT_CONNECTED if the client is not connected to the gateway
 * @retval MQTTSN_CLIENT_SUBSCRIBE_BUFFER_FULL if the client has no more space to subscribe to new topics
 * @retval MQTTSN_CLIENT_SUBSCRIBE_SERIALIZATION_ERROR if the subscribe request could not be serialized
 * @retval MQTTSN_CLIENT_RESULT_FAILED_TO_SEND_PACKET if the subscribe request could not be sent via UDP socket
 */
MQTTSNClientResult MQTTSN_CLIENT_Subscribe(MQTTSNClient* client, const char* topic, MQTTSNOnPublishReceived onPublishReceivedCallback);

/** @} */

#ifdef __cplusplus
}
#endif


#endif // MQTTSNCLIENT_H_
