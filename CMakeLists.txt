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

add_compile_options(-Wall -Wextra -fno-exceptions -fno-rtti)

option(USE_VULKAN "Use Vulkan renderer")
option(USE_DUMMY_RENDERER "Use dummy renderer")
option(USE_OPENGL "Use OpenGL ES renderer")

if(CMAKE_SYSTEM_NAME MATCHES Emscripten)
    set(IS_EMSCRIPTEN ON)
    set(USE_OPENGL CACHE BOOL ON)
    set(IS_UNIX ON)
elseif(UNIX)
    set(IS_UNIX ON)
    if(CMAKE_SYSTEM_NAME MATCHES Darwin)
        set(IS_MACOS ON)
        set(USE_VULKAN CACHE BOOL ON)
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

# When using emscripten, the library is linked into the game
if(NOT IS_EMSCRIPTEN)
    add_executable(arclight Main/Main.cpp)
    target_link_libraries(arclight -Wl,--whole-archive libarclight -Wl,--no-whole-archive)

    if(UNIX)
        target_link_libraries(arclight dl)
        target_link_libraries(arclight pthread)
    endif()

    find_package(SDL2 REQUIRED)
    if(SDL2_FOUND)
        target_link_libraries(arclight SDL2)

        add_definitions(-DARCLIGHT_SDL2=1)
    else()
        message(FATAL_ERROR "Failed to find SDL2!")
    endif()

    find_package(Freetype REQUIRED)
    if(Freetype_FOUND)
        target_link_libraries(arclight ${FREETYPE_LIBRARIES})
        include_directories(${FREETYPE_INCLUDE_DIRS})
        include_directories(${FREETYPE_INCLUDE_DIR_ft2build})
    else()
        message(FATAL_ERROR "Failed to find freetype!")
    endif()

    find_package(ICU 69 COMPONENTS uc i81n REQUIRED)
    if(ICU_FOUND)
        include_directories(${ICU_INCLUDE_DIRS})
        target_link_libraries(arclight ${ICU_LIBRARIES})
    else()
        message(FATAL_ERROR "Failed to find ICU!")
    endif()
else()
    add_compile_options(-sUSE_SDL=2 -sUSE_ICU=1 -sUSE_FREETYPE=1)
    add_link_options(-sMAX_WEBGL_VERSION=2)
    add_library(arclight STATIC Main/Main.cpp)
    target_link_libraries(arclight libarclight)
endif()

include_directories(thirdparty)
include_directories(Engine/include)

add_subdirectory(thirdparty/fmt)
add_subdirectory(Engine)
