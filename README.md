# embeNET Demo for nRF52-DK board

## What's inside ?

This repository contains an exemplary project demonstrating the use of embeNET Suite on [nRF52-DK](https://www.nordicsemi.com/Products/Development-hardware/nRF52-DK) evaluation boards. 
The project is designed to be built using GNU Make.

## What's included ?

The demo includes the following components of the embeNET Suite:
- The embeNET Node library in demo mode, providing IPv6 wireless mesh networking connectivity
- A port of the embeNET Node for the nRF52 chip
- embeNET Node Management Service (ENMS) providing telemetry services
- MQTT-SN client

## What are the demo version limitations ?

The demo *can only be used for evaluation purposes* (see LICENSE.txt for details).
The demo is limited to 10 nodes only (including root node).

## What you'll need to run the demo

- PC with Windows
- One [nRF52-DK](https://www.nordicsemi.com/Products/Development-hardware/nRF52-DK) board connected to the PC (via USB cable) that will act as the root of the network
- At least one more (and up to 9) [nRF52-DK](https://www.nordicsemi.com/Products/Development-hardware/nRF52-DK) boards that will act as the network nodes
- [embeNET demo package for NRF52-DK](https://github.com/embetech-official/embenet-demo-nrf52dk/releases)

To build the project from source code and expand the demo you also need:
- NRF52 SDK, available for download from [the official Nordic Semiconductor site](https://www.nordicsemi.com/Products/Development-software/nrf5-sdk/download) 
- A compiler for ARM Cortex such as [Arm GNU Toolchain](https://developer.arm.com/downloads/-/gnu-rm)

Optionally, to play with the MQTT-SN demo service you'll need:
- MQTT For Small Things (SN) written in Java, available for download from [github](https://github.com/simon622/mqtt-sn)
- MQTT Client Toolbox, available for download from [the official MQTTX page](https://mqttx.app)

Optionally, to easily interact with the custom UDP service you'll need
- [UDP - Sender/Reciever app from Microsoft Store](https://www.microsoft.com/store/apps/9nblggh52bt0)

## How to start?

Read the ['Getting started with embeNET demo for NRF52-DK board'](https://embe.tech/docs/?q=doxyview/Getting%20started%20with%20NRF52-DK/index.html) tutorial.