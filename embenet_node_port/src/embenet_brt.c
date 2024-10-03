/**
 @file
 @license   Commercial
 @copyright (c) 2023 EMBETECH SP. Z O.O. All rights reserved.
 @version   1.1.4417
 @purpose   embeNET Node port for nRF52832
 @brief     Implementation of the border router communication

 */

#include "embenet_brt.h"

#include "embenet_critical_section.h"
#include "embenet_node_nrf52_bsp_interface.h"
#include "embenet_port_config.h"
#include "ring_buffer.h"

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#define HDLC_FLAG        0x7e
#define HDLC_ESCAPE      0x7d
#define HDLC_ESCAPE_MASK 0x20
#define HDLC_CRCINIT     0xffff
#define HDLC_CRCGOOD     0xf0b8

/**
 * @brief This table is used to expedite execution (at the expense of memory usage).
 */
static const uint16_t fcstab[256] = {0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf, 0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7, 0x1081, 0x0108, 0x3393, 0x221a,
                                     0x56a5, 0x472c, 0x75b7, 0x643e, 0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876, 0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
                                     0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5, 0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c, 0xbdcb, 0xac42, 0x9ed9, 0x8f50,
                                     0xfbef, 0xea66, 0xd8fd, 0xc974, 0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb, 0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
                                     0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a, 0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72, 0x6306, 0x728f, 0x4014, 0x519d,
                                     0x2522, 0x34ab, 0x0630, 0x17b9, 0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1, 0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
                                     0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70, 0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7, 0x0840, 0x19c9, 0x2b52, 0x3adb,
                                     0x4e64, 0x5fed, 0x6d76, 0x7cff, 0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036, 0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
                                     0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5, 0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd, 0xb58b, 0xa402, 0x9699, 0x8710,
                                     0xf3af, 0xe226, 0xd0bd, 0xc134, 0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c, 0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
                                     0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb, 0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232, 0x5ac5, 0x4b4c, 0x79d7, 0x685e,
                                     0x1ce1, 0x0d68, 0x3ff3, 0x2e7a, 0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1, 0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
                                     0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330, 0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78};

uint16_t openhdlc_crc(uint16_t crc, uint8_t byte) {
    return (crc >> 8) ^ fcstab[(crc ^ byte) & 0xff];
}

/* For openserial_asynch */
#define OUTPUT_RING_BUFFER_SIZE 256 /*< output buffer size, in worst case a frame and a status frame could be send during one time slot */
#define INPUT_RING_BUFFER_SIZE  512 /*< input buffer size */


// ring buffer descriptors
RingBuffer outputRingBufferDesc;
RingBuffer inputRingBufferDesc;

// input and output buffers
uint8_t outputRingBuffer[OUTPUT_RING_BUFFER_SIZE];
uint8_t inputRingBuffer[INPUT_RING_BUFFER_SIZE];


// true if transmission is in progress, otherwise false
volatile bool isTransmitting;

/**
 * @brief Sends byte through UART. Transmission is buffered. This function automatically handles transmission triggering.
 * @param[in] openstack_vars pointer to OpenWSN stack instance
 * @param[in] data Byte to send.
 */
static void uartWrite(const uint8_t data);

/**
 * @brief Sends data byte through UART. Automatically handles special characters recognition and substitution.
 * @param[in] openstack_vars pointer to OpenWSN stack instance
 * @param[in] byte Byte to send.
 */
static void putByte(const uint8_t byte);

/**
 * @brief Sends flag byte through UART.
 * @param[in] openstack_vars pointer to OpenWSN stack instance
 * @param[in] flag Flag byte to send.
 */
static void putFlag(const uint8_t flag);

/**
 * @brief TX interrupt handler.
 */
void tx_isr(void* context);

/**
 * @brief RX interrupt handler.
 */
void rx_isr(void* context);


