cmake_minimum_required(VERSION 3.16)

project(SmallObjectAllocator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

add_executable(SmallObjectAllocator
  main.cpp
  SmallObjectAllocator.cpp
  SmallObjectAllocator.h
  Chunk.cpp
  Chunk.h
  FixedAllocator.cpp
  FixedAllocator.h
  SmallObject.h
)

if(MSVC)
  target_compile_options(SmallObjectAllocator PRIVATE /W4)
else()
  target_compile_options(SmallObjectAllocator PRIVATE -Wall -Wextra -Wpedantic)
endif()
