/**
@file
@license   Commercial
@copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
@version   1.1.4417
@purpose   embeNET Node port for nRF52832
@brief     Implementation of the radio interface
@note
*/

#include "embenet_radio.h"

#include "embenet_critical_section.h"
#include "embenet_port_config.h"
#include "embenet_port_interrupt_priorities.h"
#include "embenet_port_radio_config.h"
#include "embenet_port_radio_extension.h"
#include "embenet_random.h"
#include "embenet_timer.h"
#include <embetech/expect.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wcast-align"
#include "nrf52.h"
#include "nrf52_bitfields.h"
#include "nrf_clock.h"
#include "nrf_radio.h"
#pragma GCC diagnostic pop

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define CRC_LENGTH     2        ///< Length of CRC in 802.15.4 frames [bytes]
#define CRC_POLYNOMIAL 0x011021 ///< Polynomial used for CRC calculation in 802.15.4 frames

/// driver states
typedef enum {
    EMBENET_RADIO_STATE_UNINITIALIZED = 0, //!< uninitialized
    EMBENET_RADIO_STATE_IDLE,              //!< idle
    EMBENET_RADIO_STATE_ACTIVE,            //!< active
    EMBENET_RADIO_STATE_TX_PREPARED,       //!< prepared to transmitting
    EMBENET_RADIO_STATE_RX_PREPARED,       //!< prepared to receiving
    EMBENET_RADIO_STATE_TX_CONTINUOUS_PN9, //!< transmitting continuous PN 9 signal
    EMBENET_RADIO_STATE_TX_CONTINUOUS_CW,  //!< transmitting continuous carrier wave signal
    EMBENET_RADIO_STATE_RX_LISTENING,      //!< listening
    EMBENET_RADIO_STATE_RX_FRAME,          //!< receiving state
    EMBENET_RADIO_STATE_TX_FRAME,          //!< transmitting frame
    EMBENET_RADIO_STATE_RX_CAD,            //!< doing channel activity detection
} RadioState;


// structure to hold radio variables and states
typedef struct {
    bool                     lockHfxo;              ///< if true, @EMBENET_RADIO_Idle does not turn off HFXO
    EMBENET_RADIO_CaptureCbt onStartOfFrameHandler; ///< handler to method called when start of frame interrupt occurs
    EMBENET_RADIO_CaptureCbt onEndOfFrameHandler;   ///< handler to method called when end of frame interrupt occurs
    void*                    handlersContext;       ///< context passed to hanlders
    uint8_t                  frame[EMBENET_RADIO_MAX_PSDU_LENGTH + 1];
} Admin;
static Admin admin;

static volatile RadioState state; ///< radio driver state

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function" // warning or error suppress on unused function
static const char*     getStateName(void) {
    switch (state) {
        case EMBENET_RADIO_STATE_UNINITIALIZED: return "UNINITIALIZED";
        case EMBENET_RADIO_STATE_IDLE: return "IDLE";
        case EMBENET_RADIO_STATE_ACTIVE: return "ACTIVE";
        case EMBENET_RADIO_STATE_TX_PREPARED: return "TX_PREPARED";
        case EMBENET_RADIO_STATE_RX_PREPARED: return "RX_PREPARED";
        case EMBENET_RADIO_STATE_TX_CONTINUOUS_PN9: return "TX_CONTINUOUS_PN9";
        case EMBENET_RADIO_STATE_TX_CONTINUOUS_CW: return "TX_CONTINUOUS_CW";
        case EMBENET_RADIO_STATE_RX_LISTENING: return "RX_LISTENING";
        case EMBENET_RADIO_STATE_RX_FRAME: return "RX_FRAME";
        case EMBENET_RADIO_STATE_TX_FRAME: return "TX_FRAME";
        case EMBENET_RADIO_STATE_RX_CAD: return "RX_CAD";
        default: return "UNKNOWN";
    }
}
#pragma GCC diagnostic pop

static inline void setChannel(uint8_t channel) {
    if (channel > EMBENET_RADIO_MAX_CHANNEL) {
        channel = EMBENET_RADIO_MAX_CHANNEL;
    }
    if (channel <= 10) {
        nrf_radio_frequency_set((uint16_t)(2404 + channel * 2));
    }
    if ((channel >= 11) && (channel <= 36)) {
        nrf_radio_frequency_set((uint16_t)(2406 + channel * 2));
    }
    if (37 == channel) {
        nrf_radio_frequency_set(2402);
    }
    if (38 == channel) {
        nrf_radio_frequency_set(2426);
    }
    if (39 == channel) {
        nrf_radio_frequency_set(2480);
    }
}

