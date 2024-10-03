/**
@file
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET demo for nRF52
@brief     This is a demo application showing simple usage of the embeNET wireless communication protocol
*/

// Standard C library
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

// embeNET includes
#include "embenet_node.h"
#include "enms_node.h"
#include "gpio_config.h"
// demo services
#include "custom_service.h"
#include "mqttsn_client_service.h"
// board and chip specific header files
#include "SEGGER_RTT.h"
#include "app_error.h"
#include "nrf_assert.h"
#include "nrf_clock.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_nvic.h"
#include "nrf_power.h"
#include "nrf_gpio.h"


/// Descriptor of the ENMS service (network maintenance and visualization)
EnmsNode enmsNode;

// Enable printf logging through RTT
_ssize_t _write(int file, const void* ptr, size_t len) {
    (void)file; /* Not used, avoid warning */
    for (size_t i = 0; i < len; ++i) {
        SEGGER_RTT_PutCharSkipNoLock(0, ((const uint8_t*)ptr)[i]);
    }
    return len;
}

static void gpio_init(void) {
    // Configure Buttons
    nrf_gpio_cfg_input(BUTTON1, NRF_GPIO_PIN_PULLUP);
    // Configure LEDs
    nrf_gpio_cfg_output(LED1);
    nrf_gpio_cfg_output(LED2);
    nrf_gpio_cfg_output(LED3);
    nrf_gpio_cfg_output(LED4);
    nrf_gpio_pin_set(LED1);
    nrf_gpio_pin_set(LED2);
    nrf_gpio_pin_set(LED3);
    nrf_gpio_pin_set(LED4);
}

static void init(void) {
    // preemption priority grouping
    NVIC_SetPriorityGrouping(3); // 16 preemption, 16 subpriorities

    // enable low power sleep
    nrf_power_task_trigger(NRF_POWER_TASK_LOWPWR);
    // configure system clock
    // Start LFCLK
    nrf_clock_event_clear(NRF_CLOCK_EVENT_LFCLKSTARTED);
    nrf_clock_lf_src_set(NRF_CLOCK_LFCLK_Xtal);
    nrf_clock_task_trigger(NRF_CLOCK_TASK_LFCLKSTART);
    // Start HFCLK
    nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
    nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTART);
    // wait for clocks to be started
    while (!nrf_clock_event_check(NRF_CLOCK_EVENT_LFCLKSTARTED) || !nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED))
        ;
    // initialize RTT
    SEGGER_RTT_Init();
}

/**
 * User-defined callback that will be called, when the node joins the network.
 * @param[in] panId Identifier of the Network that node joined
 * @param[in] quickJoinCredentials pointer to the Quick Join Credentials that MAY be stored by the user to facilitate rejoining process.
 */
static void onJoined(EMBENET_PANID panId, const EMBENET_NODE_QuickJoinCredentials* quickJoinCredentials) {
    printf("Joined network with PANID: 0x%04" PRIx16 "\n", panId);

    // Start ENMS Service that provides network-wide telemetry information
    EnmsNodeResult enmsStartStatus = ENMS_NODE_Start(&enmsNode);
    if (ENMS_NODE_RESULT_OK == enmsStartStatus) {
        printf("ENMS service started\n");
    } else {
        printf("ENMS service failed to start with status: %d\n", (int)enmsStartStatus);
    }

#if 1 != IS_ROOT
    // Start exemplary, user-defined custom service
    custom_service_start();
    // Start MQTT-SN demo service
    mqttsn_client_service_start();
#endif
}

/**
 * @brief User-defined callback, that will be called after the node leaves the network
 */
static void onLeft(void) {
    printf("Node has left the network\n");
    // Stop ENMS service
    EnmsNodeResult enmsStopStatus = ENMS_NODE_Stop(&enmsNode);
    if (ENMS_NODE_RESULT_OK == enmsStopStatus) {
        printf("ENMS service stopped\n");
    } else {
        printf("ENMS service failed to stop with status: %d\n", (int)enmsStopStatus);
    }

#if 1 != IS_ROOT
    // Stop exemplary, user-defined custom service
    custom_service_stop();
    // Stop MQTT-SN demo service
    mqttsn_client_service_stop();
#endif
}


/**
 * @brief User-defined callback, that will be called when node tries to join the network
 *
 * NOTE: This callback is included in this demo only for debugging purposes
 */
static void onJoinAttempt(EMBENET_PANID panId, const void* panData, size_t panDataSize) {
    printf("Node is attempting to join the network with PANID 0x%04" PRIx16 "\n", panId);
    printf("Network-wide data (%uB)\n", (unsigned)panDataSize);
}


