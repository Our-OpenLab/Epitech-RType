add_executable(basic_example basic_example.cpp)

target_link_libraries(basic_example PRIVATE network_lib)

target_include_directories(basic_example PRIVATE ${CMAKE_SOURCE_DIR}/include)