void EMBENET_BRT_Init(void) {
    // initialize ring buffers for input and output to store packets inside them
    RingBuffer_Init(&inputRingBufferDesc, inputRingBuffer, INPUT_RING_BUFFER_SIZE);
    RingBuffer_Init(&outputRingBufferDesc, outputRingBuffer, OUTPUT_RING_BUFFER_SIZE);

    isTransmitting = false;
    // set callbacks
    EMBENET_NODE_BSP_UART_Init((EMBENET_NODE_BSP_UART_Callback_t)tx_isr, (EMBENET_NODE_BSP_UART_Callback_t)rx_isr, NULL);
}


void EMBENET_BRT_Deinit(void) {
    // set callbacks
    EMBENET_NODE_BSP_UART_Deinit();
    isTransmitting = false;
    // initialize ring buffers for input and output to store packets inside them
    RingBuffer_Init(&inputRingBufferDesc, inputRingBuffer, INPUT_RING_BUFFER_SIZE);
    RingBuffer_Init(&outputRingBufferDesc, outputRingBuffer, OUTPUT_RING_BUFFER_SIZE);
}


void EMBENET_BRT_Send(const void* packet, size_t packetLength) {
    uint16_t finalCrc = HDLC_CRCINIT;

    uint8_t const* packetBytes = (uint8_t const*)packet;
    // HDLC flag
    putFlag(HDLC_FLAG);
    // address (eui64)
    for (size_t i = 0; i < packetLength; ++i) {
        finalCrc = openhdlc_crc(finalCrc, packetBytes[i]);
        putByte(packetBytes[i]);
    }
    // writing CRC
    finalCrc = (uint16_t)(~finalCrc);
    putByte((uint8_t)((finalCrc >> 0) & 0xff));
    putByte((uint8_t)((finalCrc >> 8) & 0xff));
    putFlag(HDLC_FLAG);
}


size_t EMBENET_BRT_Receive(void* packetBuffer, size_t packetBufferSize) {
    static uint8_t inputFrame[EMBENET_BRT_MAX_FRAME_SIZE];
    static size_t  inputFrameDataIndex;
    static bool    inputFrameReceiving;
    static uint8_t lastDataByte;

    uint8_t dataByte;
    bool    frameComplete = false;

    while (RingBuffer_GetLen(&inputRingBufferDesc) > 0 && frameComplete == false) {
        EMBENET_CRITICAL_SECTION_Enter();
        RingBuffer_GetChar(&inputRingBufferDesc, &dataByte);
        EMBENET_CRITICAL_SECTION_Exit();


        if (false == inputFrameReceiving) {
            // looking for new frame
            if (HDLC_FLAG == dataByte) {
                inputFrameReceiving = true;
                inputFrameDataIndex = 0;
            }
        } else {
            // already receiving frame
            if (HDLC_FLAG == dataByte) {
                if (HDLC_FLAG != lastDataByte) {
                    // finished frame
                    inputFrameReceiving = false;
                    if (inputFrameDataIndex > 2) {
                        frameComplete = true; // one frame at a time
                    } else {
                        // new frame may be ongoing
                        inputFrameReceiving = true;
                        inputFrameDataIndex = 0;
                    }
                } else {
                    // two '~' at a time, repeating frame start
                    inputFrameReceiving = true;
                    inputFrameDataIndex = 0;
                }
            } else {
                // add byte to buffer
                if (inputFrameDataIndex < sizeof(inputFrame)) {
                    if (HDLC_ESCAPE != dataByte) {
                        if (HDLC_ESCAPE != lastDataByte) {
                            inputFrame[inputFrameDataIndex++] = dataByte;
                        } else {
                            inputFrame[inputFrameDataIndex++] = (dataByte ^ HDLC_ESCAPE_MASK);
                        }
                    }
                } else {
                    inputFrameReceiving = false;
                    inputFrameDataIndex = 0;
                }
            }
        }
        lastDataByte = dataByte;
    }

    if (true == frameComplete && inputFrameDataIndex > 0) {
        // full frame has been received, structure:
        // - data: variable bytes count
        // - CRC: 2B
        // check CRC
        uint16_t crc = HDLC_CRCINIT;
        for (size_t index = 0; index < inputFrameDataIndex - 2; ++index) { // two last bytes are CRC bytes
            crc = openhdlc_crc(crc, inputFrame[index]);
        }
        crc = (uint16_t)(~crc);
        if ((((crc >> 0) & 0xff) == inputFrame[inputFrameDataIndex - 2]) && (((crc >> 8) & 0xff) == inputFrame[inputFrameDataIndex - 1])) { // two last bytes are CRC bytes
            // CRC OK!
            size_t returnSize = inputFrameDataIndex - 2;
            // frame received, pretend last HDLC flag belongs to new frame
            inputFrameReceiving = true;
            inputFrameDataIndex = 0;
            // copy data
            if (packetBufferSize >= returnSize) { // data bytes count without CRC
                memcpy(packetBuffer, &inputFrame[0], returnSize);
                return returnSize;
            } else {
                memcpy(packetBuffer, &inputFrame[0], packetBufferSize);
                return 0;
            }
        } else {
            // frame malformed, pretend last HDLC flag belongs to new frame
            inputFrameReceiving = true;
            inputFrameDataIndex = 0;
        }
    }

    return 0;
}

