set(SOURCES
        src/main.cpp
)

add_executable(new_server ${SOURCES})

find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)

target_include_directories(new_server PRIVATE include)

target_link_libraries(new_server PRIVATE nlohmann_json::nlohmann_json CURL::libcurl)
