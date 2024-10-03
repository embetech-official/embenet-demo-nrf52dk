/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET Node port for nRF52832
@brief     Random number generator

*/

#include "embenet_random.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wcast-align"
#include "nrf_rng.h"
#pragma GCC diagnostic pop

#include <stdbool.h>
#include <stdint.h>

uint32_t EMBENET_RANDOM_Get(void) {
    uint32_t randomValue = 0;

    for (unsigned i = 0; i < 4; ++i) {
        nrf_rng_error_correction_enable();
        nrf_rng_shorts_enable(NRF_RNG_SHORT_VALRDY_STOP_MASK);
        nrf_rng_event_clear(NRF_RNG_EVENT_VALRDY);
        nrf_rng_task_trigger(NRF_RNG_TASK_START);
        while (false == nrf_rng_event_get(NRF_RNG_EVENT_VALRDY))
            ;
        randomValue |= (uint32_t)nrf_rng_random_value_get() << (i * 8);
    }
    return randomValue;
}
