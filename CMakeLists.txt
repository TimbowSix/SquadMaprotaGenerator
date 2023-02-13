cmake_minimum_required(VERSION 3.13)

project(SquadMaprota VERSION 3.1.2 LANGUAGES C CXX)

enable_testing()

find_package(
    Boost 1.81.0 REQUIRED
    COMPONENTS json
)

add_subdirectory(src)
add_subdirectory(external)

install(
  FILES config.json data/bioms.json
  PERMISSIONS WORLD_READ WORLD_WRITE
  DESTINATION /etc/maprota
  )

include(InstallRequiredSystemLibraries)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CPACK_PACKAGE_VERSION_MAJOR "${SquadMaprota_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${SquadMaprota_VERSION_MINOR}")
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_FILE_NAME "SquadMaprotaGenerator")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Timbow Fletschoa Kayms")
include(CPack)

# uninstall target
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.txt"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
