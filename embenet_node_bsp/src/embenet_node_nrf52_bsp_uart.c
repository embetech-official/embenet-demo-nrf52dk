/**
@file
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET Node BSP for nRF52832
@brief     Implementation of the HDLC uart communication
*/

#include "bsp_interrupt_priorities.h"

#include <embenet_node_nrf52_bsp_interface.h>
#include <embenet_port_radio_extension.h>
#include <embetech/expect.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wcast-align"
#include <nrf_clock.h>
#include <nrf_uart.h>
#pragma GCC diagnostic pop

#include <stddef.h>

#define HDLC_UART          NRF_UART0                /* UART instance */
#define HDLC_UART_IRQn     UARTE0_UART0_IRQn        /* IRQ number */
#define HDLC_UART_TXPIN    6                        /* USART transmission pin */
#define HDLC_UART_RXPIN    8                        /* USART reception pin */
#define HDLC_UART_BAUDRATE NRF_UART_BAUDRATE_115200 /* USART baudrate */

static EMBENET_NODE_BSP_UART_Callback_t txCb;
static EMBENET_NODE_BSP_UART_Callback_t rxCb;
static void*                            context;

void EMBENET_NODE_BSP_UART_Init(const EMBENET_NODE_BSP_UART_Callback_t txEndCallback, const EMBENET_NODE_BSP_UART_Callback_t rxCallback, void* cbContext) {
    // firstly lock external oscillator
    EMBENET_RADIO_LockHfxo();

    // Start HFCLK and wait for it to start.
    nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
    nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTART);
    while (!nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED))
        ;

    txCb    = txEndCallback;
    rxCb    = rxCallback;
    context = cbContext;

    nrf_uart_baudrate_set(HDLC_UART, HDLC_UART_BAUDRATE);
    nrf_uart_txrx_pins_set(HDLC_UART, HDLC_UART_TXPIN, HDLC_UART_RXPIN);
    nrf_uart_configure(HDLC_UART, NRF_UART_PARITY_EXCLUDED, NRF_UART_HWFC_DISABLED);
    nrf_uart_errorsrc_get_and_clear(HDLC_UART);

    NVIC_SetPriority(HDLC_UART_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), INTERRUPT_PRIORITY_HDLC_UART, INTERRUPT_SUBPRIORITY_HDLC_UART));
    NVIC_EnableIRQ(HDLC_UART_IRQn);

    nrf_uart_event_clear(HDLC_UART, NRF_UART_EVENT_TXDRDY);
    nrf_uart_int_enable(HDLC_UART, NRF_UART_INT_MASK_TXDRDY);
    nrf_uart_event_clear(HDLC_UART, NRF_UART_EVENT_RXDRDY);
    nrf_uart_int_enable(HDLC_UART, NRF_UART_INT_MASK_RXDRDY);

    nrf_uart_task_trigger(HDLC_UART, NRF_UART_TASK_STARTRX);
    nrf_uart_task_trigger(HDLC_UART, NRF_UART_TASK_STARTTX);


    nrf_uart_enable(HDLC_UART);
}


void EMBENET_NODE_BSP_UART_Deinit(void) {
    nrf_uart_disable(HDLC_UART);
    nrf_uart_hwfc_pins_disconnect(HDLC_UART);
    nrf_uart_txrx_pins_disconnect(HDLC_UART);
    nrf_uart_int_disable(HDLC_UART, NRF_UART_INT_MASK_TXDRDY);
    nrf_uart_int_disable(HDLC_UART, NRF_UART_INT_MASK_RXDRDY);
    NVIC_DisableIRQ(HDLC_UART_IRQn);

    txCb    = NULL;
    rxCb    = NULL;
    context = NULL;
    EMBENET_RADIO_UnlockHfxo();
}


void EMBENET_NODE_BSP_UART_WriteByte(uint8_t byteToWrite) {
    nrf_uart_txd_set(HDLC_UART, byteToWrite);
} /* uart_writeByte */

uint8_t EMBENET_NODE_BSP_UART_ReadByte(void) {
    return nrf_uart_rxd_get(HDLC_UART);
}


void UARTE0_UART0_IRQHandler(void) {
    if (!nrf_clock_hf_is_running(NRF_CLOCK_HFCLK_HIGH_ACCURACY)) {
        EXPECT_ABORT("UART without HFXO will not work properly");
    }

    if (nrf_uart_event_check(HDLC_UART, NRF_UART_EVENT_RXDRDY)) {
        nrf_uart_event_clear(HDLC_UART, NRF_UART_EVENT_RXDRDY);
        if (rxCb) {
            rxCb(context);
        }
    } else if (nrf_uart_event_check(HDLC_UART, NRF_UART_EVENT_TXDRDY)) {
        nrf_uart_event_clear(HDLC_UART, NRF_UART_EVENT_TXDRDY);
        if (txCb) {
            txCb(context);
        }
    }
}
