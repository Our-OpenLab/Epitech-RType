cmake_minimum_required(VERSION 3.26.0)

project(client VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
        src/main.cpp
        src/core/client.cpp
        src/core/renderer.cpp
        src/core/ping_manager.cpp
        src/core/game_state.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(SDL2 REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${PROJECT_NAME} PRIVATE network_lib ecs_lib shared SDL2::SDL2)
