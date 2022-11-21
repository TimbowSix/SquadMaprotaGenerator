cmake_minimum_required(VERSION 3.13.0)

project(MapRotaGenerator)

set(INCLUDE_DIRECTORIES
    /src
)

set(SOURCES
    src/main.c
    src/io.c
    src/statistics.c
    src/utils.c
    src/rotaMap.c
    src/data.c
    src/config.c
    src/biom.c
    src/generator.c
    src/rotaLayer.c
)
include_directories(INCLUDE_DIRECTORIES)


add_executable(${PROJECT_NAME} ${SOURCES})

add_subdirectory(lib)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libcurl REQUIRED IMPORTED_TARGET libcurl>=7.17.0)
target_link_libraries(${PROJECT_NAME} PUBLIC PkgConfig::libcurl)