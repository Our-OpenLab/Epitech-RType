add_executable(client main.cpp)

find_package(asio REQUIRED)

target_link_libraries(client PRIVATE network_lib)
