cmake_minimum_required(VERSION 3.26.0)

project(RTypeGame VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Define source and header files
set(SOURCES
        src/main.cpp

        src/engine/game_engine.cpp
        src/engine/game_state.cpp
)

set(HEADERS
        include/rtype-game/controller.hpp
        include/rtype-game/event_queue.hpp
        include/rtype-game/event_type.hpp
        include/rtype-game/game_network_server.hpp
        include/rtype-game/main_server.hpp
        include/rtype-game/message_dispatcher.hpp
        include/rtype-game/my_packet_types.hpp
        include/rtype-game/packet_factory.hpp
        include/rtype-game/protocol.hpp
        include/rtype-game/service_container.hpp
        include/rtype-game/signal_handler.hpp

        include/rtype-game/network_messages.hpp

        include/rtype-game/engine/ecs_alias.hpp
        include/rtype-game/engine/game_engine.hpp
        include/rtype-game/engine/game_state.hpp

        include/rtype-game/engine/systems/collision_system.hpp
        include/rtype-game/engine/systems/enemy_movement_system.hpp
        include/rtype-game/engine/systems/enemy_spawn_system.hpp
        include/rtype-game/engine/systems/movement_system.hpp
        include/rtype-game/engine/systems/projectile_system.hpp
        include/rtype-game/engine/systems/player_shooting_system.hpp

        include/rtype-game/handlers/ping_handlers.hpp

        include/rtype-game/player_actions.hpp
)

# Define the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Add network_lib library
target_link_libraries(${PROJECT_NAME} PRIVATE network_lib)

# Add ecs_lib library
target_link_libraries(${PROJECT_NAME} PRIVATE ecs_lib)

# Add cURL dependency for interacting with Kubernetes API
find_package(CURL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)

# Add nlohmann_json dependency for JSON parsing
find_package(nlohmann_json REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# Add cameron314/concurrentqueue dependency for thread-safe queues
find_package(concurrentqueue REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE concurrentqueue::concurrentqueue)

# Add libsodium dependency for encryption
find_package(libsodium REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE libsodium::libsodium)

# Add libpqxx dependency for PostgreSQL
find_package(libpqxx REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE libpqxx::pqxx)

# Add asio dependency for networking
find_package(asio REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE asio::asio)

# Set the output directory for the binary
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Set compile options for Debug and Release modes
target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-g -Wall>
        $<$<CONFIG:Release>:-Wall -g>
)
