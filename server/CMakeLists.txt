set(SOURCES
        src/main.cpp
        src/message_dispatcher.cpp
        src/game_state.cpp
)

set(HEADERS
        include/game/game_server.hpp
        include/game/game_logic.hpp
        include/game/message_dispatcher.hpp
)

add_executable(server ${SOURCES})

target_include_directories(server PRIVATE include)

target_link_libraries(server PRIVATE network_lib shared)
