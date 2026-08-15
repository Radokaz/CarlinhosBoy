cmake_minimum_required(VERSION 4.2.0)

# Project name
project(Carlinhos_Boy LANGUAGES C CXX)

# Set C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
    set(BUILDING_UWP TRUE)
else()
    set(BUILDING_UWP FALSE)
endif()

# Source folder to find the .c/.cpp files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")

if(BUILDING_UWP)
  list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/tinyfiledialogs.c")
endif()

file(COPY ${CMAKE_SOURCE_DIR}/resources/dmg_boot.bin DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/resources/cgb_boot.bin DESTINATION ${CMAKE_BINARY_DIR})

if(WIN32)
  set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_SOURCE_DIR}/resources/Cavalo.rc")
  add_executable(cb ${SOURCES} ${APP_ICON_RESOURCE_WINDOWS})
else()
  add_executable(cb ${SOURCES})
endif()

if(MSVC)
  target_compile_definitions(cb PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Esse SDL é para a versão uwp apenas
if(BUILDING_UWP)
  set(SDL3_UWP_ROOT "${CMAKE_SOURCE_DIR}/include/sdl3-uwp")
  set(SDL3_UWP_BIN "${CMAKE_SOURCE_DIR}/include/sdl3-uwp/VisualC-WinRT/x64/Release/SDL-UWP")

  add_library(SDL3::SDL3 SHARED IMPORTED GLOBAL)
  set_target_properties(SDL3::SDL3 PROPERTIES
    IMPORTED_IMPLIB   "${SDL3_UWP_BIN}/SDL3.lib"
    IMPORTED_LOCATION "${SDL3_UWP_BIN}/SDL3.dll"
    INTERFACE_INCLUDE_DIRECTORIES "${SDL3_UWP_ROOT}/include"
  )

  set(PLATFORM "SDL" CACHE STRING "" FORCE)
  set(MESA_BIN_DIR "${CMAKE_SOURCE_DIR}/include/mesa-uwp/bin")
  target_sources(cb PRIVATE uwp/Package.appxmanifest) 
  target_sources(cb PRIVATE
    "${MESA_BIN_DIR}/opengl32.dll"
    "${MESA_BIN_DIR}/libgallium_wgl.dll"
    "${MESA_BIN_DIR}/libEGL.dll"
    "${MESA_BIN_DIR}/libGLESv1_CM.dll"
    "${MESA_BIN_DIR}/libGLESv2.dll"
    "${MESA_BIN_DIR}/z-1.dll"
    "${MESA_BIN_DIR}/spirv_to_dxil.dll"
    "${MESA_BIN_DIR}/vulkan_dzn.dll"
    "${MESA_BIN_DIR}/glsl_compiler.exe"
    "${MESA_BIN_DIR}/spirv2dxil.exe"
    "${SDL3_UWP_BIN}/SDL3.dll"
    "${CMAKE_SOURCE_DIR}/resources/dmg_boot.bin"
    "${CMAKE_SOURCE_DIR}/resources/cgb_boot.bin"
  )
  set_source_files_properties(
    "${MESA_BIN_DIR}/opengl32.dll"
    "${MESA_BIN_DIR}/libgallium_wgl.dll"
    "${MESA_BIN_DIR}/libEGL.dll"
    "${MESA_BIN_DIR}/libGLESv1_CM.dll"
    "${MESA_BIN_DIR}/libGLESv2.dll"
    "${MESA_BIN_DIR}/z-1.dll"
    "${MESA_BIN_DIR}/spirv_to_dxil.dll"
    "${MESA_BIN_DIR}/vulkan_dzn.dll"
    "${MESA_BIN_DIR}/glsl_compiler.exe"
    "${MESA_BIN_DIR}/spirv2dxil.exe"
    "${SDL3_UWP_BIN}/SDL3.dll"
    "${CMAKE_SOURCE_DIR}/resources/dmg_boot.bin"
    "${CMAKE_SOURCE_DIR}/resources/cgb_boot.bin"
    PROPERTIES VS_DEPLOYMENT_CONTENT 1
  )
  target_compile_definitions(cb PRIVATE UWP_BUILDING)
  target_include_directories(cb PRIVATE "${SDL3_UWP_ROOT}/include;${SDL3_UWP_ROOT}/include/build_config")
endif()
 
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

if(NOT BUILDING_UWP)
  # versão para debug
  target_compile_options(cb PRIVATE -Werror -Wall -Wpedantic -g)
  target_link_options(cb PRIVATE -fsanitize=address)

  # essa é pra versão final
  # target_compile_options(cb PRIVATE -O2 -march=x86-64-v3 -flto -fno-exceptions)
endif()

target_include_directories(cb PRIVATE include)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_link_options(cb PRIVATE -static -static-libgcc -static-libstdc++)
endif()

add_subdirectory(include/raylib)

if(BUILDING_UWP)
  target_include_directories(raylib BEFORE PRIVATE "${SDL3_UWP_ROOT}/include;${SDL3_UWP_ROOT}/include/build_config")
  target_link_libraries(raylib PRIVATE SDL3::SDL3)
  target_link_libraries(cb PRIVATE SDL3::SDL3)
endif()

target_link_libraries(cb PRIVATE raylib)

if(MSVC)
  target_compile_options(raylib PRIVATE /showIncludes)
  target_compile_definitions(raylib PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
