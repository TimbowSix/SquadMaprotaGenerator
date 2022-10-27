cmake_minimum_required(VERSION 3.13.0)

project(MapRotaGenerator)

include_directories(include)

set(SOURCES
    src/main.c
)

add_executable(${PROJECT_NAME} ${SOURCES})