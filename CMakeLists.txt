cmake_minimum_required(VERSION 3.13.0)

project(MapRotaGenerator)

set(SOURCES
    src/main.c
    src/io.c
    src/statistics.c
    src/utils.c
    src/rotaMap.c
    src/data.c
    src/config.c
    src/biom.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

add_subdirectory(lib)

find_package(CURL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)