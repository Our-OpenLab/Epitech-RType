cmake_minimum_required(VERSION 3.26.0)

project(shared VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(HEADERS
        include/shared/my_packet_types.hpp
        include/shared/network_messages.hpp
        include/shared/player_actions.hpp
        include/shared/components.hpp
        include/shared/vector2.hpp
        include/shared/vector3.hpp
)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
