cmake_minimum_required(VERSION 4.2.0)

# Project name
project(Carlinhos_Boy LANGUAGES C CXX)

# Set C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source folder to find the .c/.cpp files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")

file(COPY ${CMAKE_SOURCE_DIR}/dmg_boot.bin DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/cgb_boot.bin DESTINATION ${CMAKE_BINARY_DIR})
# Create executable
add_executable(gb ${SOURCES})
target_compile_options(gb PRIVATE -Werror -g)
target_include_directories(gb PRIVATE include)
#target_link_options(gb PRIVATE -fsanitize=address)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_link_options(gb PRIVATE -static -static-libgcc -static-libstdc++)
endif()

add_subdirectory(include/raylib)
target_link_libraries(gb PRIVATE raylib)

# For multi-file projects with headers and separate .cpp files
# or
# add_executable(rooster main.cpp dog.cpp dog.h cat.cpp cat.h)