static inline void setPower(int8_t power) {
    if (power < EMBENET_RADIO_MIN_OUTPUT_POWER) {
        power = EMBENET_RADIO_MIN_OUTPUT_POWER;
    }
    if (power > EMBENET_RADIO_MAX_OUTPUT_POWER) {
        power = EMBENET_RADIO_MAX_OUTPUT_POWER;
    }
    NRF_RADIO->TXPOWER = (uint32_t)power;
}

/*
There are separate functions for setting, waiting and turning off the HFXO.
HFXO is turned on in @ref EMBENET_RADIO_EnableTx and in @ref EMBENET_RADIO_EnableRx.
HFXO is checked against being stable in @ref EMBENET_RADIO_TxNow and @ref EMBENET_RADIO_RxNow.
Between these two groups of functions, the stack does some work, during which the HFXO can stabilize in the background.
 */

static inline void hfclkWait(void) {
    while (!nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED))
        ;
}

static inline void hfclkTurnOn(void) {
    nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
    nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTART);
}

static inline void hfclkTurnOff(void) {
    nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTOP);
}


void EMBENET_RADIO_LockHfxo(void) {
    hfclkTurnOn();
    hfclkWait();
    admin.lockHfxo = true;
}

void EMBENET_RADIO_UnlockHfxo(void) {
    admin.lockHfxo = false;
}

EMBENET_RADIO_Status EMBENET_RADIO_Init(void) {
    // default state
    EMBENET_RADIO_Deinit();

    // reset peripheral
    nrf_radio_power_set(false);
    nrf_radio_power_set(true);

    // shorts
    nrf_radio_shorts_enable(NRF_RADIO_SHORT_READY_START_MASK);       // shortcut between READY event and START task
    nrf_radio_shorts_enable(NRF_RADIO_SHORT_END_DISABLE_MASK);       // shortcut between END event and DISABLE task
    nrf_radio_shorts_enable(NRF_RADIO_SHORT_ADDRESS_RSSISTART_MASK); // shortcut between ADDRESS event and RSSISTART task

    // modulation
    nrf_radio_mode_set(NRF_RADIO_MODE_BLE_1MBIT);

    // packet config
    nrf_radio_packet_conf_t packetConf = {
        .lflen      = 8,                             // LENGTH field is 8 bit long
        .s0len      = 0,                             // S0 field disabled
        .s1len      = 0,                             // S1 field disabled
        .s1incl     = 0,                             // S1 field not included
        .plen       = 0,                             // 8 bit preamble
        .maxlen     = EMBENET_RADIO_MAX_PSDU_LENGTH, // maximum packet length
        .statlen    = 0,                             // static length disabled
        .balen      = 3,                             // base address has 3B (+1B of prefix)
        .big_endian = false,                         // transmission bit order
        .whiteen    = true                           // no whitening at the beginning};
    };
    nrf_radio_packet_configure(&packetConf);
    nrf_radio_base0_set(0xE7E7E7E7); // configure radio address registers (same as Nordic ESB default values)
    nrf_radio_base1_set(0x43434343);
    nrf_radio_prefix0_set(0x23C343E7);
    nrf_radio_prefix1_set(0x13E363A3);
    nrf_radio_txaddress_set(0);
    nrf_radio_rxaddresses_set(1 << 0);
    nrf_radio_crc_configure(CRC_LENGTH, NRF_RADIO_CRC_ADDR_SKIP, CRC_POLYNOMIAL);
    nrf_radio_crcinit_set(0xFFFFFFFF);
    nrf_radio_modecnf0_set(true, 2);
    nrf_radio_packetptr_set(admin.frame);

    // interrupts
    nrf_radio_int_enable(NRF_RADIO_INT_ADDRESS_MASK); // address (SFD) sent or received
    nrf_radio_int_enable(NRF_RADIO_INT_END_MASK);     // packet sent or received
                                                      //    nrf_radio_int_enable(NRF_RADIO_INT_CRCERROR_MASK);
                                                      //    nrf_radio_int_enable(NRF_RADIO_INT_CRCOK_MASK);


    NVIC_ClearPendingIRQ(RADIO_IRQn);
    NVIC_SetPriority(RADIO_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), INTERRUPT_PRIORITY_RADIO, INTERRUPT_SUBPRIORITY_RADIO));
    NVIC_EnableIRQ(RADIO_IRQn);

    state = EMBENET_RADIO_STATE_IDLE;

    return EMBENET_RADIO_STATUS_SUCCESS;
}

