cmake_minimum_required(VERSION 3.26.0)

project(r-type VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(network_lib)
add_subdirectory(ecs_lib)
add_subdirectory(shared)

# add_subdirectory(client)
# add_subdirectory(server)

# add_subdirectory(new_server)

add_subdirectory(rtype-core)
add_subdirectory(rtype-client)
