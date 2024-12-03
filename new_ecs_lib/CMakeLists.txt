cmake_minimum_required(VERSION 3.26.0)

project(new_ecs_lib VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(HEADERS
        include/new_ecs/sparse_array.hpp
        include/new_ecs/registry.hpp
        include/new_ecs/zipper.hpp
)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(examples)
