cmake_minimum_required(VERSION 3.26.0)

project(network_lib VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(HEADERS
        include/network/tcp/tcp_client_connection.hpp
        include/network/tcp/tcp_connection.hpp
        include/network/tcp/tcp_connection_interface.hpp
        include/network/tcp/tcp_server_connection.hpp

        include/network/udp/udp_client_connection.hpp
        include/network/udp/udp_connection.hpp
        include/network/udp/udp_connection_interface.hpp
        include/network/udp/udp_server_connection.hpp

        include/network/concurrent_queue.hpp
        include/network/network_client.hpp
        include/network/network_server.hpp
        include/network/owned_packet.hpp
        include/network/protocol.hpp
)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(asio REQUIRED)

target_link_libraries(${PROJECT_NAME} INTERFACE asio::asio)
