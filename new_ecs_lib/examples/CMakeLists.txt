add_executable(new_basic_example basic_example.cpp)

target_link_libraries(new_basic_example PRIVATE network_lib)

target_include_directories(new_basic_example PRIVATE ${CMAKE_SOURCE_DIR}/include)
