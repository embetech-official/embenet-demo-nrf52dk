cmake_minimum_required(VERSION 3.21)

project(
  embenet_node_port_nrf52832
  LANGUAGES C
  VERSION 0.1.0
)

option(EMBENET_NODE_NRF52832_PORT_BUILD_TESTS "Forces building test cases" OFF)

add_subdirectory(src)

if (PROJECT_IS_TOP_LEVEL OR EMBENET_NODE_NRF52832_PORT_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif ()
