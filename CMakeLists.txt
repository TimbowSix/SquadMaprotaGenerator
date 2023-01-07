cmake_minimum_required(VERSION 3.13)

project(SquadMaprota VERSION 2.2 LANGUAGES C CXX)

enable_testing()

find_package(
    Boost 1.81.0 REQUIRED
    COMPONENTS json
)

add_subdirectory(src)
