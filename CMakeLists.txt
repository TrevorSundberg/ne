cmake_minimum_required(VERSION 2.8)
project(ne)

# Convert the value inside CMAKE_SYSTEM_NAME to an all uppercase
# name where all symbols are replaced with underscores.
# "GNU/kFreeBSD" -> "GNU_KFREEBSD", "Windows" -> "WINDOWS", "Emscripten" -> "EMSCRIPTEN"...
# See https://github.com/Kitware/CMake/blob/master/Modules/CMakeDetermineSystem.cmake
string(REGEX REPLACE "[^0-9a-zA-Z]" "_" NE_PLATFORM ${CMAKE_SYSTEM_NAME})
string(TOUPPER ${NE_PLATFORM} NE_PLATFORM)
string(CONCAT NE_PLATFORM_NAME "\\" "\"" ${NE_PLATFORM} "\\" "\"")

add_definitions(-D NE_PLATFORM_NAME=${NE_PLATFORM_NAME})
add_definitions(-D NE_PLATFORM_${NE_PLATFORM}=1)

# In the future we'll move this to be individual CMake files for each
# package (test will bring them all together) and then this CMake file
# will just exist for convenience of building.
add_executable(
  ne
  packages/ne_core/ne_core.h
  packages/ne_core/ne_core.cpp
  packages/ne_core/ne_intrinsic.h
  packages/ne_core/ne_intrinsic.cpp
  packages/ne_io/ne_io.h
  packages/ne_io/ne_io.cpp
  packages/test_core/test_core.h
  packages/test_core/test_core.cpp
  packages/test_io/test_io.h
  packages/test_io/test_io.cpp
  packages/test/test.h
  packages/test/test.cpp)

#target_include_directories(ne directory...)
#target_link_libraries(ne directory...)