/**
 * @brief User-defined callback, that will be called when the node receives UDP datagram on closed port
 *
 * NOTE: This callback is included for purely debugging purposes. It should be Never used as a method of reliable data transfer
 */
static void dataOnUregisteredPort(uint16_t port) {
    printf("Got UDP datagram on unregistered port no: %" PRIu16 "\n", port);
}

/**
 * @brief User-defined callback, that will be called when provided quick join credentials become obsolete.
 *
 * If the quick join feature is used, user should delete the stored data and store new data, when onJoined callback will be called again.
 * This demo application does not use the quick join feature.
 */
static void onQuickJoinCredentialsObsolete(void) {
    printf("Quick join credentials became obsolete\n");
}

/**
  * @brief  The application entry point.
  */
int main(void) {
    // Initialize hardware
    init();
    gpio_init();

    printf("+---------------------------------------------+\n"
           "   embeNET Node demo for NRF52 DK board        \n"
           "+---------------------------------------------+\n");

    // Initialize structure to provide Stack with user-defined event handlers
    const static EMBENET_NODE_EventHandlers handlers = {.onJoined                       = onJoined,
                                                        .onLeft                         = onLeft,
                                                        .onJoinAttempt                  = onJoinAttempt,
                                                        .onDataOnUnregisteredPort       = dataOnUregisteredPort,
                                                        .onQuickJoinCredentialsObsolete = onQuickJoinCredentialsObsolete};

    // Initialize network stack
    if (EMBENET_RESULT_OK == EMBENET_NODE_Init(&handlers)) {
        printf("embeNET Node initialized\n");
    } else {
        printf("Failed to initialize embeNET Node\n");
    }
	// Construct 128-bit hardware ID using 64-bit UID (here actually 802.15.4 MAC Address)
    uint8_t  hardwareId[16] = {0x00};
    uint64_t uid            = EMBENET_NODE_GetUID();
    memcpy(hardwareId, (void const*)&uid, sizeof(uid));
    // Initialize ENMS service on its default port. You may specify custom Hardware Identifier
    if (ENMS_NODE_RESULT_OK == ENMS_NODE_Init(&enmsNode, ENMS_DEFAULT_PORT, hardwareId, NULL)) {
        printf("ENMS service initialized\n");
    } else {
        printf("Failed to initialize ENMS service!\n");
    }

#if 1 == IS_ROOT
    printf("Acting as root with UID: 0x%x%08x\n", (unsigned)(EMBENET_NODE_GetUID()>>32), (unsigned)(EMBENET_NODE_GetUID()));
	
    // When the application is built for Root node, start as root instead of joining the network
    EMBENET_NODE_RootStart(NULL, 0);
#else
    printf("Acting as node with UID: 0x%x%08x\n", (unsigned)(EMBENET_NODE_GetUID()>>32), (unsigned)(EMBENET_NODE_GetUID()));

    // Initialize exemplary, user-defined custom service
    custom_service_init();
    // Initialize MQTT-SN service
    mqttsn_client_service_init();

    // Additionally tell the ENMS what services are running
    (void)ENMS_NODE_RegisterService(&enmsNode, "custom", 1);
    (void)ENMS_NODE_RegisterService(&enmsNode, "mqttsn", 1);

    // embeNET network configuration:
    // K1 key, used to authenticate the network node should join and
    // PSK - Node's secret key.
    // Note that the psk value should be preferably stored in secure memory, or be preloaded using custom bootloader.
    EMBENET_NODE_Config config = {
        .k1.val  = {0xc0, 0x8b, 0x76, 0x62, 0x77, 0x09, 0x9e, 0x7d, 0x7e, 0x9c, 0x02, 0x22, 0xf1, 0x68, 0xcc, 0x9e},
        .psk.val = {0x46, 0xd7, 0xdc, 0x94, 0xe8, 0xee, 0x74, 0x96, 0xce, 0xaf, 0x54, 0xa3, 0xab, 0x64, 0xcb, 0xeb},
    };

    printf("Trying to join a network...\n");
    // Make the node join the network
    EMBENET_NODE_Join(&config);
#endif

    while (1) {
        // Periodically call embeNET Node process function.
        EMBENET_NODE_Proc();
        #if 1 != IS_ROOT
            // When acting as Node, run the MQTT-SN service process
            mqttsn_client_service_proc();
        #endif		
    }
}


/**
 * This hander is called by the embeNET stack when a critical error aborts operation.
 */
 __attribute__((noreturn)) void EXPECT_OnAbortHandler(char const* why, char const* file, int line) {
    printf("Program aborted: %s %s:%i\n", why, file, line);
    while(1) {
        ;
    }
    __builtin_unreachable();
}
