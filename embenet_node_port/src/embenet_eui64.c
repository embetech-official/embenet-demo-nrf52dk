/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET Node port for nRF52832
@brief     Implementation of the EUI64 handling
 */

#include "embenet_eui64.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wcast-align"
#include "nrf_ficr.h"
#pragma GCC diagnostic pop


#include <stdint.h>
#include <string.h>

#define EUI_SUBSTITUTION 1

#if 1 == EUI_SUBSTITUTION

typedef struct {
    uint64_t native;
    uint64_t humanReadableNumber;
} EuiLookup;

static const EuiLookup lookup[] = {{0x063325ede6c4ea1a, 0x11},  // 682072160
                                   {0x74f023263aa44a59, 0x12},  // 682573125
                                   {0x0, 0x13},                 // not working
                                   {0x12ce152a9fc5fa1c, 0x14},  // 682546473
                                   {0x04a4c2da7ce87b8d, 0x15},  // 682620198
                                   {0xc242ab8cf8e8c28f, 0x16},  // 683898998
                                   {0xbb2e7b06d3ddf474, 0x17},  // 682767942
                                   {0x32d72f87fdf8de90, 0x18},  // 682247480
                                   {0xcbec8549dc63e6ca, 0x19}}; // 682129802
#endif

uint64_t EMBENET_EUI64_Get(void) {
    uint64_t native = (uint64_t)NRF_FICR->DEVICEID[0] + ((uint64_t)NRF_FICR->DEVICEID[1] << 32);
#if 1 == EUI_SUBSTITUTION
    for (size_t i = 0; i != (sizeof(lookup) / sizeof(EuiLookup)); ++i) {
        if (native == lookup[i].native) {
            native = lookup[i].humanReadableNumber;
            break;
        }
    }
#endif
    return native;
}
