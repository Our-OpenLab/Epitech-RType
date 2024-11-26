cmake_minimum_required(VERSION 3.26.0)

project(network_lib VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(HEADERS
        include/network/protocol.hpp
        include/network/concurrent_queue.hpp
        include/network/client_connection.hpp
        include/network/server_connection.hpp
        include/network/network_server.hpp
        include/network/client.hpp
)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(asio REQUIRED)

target_link_libraries(${PROJECT_NAME} INTERFACE asio::asio)
