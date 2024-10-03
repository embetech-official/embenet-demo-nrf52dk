/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET Node port for nRF52832
@brief     Implementation of the timer interface
*/

#include "embenet_timer.h"

#include "embenet_critical_section.h"
#include "embenet_port_config.h"
#include "embenet_port_interrupt_priorities.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wcast-align"
#include "nrf_clock.h"
#include "nrf_rtc.h"
#pragma GCC diagnostic pop

#include <stdbool.h>
#include <stdint.h>

#define GUARD_US                   62
#define SCALE_FACTOR               (uint64_t)(1000000000)  // scaling factor for the purpose of reducing integer calculating errors
#define PERIOD_SCALED_UP           (uint64_t)(30517578125) // scaled up period by @ref SCALE_FACTOR
#define TIMER_MAX_VALUE            (uint32_t)0xFFFFFF
#define MAX_COMPARE_DURATION_TICKS 0x7FFFFF
#define MAX_COMPARE_DURATION_US    (EMBENET_TimeUs)(250000000) // max value reduced by MSB of used timer expressed in us (approximation)
#define OVERFLOW_EXTENSION_US      (uint64_t)(512000000)

static EMBENET_TIMER_CompareCallback callback;
static void*                         callbackContext;
static volatile uint64_t             timerValueExtension;
static volatile bool                 softwareIrq;


void EMBENET_TIMER_Init(EMBENET_TIMER_CompareCallback compareCallback, void* context) {
    // Start LFCLK and wait for it to start.
    if ((false == nrf_clock_lf_is_running()) || (NRF_CLOCK_LFCLK_Xtal != nrf_clock_lf_src_get())) {
        nrf_clock_event_clear(NRF_CLOCK_EVENT_LFCLKSTARTED);
        nrf_clock_lf_src_set(NRF_CLOCK_LFCLK_Xtal);
        nrf_clock_task_trigger(NRF_CLOCK_TASK_LFCLKSTART);
        while (nrf_clock_event_check(NRF_CLOCK_EVENT_LFCLKSTARTED))
            ;
    }
    // configure timer
    EMBENET_TIMER_Deinit();
    nrf_rtc_prescaler_set(EMBENET_PORT_TIMER, 0);
    nrf_rtc_event_clear(EMBENET_PORT_TIMER, NRF_RTC_EVENT_OVERFLOW);
    nrf_rtc_event_enable(EMBENET_PORT_TIMER, NRF_RTC_EVENT_OVERFLOW);
    nrf_rtc_int_enable(EMBENET_PORT_TIMER, NRF_RTC_INT_OVERFLOW_MASK);
    nrf_rtc_event_clear(EMBENET_PORT_TIMER, NRF_RTC_EVENT_COMPARE_0);
    nrf_rtc_event_enable(EMBENET_PORT_TIMER, NRF_RTC_EVENT_COMPARE_0);
    nrf_rtc_task_trigger(EMBENET_PORT_TIMER, NRF_RTC_TASK_START);

    NVIC_ClearPendingIRQ(EMBENET_PORT_TIMER_IRQn);
    NVIC_SetPriority(EMBENET_PORT_TIMER_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), INTERRUPT_PRIORITY_EMBENET_TIMER, INTERRUPT_SUBPRIORITY_EMBENET_TIMER));
    NVIC_EnableIRQ(EMBENET_PORT_TIMER_IRQn);

    callback        = compareCallback;
    callbackContext = context;

    timerValueExtension = 0;
    softwareIrq         = false;
}


void EMBENET_TIMER_Deinit(void) {
    nrf_rtc_event_disable(EMBENET_PORT_TIMER, NRF_RTC_EVENT_COMPARE_0);
    nrf_rtc_task_trigger(EMBENET_PORT_TIMER, NRF_RTC_TASK_STOP);
    nrf_rtc_task_trigger(EMBENET_PORT_TIMER, NRF_RTC_TASK_CLEAR);
}


