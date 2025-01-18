cmake_minimum_required(VERSION 3.26.0)

project(RTypeClient VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Define source and header files
set(SOURCES
        src/main.cpp

        src/core/renderer.cpp

        src/scenes/login_scene.cpp
        src/scenes/register_scene.cpp
        src/scenes/main_menu_scene.cpp
        src/scenes/create_lobby_scene.cpp
        src/scenes/lobby_scene.cpp
        src/scenes/lobby_password_scene.cpp
        src/scenes/game_scene.cpp

        src/ui/text_box.cpp
        src/ui/chat_overlay.cpp
)

set(HEADERS
        include/core/controller.hpp
        include/core/event_queue.hpp
        include/core/event_type.hpp
        include/core/main_server.hpp
        include/core/message_dispatcher.hpp
        include/core/my_packet_types.hpp
        include/core/packet_factory.hpp
        include/core/protocol.hpp
        include/core/renderer.hpp
        include/core/resource_manager.hpp
        include/core/service_locator.hpp
        include/core/signal_handler.hpp

        include/scenes/scene.hpp
        include/scenes/scene_manager.hpp
        include/scenes/login_scene.hpp
        include/scenes/register_scene.hpp
        include/scenes/main_menu_scene.hpp
        include/scenes/create_lobby_scene.hpp
        include/scenes/lobby_scene.hpp
        include/scenes/lobby_password_scene.hpp
        include/scenes/game_scene.hpp

        include/ui/ui_element.hpp
        include/ui/button.hpp
        include/ui/text.hpp
        include/ui/text_box.hpp
        include/ui/text_link.hpp
        include/ui/chat_overlay.hpp
)

# Define the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Add network_lib library
target_link_libraries(${PROJECT_NAME} PRIVATE network_lib)

# Add cameron314/concurrentqueue dependency for thread-safe queues
find_package(concurrentqueue REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE concurrentqueue::concurrentqueue)

# Add asio dependency for networking
find_package(asio REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE asio::asio)

# Add SDL2 dependency for rendering
find_package(SDL2 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)

# Add SDL2_ttf dependency for rendering text
find_package(SDL2_ttf)
target_link_libraries(${PROJECT_NAME} PRIVATE sdl_ttf::sdl_ttf)

# Set the output directory for the binary
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Set compile options for Debug and Release modes
target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-g -Wall>
        $<$<CONFIG:Release>:-O3 -Wall>
)
