cmake_minimum_required(VERSION 3.26.0)

project(client VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
        src/main.cpp
        src/core/client.cpp
        src/core/message_dispatcher.cpp
        src/core/renderer.cpp
        src/engine/game_engine.cpp
        src/engine/game_state.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLM REQUIRED)
find_package(OpenGL REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${PROJECT_NAME} PRIVATE network_lib ecs_lib shared SDL2::SDL2 GLEW::GLEW OpenGL::GL glm::glm)
