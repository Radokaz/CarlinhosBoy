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

if(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
    set(BUILDING_UWP TRUE)
else()
    set(BUILDING_UWP FALSE)
endif()

# Esse SDL é para a versão uwp apenas
if(BUILDING_UWP)
  set(SDL3_DIR "${CMAKE_SOURCE_DIR}/include/sdl3-uwp" CACHE PATH "" FORCE)
  set(SDL3_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include/sdl3-uwp/include" CACHE PATH "" FORCE)
  set(SDL3_LIBRARIES "${CMAKE_SOURCE_DIR}/include/sdl3-uwp/VisualC-WinRT/x64/Release/SDL3.lib" CACHE FILEPATH "" FORCE)
  set(PLATFORM "SDL" CACHE STRING "" FORCE)
  target_compile_definitions(cb PRIVATE UWP_BUILDING)
endif()
 
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

# versão para debug
target_compile_options(cb PRIVATE -Werror -Wall -Wpedantic -g)
target_link_options(cb PRIVATE -fsanitize=address)

# essa é pra versão final
# target_compile_options(cb PRIVATE -O2 -march=x86-64-v3 -flto -fno-exceptions)
target_include_directories(cb PRIVATE include)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_link_options(cb PRIVATE -static -static-libgcc -static-libstdc++)
endif()

add_subdirectory(include/raylib)
target_link_libraries(cb PRIVATE raylib)

# Link com o mesa para o uwp
if(BUILDING_UWP)
  set(MESA_BIN_DIR "${CMAKE_SOURCE_DIR}/include/mesa-uwp/bin")
  add_custom_command(TARGET cb POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
       "${MESA_BIN_DIR}/opengl32.dll"
       "${MESA_BIN_DIR}/libgallium_wgl.dll"
       $<TARGET_FILE_DIR:cb>)
endif()

