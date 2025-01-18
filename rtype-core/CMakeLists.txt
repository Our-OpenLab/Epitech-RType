cmake_minimum_required(VERSION 3.26.0)

project(RTypeCore VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Define source and header files
set(SOURCES
        src/game_state.cpp
        src/main.cpp

        src/database/database.cpp

        src/database/dao/user_dao.cpp
        src/database/dao/message_dao.cpp
        src/database/dao/lobby_dao.cpp
        src/database/dao/lobby_player_dao.cpp

       # src/domain/entities/user.cpp

        src/domain/repositories/user_repository.cpp
        src/domain/repositories/message_repository.cpp
        src/domain/repositories/lobby_repository.cpp
        src/domain/repositories/lobby_player_repository.cpp

        src/domain/services/user_service.cpp
        src/domain/services/message_service.cpp
        src/domain/services/lobby_service.cpp
        src/domain/services/lobby_player_service.cpp
)

set(HEADERS
        include/database/database.hpp

        include/database/dao/user_dao.hpp
        include/database/dao/message_dao.hpp
        include/database/dao/lobby_dao.hpp
        include/database/dao/lobby_player_dao.hpp

        include/domain/entities/user.hpp
        include/domain/entities/message.hpp
        include/domain/entities/lobby.hpp
        include/domain/entities/lobby_player.hpp

        include/domain/repositories/user_repository_interface.hpp
        include/domain/repositories/user_repository.hpp
        include/domain/repositories/message_repository_interface.hpp
        include/domain/repositories/message_repository.hpp
        include/domain/repositories/lobby_repository_interface.hpp
        include/domain/repositories/lobby_repository.hpp
        include/domain/repositories/lobby_player_repository_interface.hpp
        include/domain/repositories/lobby_player_repository.hpp

        include/domain/services/user_service.hpp
        include/domain/services/message_service.hpp
        include/domain/services/lobby_service.hpp
        include/domain/services/lobby_player_service.hpp

        include/rtype-core/controller.hpp
        include/rtype-core/event_queue.hpp
        include/rtype-core/event_type.hpp
        include/rtype-core/game_network_server.hpp
        include/rtype-core/game_state.hpp
        include/rtype-core/main_server.hpp
        include/rtype-core/message_dispatcher.hpp
        include/rtype-core/my_packet_types.hpp
        include/rtype-core/packet_factory.hpp
        include/rtype-core/protocol.hpp
        include/rtype-core/service_container.hpp
        include/rtype-core/signal_handler.hpp

        include/rtype-core/handlers/user_register_handler.hpp
        include/rtype-core/handlers/user_login_handler.hpp
        include/rtype-core/handlers/private_message_handler.hpp
        include/rtype-core/handlers/create_lobby_handler.hpp
        include/rtype-core/handlers/player_ready_handler.hpp
        include/rtype-core/handlers/get_user_list_handler.hpp
        include/rtype-core/handlers/private_chat_history_handler.hpp
        include/rtype-core/handlers/get_lobby_players_handler.hpp
        include/rtype-core/handlers/leave_lobby_handler.hpp
        include/rtype-core/handlers/join_lobby_handler.hpp
        include/rtype-core/handlers/get_lobby_list_handler.hpp
)

# Define the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Add network_lib library
target_link_libraries(${PROJECT_NAME} PRIVATE network_lib)

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
