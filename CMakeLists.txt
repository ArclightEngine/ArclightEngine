cmake_minimum_required(VERSION 3.13)

project(ArclightEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_ENABLE_EXPORTS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options (-fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options (-fcolor-diagnostics)
endif ()

if(NOT MSVC)
    add_compile_options(-Wall -Wextra -fno-exceptions -fno-rtti)
else ()
    
endif()

option(USE_VULKAN "Use Vulkan renderer")
option(USE_DUMMY_RENDERER "Use dummy renderer")
option(USE_OPENGL "Use OpenGL ES renderer")
option(USE_WEBGPU "Use WebGPU renderer")

option(BUILD_EXAMPLES "Build example games" OFF)

if(CMAKE_SYSTEM_NAME MATCHES Emscripten)
    set(IS_EMSCRIPTEN ON)
    set(USE_OPENGL CACHE BOOL ON)
    set(IS_UNIX ON)
elseif(UNIX)
    set(IS_UNIX ON)
    if(CMAKE_SYSTEM_NAME MATCHES Darwin)
        set(IS_MACOS ON)
        set(USE_OPENGL CACHE BOOL ON)
    elseif(CMAKE_SYSTEM_NAME MATCHES Linux)
        set(IS_LINUX ON)
        set(USE_VULKAN CACHE BOOL ON)
    else() # Currently we do not support FreeBSD, etc.
        message(FATAL_ERROR "Unknown UNIX system ${CMAKE_SYSTEM_NAME}")
    endif()
elseif(CMAKE_SYSTEM_NAME MATCHES Windows)
    set(IS_WINDOWS ON)
    set(USE_VULKAN CACHE BOOL ON)
endif()

set(ENGINE_LIBS "")

include_directories(thirdparty)
include_directories(Engine/include)

add_subdirectory(thirdparty/fmt)

# When using emscripten, the library is linked into the game
if(NOT IS_EMSCRIPTEN)
    # Unless on emscripten use the in-tree freetype, ICU
    add_subdirectory(thirdparty/freetype)
    set(ENGINE_LIBS "${ENGINE_LIBS};freetype")

    add_executable(arclight Main/Main.cpp)
    target_link_libraries(arclight -Wl,--whole-archive libarclight -Wl,--no-whole-archive)

    if(IS_WINDOWS)
        include_directories(${CMAKE_SOURCE_DIR}/thirdparty/icu/include)
        set(ENGINE_LIBS "${ENGINE_LIBS};${CMAKE_SOURCE_DIR}/thirdparty/icu/lib64/icuuc.lib")
    else()
        find_package(ICU COMPONENTS uc REQUIRED)
        if(ICU_FOUND)
            include_directories(${ICU_INCLUDE_DIRS})
            set(ENGINE_LIBS "${ENGINE_LIBS};${ICU_LIBRARIES}")
        else()
            message(FATAL_ERROR "Failed to find ICU!")
        endif()
    endif()


    find_package(SDL2 REQUIRED)
    if(SDL2_FOUND)
        include_directories(${SDL2_INCLUDE_DIRS})
        set(ENGINE_LIBS "${ENGINE_LIBS};${SDL2_LIBRARIES}")

        add_definitions(-DARCLIGHT_SDL2=1)
    else()
        message(FATAL_ERROR "Failed to find SDL2!")
    endif()

    if(UNIX)
        target_link_libraries(arclight dl)
        target_link_libraries(arclight pthread)
    endif()
else()
    add_compile_options(-sUSE_SDL=2 -sUSE_ICU=1 -sUSE_FREETYPE=1 -DARCLIGHT_SINGLE_EXECUTABLE=1)

    add_link_options(-sMAX_WEBGL_VERSION=2)
    add_library(arclight STATIC Main/Main.cpp)

    find_package(fmt)
    target_link_libraries(arclight fmt::fmt-header-only)
    target_link_libraries(arclight libarclight)
endif()

add_subdirectory(Engine)

if(BUILD_EXAMPLES)
    set(ARCLIGHT_ENGINE_PATH ${CMAKE_SOURCE_DIR})
    add_subdirectory(Examples/Lightris)
endif()

if(IS_WINDOWS)
    add_custom_command(TARGET arclight POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/thirdparty/SDL2/lib/x64/SDL2.dll
            ${CMAKE_SOURCE_DIR}/thirdparty/icu/bin64/icuuc69.dll
            ${CMAKE_SOURCE_DIR}/thirdparty/icu/bin64/icudt69.dll
            $<TARGET_FILE_DIR:arclight>)
endif()

install(TARGETS arclight libarclight)
