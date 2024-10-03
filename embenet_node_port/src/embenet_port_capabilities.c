/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET Node port for nRF52832
@brief     Definition of capabilities
 */

#include "embenet_port_capabilities.h"

#include "embenet_timer.h"

const EMBENET_MAC_Timings embenetMacTimings = {
    .TsTxOffsetUs     = 1700,       //
    .TsTxAckDelayUs   = 1000,       //
    .TsLongGTUs       = (1000 / 2), //
    .TsShortGTUs      = (300 / 2),  //
    .TsSlotDurationUs = 5000,       //
    .wdRadioTxUs      = 500,        //
    .wdDataDurationUs = 2000,       //
    .wdAckDurationUs  = 1000        //
};

const size_t  embenetMacChannelListSize = 37;
const uint8_t embenetMacChannelList[]   = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36};

/// fast mode refers to configuration in which node performance is much faster (in cost of expected network scalability)
const size_t  embenetMacAdvChannelListSize = 3;
const uint8_t embenetMacAdvChannelList[]   = {37, 38, 39};

const EMBENET_TimeUs embenetMacTimeCorrectionGuardUs = 200;
const uint32_t       embenetMacKaPeriodSlots         = ((5000000 / embenetMacTimings.TsSlotDurationUs) + 1);
const uint32_t       embenetMacDesyncTimeoutSlots    = ((30000000 / embenetMacTimings.TsSlotDurationUs) + 1);

const bool                      embenetMacTopologyActive = false;
const EMBENET_MAC_TopologyEntry embenetMacTopologyList[] = {{0x1f194c2004, 4}, //
                                                            {0x1f194c2004, 1},
                                                            {4, 5},  //
                                                            {5, 6},  //
                                                            {5, 7},  //
                                                            {5, 8},  //
                                                            {5, 9},  //
                                                            {6, 7},  //
                                                            {6, 8},  //
                                                            {6, 9},  //
                                                            {7, 8},  //
                                                            {7, 9},  //
                                                            {8, 9}}; //

const size_t embenetMacTopologyListEntriesCount = sizeof(embenetMacTopologyList) / sizeof(EMBENET_MAC_TopologyEntry);