void EMBENET_BRT_SendRaw(const void* data, size_t dataLength) {
    for(size_t i = 0; i < dataLength; ++i) {
        uartWrite(((uint8_t const*)data)[i]);
    }
}

size_t EMBENET_BRT_ReceiveRaw(void* data, size_t dataBufferSize) {
    EMBENET_CRITICAL_SECTION_Enter();
    uint8_t* dataBytes = (uint8_t*)data;
    while(RingBuffer_GetChar(&inputRingBufferDesc, dataBytes)) {
        ++dataBytes;
        if(dataBytes >= (uint8_t*)data + dataBufferSize) {
            break;
        }
    }
    EMBENET_CRITICAL_SECTION_Exit();
    return (size_t)(dataBytes - (uint8_t*)data);
}

void EMBENET_BRT_Reset(void) {
    for (volatile int i = 0; i < 1000000; ++i)
        ;
    NVIC_SystemReset();
}

bool EMBENET_BRT_IsBusy(void) {
    return isTransmitting;
}

void tx_isr(void* context) {
    (void)(context);
    uint8_t chunk;

    EMBENET_CRITICAL_SECTION_Enter();
    // get first byte from output ring buffer
    if (true == RingBuffer_GetChar(&outputRingBufferDesc, &chunk)) {
        // send it by uart

        EMBENET_NODE_BSP_UART_WriteByte(chunk);
    } else {
        // buffer is empty, transmitting is over for now
        isTransmitting = false;
    }
    EMBENET_CRITICAL_SECTION_Exit();
}


void rx_isr(void* context) {
    (void)(context);
    uint8_t rxByte;

    // read byte just received
    rxByte = EMBENET_NODE_BSP_UART_ReadByte();


    EMBENET_CRITICAL_SECTION_Enter();
    RingBuffer_PutChar(&inputRingBufferDesc, rxByte);
    EMBENET_CRITICAL_SECTION_Exit();
}


static void putFlag(uint8_t flag) {
    uartWrite(flag);
}


static void putByte(uint8_t byte) {
    // add byte to buffer
    if (byte == HDLC_FLAG || byte == HDLC_ESCAPE) {
        uartWrite(HDLC_ESCAPE);
        uartWrite(byte ^ HDLC_ESCAPE_MASK);
    } else {
        uartWrite(byte);
    }
}


static void uartWrite(uint8_t data) {
    EMBENET_CRITICAL_SECTION_Enter();
    if (true == isTransmitting) {
        // put to FIFO buffer
        RingBuffer_PutChar(&outputRingBufferDesc, data);

    } else {
        // change transmitting flag state
        isTransmitting = true;
        // put byte to uart
        EMBENET_NODE_BSP_UART_WriteByte(data);
    }
    EMBENET_CRITICAL_SECTION_Exit();
}
