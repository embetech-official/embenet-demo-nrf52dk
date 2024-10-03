cmake_minimum_required(VERSION 3.21)

add_library(
  embenet_node_port_nrf52832
  embenet_aes128.c
  embenet_brt.c
  embenet_critical_section.c
  embenet_eui64.c
  embenet_port_capabilities.c
  embenet_radio.c
  embenet_random.c
  embenet_timer.c
)

add_library(embetech::embenet_node_port_nrf52832 ALIAS embenet_node_port_nrf52832)

target_include_directories(embenet_node_port_nrf52832 PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/private_include PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_definitions(embenet_node_port_nrf52832 PUBLIC EMBENET_NODE_DEMO_MODE=$<BOOL:${EMBENET_NODE_DEMO_MODE}>)

# configuring and adding nrf52_sdk
set(NORDIC_SDK_DEVICE NRF52832_XXAA)
find_package(nrf5_sdk REQUIRED)
target_include_directories(nrf5_sdk PUBLIC private_include)

find_package(embenet_node_port_interface REQUIRED)
find_package(logger REQUIRED)
find_package(embeutils REQUIRED)
find_package(aes128 REQUIRED)
find_package(ring_buffer REQUIRED)

target_link_libraries(embenet_node_port_nrf52832 PUBLIC embenet_node_port_interface PRIVATE  nrf5_sdk logger embeutils aes128 ring_buffer)