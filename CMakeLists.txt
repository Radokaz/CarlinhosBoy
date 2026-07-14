cmake_minimum_required(VERSION 4.2.0)

# Project name
project(Carlinhos_Boy LANGUAGES C CXX)

# Set C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source folder to find the .c/.cpp files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")

file(COPY ${CMAKE_SOURCE_DIR}/resources/dmg_boot.bin DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/resources/cgb_boot.bin DESTINATION ${CMAKE_BINARY_DIR})

if(WIN32)
  set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_SOURCE_DIR}/resources/Cavalo.rc")
  add_executable(cb ${SOURCES} ${APP_ICON_RESOURCE_WINDOWS})
else()
  add_executable(cb ${SOURCES})
endif()

# Create executable
target_compile_options(cb PRIVATE -Werror -g)
target_include_directories(cb PRIVATE include)
#target_link_options(gb PRIVATE -fsanitize=address)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_link_options(cb PRIVATE -static -static-libgcc -static-libstdc++)
endif()

add_subdirectory(include/raylib)
target_link_libraries(cb PRIVATE raylib)
