cmake_minimum_required(VERSION 3.5)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(ne)

# Convert the value inside CMAKE_SYSTEM_NAME to an all uppercase
# name where all symbols are replaced with underscores.
# "GNU/kFreeBSD" -> "GNU_KFREEBSD", "Windows" -> "WINDOWS", "Emscripten" -> "EMSCRIPTEN"...
# See https://github.com/Kitware/CMake/blob/master/Modules/CMakeDetermineSystem.cmake
string(REGEX REPLACE "[^0-9a-zA-Z]" "_" NE_CORE_PLATFORM ${CMAKE_SYSTEM_NAME})
string(TOUPPER ${NE_CORE_PLATFORM} NE_CORE_PLATFORM)

# Convert the value inside CMAKE_SYSTEM_NAME to a quoted string.
# We guarantee that all names will be at least 8 bytes long by
# appending 7 nulls (+1 for the implicit null in C strings).
# We also guarantee the first 8 bytes will be unique.
# This is currently true for all possible CMAKE_SYSTEM_NAMEs.
# See https://github.com/Kitware/CMake/blob/master/Modules/CMakeDetermineSystem.cmake
string(CONCAT NE_CORE_PLATFORM_NAME "\\" "\"" ${CMAKE_SYSTEM_NAME} "\\0\\0\\0\\0\\0\\0\\0\\" "\"")

add_definitions(-D NE_CORE_PLATFORM_NAME=${NE_CORE_PLATFORM_NAME})
add_definitions(-D NE_CORE_PLATFORM_${NE_CORE_PLATFORM}=1)

# In the future we'll move this to be individual CMake files for each
# package (test will bring them all together) and then this CMake file
# will just exist for convenience of building.
add_executable(
  ne
  packages/ne_core/ne_core.h
  packages/ne_core/ne_core_private.h
  packages/ne_core/ne_core.cpp
  packages/ne_core/ne_core_platform.hpp
  packages/ne_core/ne_core_platform.cpp
  packages/ne_io/ne_io.h
  packages/ne_io/ne_io.cpp
  packages/ne_time/ne_time.h
  packages/ne_time/ne_time.cpp
  packages/ne_filesystem/ne_filesystem.h
  packages/ne_filesystem/ne_filesystem.cpp
  packages/test/test.h
  packages/test/test.cpp
  packages/test_core/test_core.h
  packages/test_core/test_core.cpp
  packages/test_io/test_io.h
  packages/test_io/test_io.cpp
  packages/test_time/test_time.h
  packages/test_time/test_time.cpp
  packages/test_filesystem/test_filesystem.h
  packages/test_filesystem/test_filesystem.cpp
)

# Enable extra warnings.
if(MSVC)
  target_compile_options(ne PRIVATE /W4)
else()
  target_compile_options(ne PRIVATE -Wall -Wextra -pedantic)
endif()

#target_include_directories(ne directory...)
#target_link_libraries(ne directory...)