void EMBENET_RADIO_SetCallbacks(EMBENET_RADIO_CaptureCbt onStartFrame, EMBENET_RADIO_CaptureCbt onEndFrame, void* cbtContext) {
    admin.onStartOfFrameHandler = onStartFrame;
    admin.onEndOfFrameHandler   = onEndFrame;
    admin.handlersContext       = cbtContext;
}

void EMBENET_RADIO_Deinit(void) {
    admin.onStartOfFrameHandler = NULL;
    admin.onEndOfFrameHandler   = NULL;
    admin.lockHfxo              = false;
    state                       = EMBENET_RADIO_STATE_UNINITIALIZED;
}

EMBENET_RADIO_Status EMBENET_RADIO_Idle(void) {
    nrf_radio_task_trigger(NRF_RADIO_TASK_DISABLE);
    if (!admin.lockHfxo) {
        hfclkTurnOff();
    }

    state = EMBENET_RADIO_STATE_IDLE;

    return EMBENET_RADIO_STATUS_SUCCESS;
}

EMBENET_RADIO_Status EMBENET_RADIO_TxEnable(EMBENET_RADIO_Channel channel, EMBENET_RADIO_Power txp, uint8_t const* psdu, size_t psduLen) {
    if (!admin.lockHfxo) {
        hfclkTurnOn();
    }

    setChannel(channel);
    setPower(txp);

    // this short may be disabled or enabled by @EMBENET_RADIO_StartContinuousTx
    nrf_radio_shorts_enable(NRF_RADIO_SHORT_READY_START_MASK);
    nrf_radio_shorts_disable(NRF_RADIO_SHORT_END_START_MASK);
    nrf_radio_shorts_enable(NRF_RADIO_SHORT_END_DISABLE_MASK);


    if (psduLen > EMBENET_RADIO_MAX_PSDU_LENGTH) {
        psduLen = EMBENET_RADIO_MAX_PSDU_LENGTH;
    }
    if (psduLen < EMBENET_RADIO_MIN_PSDU_LENGTH) {
        return EMBENET_RADIO_STATUS_GENERAL_ERROR;
    }
    admin.frame[0] = (uint8_t)psduLen;
    memcpy(&admin.frame[1], psdu, psduLen);

    state = EMBENET_RADIO_STATE_TX_PREPARED;

    return EMBENET_RADIO_STATUS_SUCCESS;
}

EMBENET_RADIO_Status EMBENET_RADIO_TxNow(void) {
    hfclkWait();

    nrf_radio_task_trigger(NRF_RADIO_TASK_TXEN);

    state = EMBENET_RADIO_STATE_TX_FRAME;

    return EMBENET_RADIO_STATUS_SUCCESS;
}

EMBENET_RADIO_Status EMBENET_RADIO_RxEnable(EMBENET_RADIO_Channel channel) {
    if (!admin.lockHfxo) {
        hfclkTurnOn();
    }

    setChannel(channel);
    // this short may be disabled or enabled by @EMBENET_RADIO_StartContinuousTx
    nrf_radio_shorts_enable(NRF_RADIO_SHORT_READY_START_MASK);
    nrf_radio_shorts_disable(NRF_RADIO_SHORT_END_START_MASK);
    nrf_radio_shorts_enable(NRF_RADIO_SHORT_END_DISABLE_MASK);

    state = EMBENET_RADIO_STATE_RX_PREPARED;

    return EMBENET_RADIO_STATUS_SUCCESS;
}

EMBENET_RADIO_Status EMBENET_RADIO_RxNow(void) {
    hfclkWait();

    nrf_radio_task_trigger(NRF_RADIO_TASK_RXEN);

    state = EMBENET_RADIO_STATE_RX_LISTENING;

    return EMBENET_RADIO_STATUS_SUCCESS;
}

EMBENET_RADIO_RxInfo EMBENET_RADIO_GetReceivedFrame(uint8_t* buffer, size_t bufferLength) {
    EMBENET_RADIO_RxInfo info             = {.crcValid = nrf_radio_crc_status_check(), .lqi = 0, .mpduLength = admin.frame[0], .rssi = (int8_t)(-nrf_radio_rssi_sample_get())};
    size_t               dataAmountToCopy = info.mpduLength;
    if (dataAmountToCopy > bufferLength) {
        dataAmountToCopy = bufferLength;
    }
    memcpy(buffer, &admin.frame[1], dataAmountToCopy);

    return info;
}


