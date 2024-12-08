set(SOURCES
        src/main.cpp
        src/core/message_dispatcher.cpp
        src/engine/game_state.cpp
        src/engine/game_engine.cpp
)

add_executable(server ${SOURCES})

target_include_directories(server PRIVATE include)

target_link_libraries(server PRIVATE network_lib ecs_lib shared)
