cmake_minimum_required(VERSION 3.10)
project(tcp_port_forwarder)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system thread)

include_directories(${Boost_INCLUDE_DIR})

add_executable(tcp_port_forwarder main.cpp session.h server.h)

target_link_libraries(tcp_port_forwarder ${Boost_LIBRARIES})
