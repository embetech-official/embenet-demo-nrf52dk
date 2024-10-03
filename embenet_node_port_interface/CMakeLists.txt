cmake_minimum_required(VERSION 3.21)

project(
  embenet_node_port_interface
  VERSION 1.0.0
  LANGUAGES C
)

add_library(embenet_node_port_interface INTERFACE)
add_library(embetech::embenet_node_port_interface ALIAS embenet_node_port_interface)

target_include_directories(
  embenet_node_port_interface
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/embenet>
)


include(${PROJECT_SOURCE_DIR}/cmake/component_install.cmake)
