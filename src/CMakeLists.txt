cmake_minimum_required(VERSION 3.13.4)

project(SquadMaprota VERSION 2.2)

find_package(Boost REQUIRED)

configure_file(RotaConfig.hpp.in  ${CMAKE_SOURCE_DIR}/generator/RotaConfig.hpp)

add_subdirectory(generator)
#add_subdirectory(optimizer)
