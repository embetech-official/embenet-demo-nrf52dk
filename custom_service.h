/**
@file
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET demo
@brief     Exemplary UDP-based client service
*/

#ifndef CUSTOM_SERVICE_H_
#define CUSTOM_SERVICE_H_

/**
 * @brief Initializes an exemplary, user-defined, custom service.
 *
 * Opens a UDP socket and initializes a periodic task.
 */
void custom_service_init(void);

/**
 * @brief Starts the exemplary, user-defined, custom service.
 */
void custom_service_start(void);

/**
 * @brief Stops the exemplary, user-defined, custom service.
 */
void custom_service_stop(void);

#endif