EMBENET_RADIO_Status EMBENET_RADIO_StartContinuousTx(EMBENET_RADIO_ContinuousTxMode mode, EMBENET_RADIO_Channel channel, EMBENET_RADIO_Power txp) {
    switch (mode) {
        case EMBENET_RADIO_CONTINUOUS_TX_MODE_PN9:
            hfclkWait();
            admin.frame[0] = EMBENET_RADIO_MAX_PSDU_LENGTH;
            for (size_t i = 1; i < sizeof(admin.frame); ++i) {
                admin.frame[i] = (uint8_t)EMBENET_RANDOM_Get();
            }

            setPower(txp);
            setChannel(channel);
            nrf_radio_shorts_enable(NRF_RADIO_SHORT_END_START_MASK);
            nrf_radio_shorts_disable(NRF_RADIO_SHORT_END_DISABLE_MASK);
            nrf_radio_task_trigger(NRF_RADIO_TASK_TXEN);

            state = EMBENET_RADIO_STATE_TX_CONTINUOUS_PN9;
            break;
        case EMBENET_RADIO_CONTINUOUS_TX_MODE_CARRIER:
            hfclkWait();
            setPower(txp);
            setChannel(channel);
            nrf_radio_shorts_disable(NRF_RADIO_SHORT_READY_START_MASK);
            nrf_radio_task_trigger(NRF_RADIO_TASK_TXEN);

            state = EMBENET_RADIO_STATE_TX_CONTINUOUS_CW;
            break;
        default: return EMBENET_RADIO_STATUS_GENERAL_ERROR;
    }

    return EMBENET_RADIO_STATUS_SUCCESS;
}

EMBENET_RADIO_Capabilities const* EMBENET_RADIO_GetCapabilities(void) {
    static EMBENET_RADIO_Capabilities timings = {.idleToTxReady   = EMBENET_RADIO_IdleToTxReady,
                                                 .idleToRxReady   = EMBENET_RADIO_IdleToRxReady,
                                                 .activeToTxReady = EMBENET_RADIO_ActiveToTxReady,
                                                 .activeToRxReady = EMBENET_RADIO_ActiveToRxReady,
                                                 .txDelay         = EMBENET_RADIO_TxDelay,
                                                 .rxDelay         = EMBENET_RADIO_RxDelay,
                                                 .txRxStartDelay  = EMBENET_RADIO_TX_RX_START_CORRECTION,
                                                 .sensitivity     = EMBENET_RADIO_SENSITIVITY,
                                                 .maxOutputPower  = EMBENET_RADIO_MAX_OUTPUT_POWER,
                                                 .minOutputPower  = EMBENET_RADIO_MIN_OUTPUT_POWER};
    return &timings;
}


void RADIO_IRQHandler(void) {
    EMBENET_TimeUs now = EMBENET_TIMER_ReadCounter();
    if (nrf_radio_event_check(NRF_RADIO_EVENT_ADDRESS)) {
        nrf_radio_event_clear(NRF_RADIO_EVENT_ADDRESS);
        if (admin.onStartOfFrameHandler && (EMBENET_RADIO_STATE_TX_CONTINUOUS_PN9 != state)) {
            if (EMBENET_RADIO_STATE_TX_FRAME == state) {
                now -= EMBENET_RADIO_TX_TX_START_CORRECTION;
            }
            if (EMBENET_RADIO_STATE_RX_LISTENING == state) {
                state = EMBENET_RADIO_STATE_RX_FRAME;
                now -= EMBENET_RADIO_TX_RX_START_CORRECTION;
            }
            admin.onStartOfFrameHandler(admin.handlersContext, now);
        }
    }
    if (nrf_radio_event_check(NRF_RADIO_EVENT_END)) {
        nrf_radio_event_clear(NRF_RADIO_EVENT_END);
        if (EMBENET_RADIO_STATE_RX_FRAME == state) {
            now -= EMBENET_RADIO_TX_RX_END_CORRECTION;
        }
        if (admin.onEndOfFrameHandler && (EMBENET_RADIO_STATE_TX_CONTINUOUS_PN9 != state)) {
            admin.onEndOfFrameHandler(admin.handlersContext, now);
        }
    }
}
