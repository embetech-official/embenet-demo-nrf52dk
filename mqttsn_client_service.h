/**
@file
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET demo
@brief     Exemplary MQTT-SN client service
*/

#ifndef MQTTSN_CLIENT_SERVICE_H_
#define MQTTSN_CLIENT_SERVICE_H_

/**
 * @brief Initializes an exemplary, user-defined MQTT-SN service.
 *
 */
void mqttsn_client_service_init(void);

/**
 * @brief Starts the exemplary, user-defined MQTT-SN service.
 */
void mqttsn_client_service_start(void);

/**
 * @brief Stops the exemplary, user-defined MQTT-SN service.
 */
void mqttsn_client_service_stop(void);

/**
 * @brief Periodically scans a button and publishes a state message using MQTT-SN.
 */
void mqttsn_client_service_proc(void);

#endif // MQTTSN_CLIENT_SERVICE_H_
