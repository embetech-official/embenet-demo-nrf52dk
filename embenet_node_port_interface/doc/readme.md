embeNET Port Interface								{#mainpage}
======================
@tableofcontents

This page contains documentation of the embeNET Port Interface.

Introduction
============

The embeNET Node library is portable across various hardware platforms. In order to support this portability the embeNET Node relies on the platform-related code
called the Port. The Port should deliver a defined set of functions and data. This set consists of several APIs that the port needs to realize in order
to allow the embeNET Node library to work. These APIs are Interfaces to several functionalities. 


The embeNET Node Port Interfaces
================================

The embeNET Node Port must provide the following interfaces:
- @ref embenet_node_port_capabilities
- @ref embenet_node_port_critical_section
- @ref embenet_node_port_aes
- @ref embenet_node_port_brt
- @ref embenet_node_port_eui64
- @ref embenet_node_port_radio
- @ref embenet_node_port_random
- @ref embenet_node_port_timer
