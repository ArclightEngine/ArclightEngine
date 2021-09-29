cmake_minimum_required(VERSION 3.13)

project(Game)

option(STANDALONE "Build game without external dependencies" OFF)
option(ARCLIGHT_ENGINE_PATH "Path to Arclight Engine directory" "%ARCLIGHTBUILD_ENGINE_PATH%")

# Don't use a prefix to build game.so
unset(CMAKE_SHARED_LIBRARY_PREFIX)

# Force PIC
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_SYSTEM_NAME MATCHES Emscripten)
    set(IS_EMSCRIPTEN ON)
    set(STANDALONE ON)
    set(IS_UNIX ON)
elseif(UNIX)
    set(IS_UNIX ON)
    if(CMAKE_SYSTEM_NAME MATCHES Darwin)
        set(IS_MACOS ON)
    elseif(CMAKE_SYSTEM_NAME MATCHES Linux)
        set(IS_LINUX ON)
    else() # Currently we do not support FreeBSD, etc.
        message(FATAL_ERROR "Unknown UNIX system ${CMAKE_SYSTEM_NAME}")
    endif()
elseif(CMAKE_SYSTEM_NAME MATCHES Windows)
    set(IS_WINDOWS ON)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(
    ${ARCLIGHT_ENGINE_PATH}/Engine/include
    ${ARCLIGHT_ENGINE_PATH}/thirdparty
)

# Add your game source files here
set(GAME_SOURCES
    src/Main.cpp
)

if(IS_STANDALONE)
    find_library(ENGINE_LIB ${ARCLIGHT_ENGINE_PATH})
endif()

if(IS_EMSCRIPTEN)
    add_compile_options(-sUSE_SDL=2 -sUSE_ICU=1 -sUSE_FREETYPE=1)
    add_executable(game.html ${GAME_SOURCES} ${ENGINE_LIB})
else()
    add_library(game SHARED ${GAME_SOURCES})
endif()