void EMBENET_TIMER_SetCompare(EMBENET_TimeUs compareValue) {
    EMBENET_CRITICAL_SECTION_Enter();
    // cancel previous or ongoing
    nrf_rtc_int_enable(EMBENET_PORT_TIMER, NRF_RTC_INT_COMPARE0_MASK); // make sure that the interrupt is active (first compare)

    // get current time and counter
    uint32_t       counter = nrf_rtc_counter_get(EMBENET_PORT_TIMER);
    EMBENET_TimeUs now     = (EMBENET_TimeUs)((uint64_t)counter * PERIOD_SCALED_UP / SCALE_FACTOR + timerValueExtension);
    if (nrf_rtc_event_pending(EMBENET_PORT_TIMER, NRF_RTC_EVENT_OVERFLOW)) {
        counter = nrf_rtc_counter_get(EMBENET_PORT_TIMER);
        now     = (EMBENET_TimeUs)((uint64_t)counter * PERIOD_SCALED_UP / SCALE_FACTOR + timerValueExtension + OVERFLOW_EXTENSION_US);
    }

    EMBENET_TimeUs delta = compareValue - now;
    if ((EMBENET_TimeUs)(delta - GUARD_US) < MAX_COMPARE_DURATION_US) {
        // arm the compare interrupt
        nrf_rtc_cc_set(EMBENET_PORT_TIMER, 0, (counter + (uint32_t)((uint64_t)(delta)*SCALE_FACTOR / PERIOD_SCALED_UP)) & TIMER_MAX_VALUE);
    } else {
        // compareValue is too close to current value and timer will be late, interrupt is triggered immediately
        NVIC_SetPendingIRQ(EMBENET_PORT_TIMER_IRQn);
        softwareIrq = true;
    }

    EMBENET_CRITICAL_SECTION_Exit();
}


EMBENET_TimeUs EMBENET_TIMER_ReadCounter(void) {
    // note that during assembly of the actual time, the counter may increment and overflow itself
    EMBENET_CRITICAL_SECTION_Enter();
    // this thing is sopthisticated
    // it can be called from nonpriviledged mode (1) and from @ref EMBENET_PORT_TIMER_IRQ_HANDLER ISR (2)
    // (1A) after entering the critical section and before getting the timer value it can increment and overflow,
    // OVF flag will be set and timerValueExtension will be not incremented - OVF FLAG MUST BE CHECKED!
    // (1B) during code execution inside the critical section timer may increment and overflow itself - no harm
    // (2) before entering critical section timerValueExtension may not be incremented and OVF flag will be set - OVF FLAG MUST BE CHECKED!
    uint64_t now = (uint64_t)nrf_rtc_counter_get(EMBENET_PORT_TIMER) * PERIOD_SCALED_UP / SCALE_FACTOR + timerValueExtension;
    if (nrf_rtc_event_pending(EMBENET_PORT_TIMER, NRF_RTC_EVENT_OVERFLOW)) {
        now = (uint64_t)nrf_rtc_counter_get(EMBENET_PORT_TIMER) * PERIOD_SCALED_UP / SCALE_FACTOR + timerValueExtension + OVERFLOW_EXTENSION_US;
    }
    EMBENET_CRITICAL_SECTION_Exit();

    return (EMBENET_TimeUs)(now);
}


EMBENET_TimeUs EMBENET_TIMER_GetMaxCompareDuration(void) {
    return (EMBENET_TimeUs)MAX_COMPARE_DURATION_US;
}


void EMBENET_PORT_TIMER_IRQ_HANDLER(void) {
    if (nrf_rtc_event_pending(EMBENET_PORT_TIMER, NRF_RTC_EVENT_OVERFLOW)) {
        nrf_rtc_event_clear(EMBENET_PORT_TIMER, NRF_RTC_EVENT_OVERFLOW);
        timerValueExtension += (uint64_t)OVERFLOW_EXTENSION_US;
    }
    if (nrf_rtc_event_pending(EMBENET_PORT_TIMER, NRF_RTC_EVENT_COMPARE_0) || softwareIrq) {
        nrf_rtc_event_clear(EMBENET_PORT_TIMER, NRF_RTC_EVENT_COMPARE_0);
        nrf_rtc_int_disable(EMBENET_PORT_TIMER, NRF_RTC_INT_COMPARE0_MASK);
        softwareIrq = false;

        if (callback != NULL) {
            callback(callbackContext);
        }
    }
}
