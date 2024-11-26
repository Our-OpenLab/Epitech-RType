cmake_minimum_required(VERSION 3.26.0)

project(r-type VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(network_lib)
add_subdirectory(ecs_lib)

add_subdirectory(client)
add_subdirectory(server)
