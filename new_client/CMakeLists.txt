add_executable(new_client bootstrap_exemple.cpp)

find_package(SDL2 REQUIRED)

target_link_libraries(new_client PRIVATE network_lib new_ecs_lib SDL2::SDL2)
