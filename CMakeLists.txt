cmake_minimum_required(VERSION 4.2.0)

# Project name
project(Gameboy_brabo LANGUAGES C CXX)

# Set C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source folder to find the .c/.cpp files
file(GLOB_RECURSE SOURCES src/*.cpp)

# Create executable
add_executable(gb ${SOURCES})
target_compile_options(gb PRIVATE -g)

target_include_directories(gb PRIVATE include)
add_subdirectory(include/raylib)
target_link_libraries(gb PRIVATE raylib)
# For multi-file projects with headers and separate .cpp files
# or
# add_executable(rooster main.cpp dog.cpp dog.h cat.cpp cat.h)
