cmake_minimum_required(VERSION 3.13.0)

project(MapRotaGenerator)

set(SOURCES
    src/main.c
    src/io.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

add_subdirectory(lib)