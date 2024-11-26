add_executable(client main.cpp)

find_package(SDL2 REQUIRED)

target_link_libraries(client PRIVATE network_lib SDL2::SDL2)
