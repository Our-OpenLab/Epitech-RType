set(SOURCES
        src/main.cpp
        src/engine/game_state.cpp
        src/engine/game_engine.cpp
        src/core/custom_network_server.cpp
)

add_executable(server ${SOURCES})

target_include_directories(server PRIVATE include)

target_link_libraries(server PRIVATE network_lib ecs_lib shared)
