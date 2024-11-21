cmake_minimum_required(VERSION 3.29.6)

project(network_lib VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SOURCES
        src/connection.cpp
#        src/protocol.cpp
#        src/serializer.cpp
#        src/udp_client.cpp
#        src/udp_server.cpp
#        src/utils.cpp
)

set(HEADERS
        include/network/protocol.hpp
        include/network/concurrent_queue.hpp
        include/network/client_connection.hpp
        include/network/server_connection.hpp
        include/network/server.hpp
        include/network/client.hpp
#        include/network/serializer.hpp
#        include/network/udp_client.hpp
#        include/network/udp_server.hpp
#        include/network/utils.hpp
)

#add_library(network_lib STATIC ${SOURCES} ${HEADERS})
add_library(${PROJECT_NAME} INTERFACE)

#target_include_directories(network_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(asio REQUIRED)

#target_link_libraries(network_lib PRIVATE asio::asio)
target_link_libraries(${PROJECT_NAME} INTERFACE asio::asio)
