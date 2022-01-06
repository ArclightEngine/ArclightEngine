set(ENGINE_SRC
    "src/Core/Application.cpp"
    "src/Core/File.cpp"
    "src/Core/Input.cpp"
    "src/Core/Resource.cpp"
    "src/Core/ResourceManager.cpp"
    "src/Core/ThreadPool.cpp"
    "src/ECS/World.cpp"
    "src/Graphics/Font.cpp"
    "src/Graphics/Image.cpp"
    "src/Graphics/Matrix.cpp"
    "src/Graphics/Text.cpp"
    "src/Graphics/Texture.cpp"
    "src/Graphics/Transform.cpp"
    "src/Graphics/VertexBuffer.cpp"
    "src/Graphics/Rendering/Pipeline.cpp"
    "src/Graphics/Rendering/RendererBackend.cpp"
    "src/Graphics/Rendering/Shader.cpp"
    "src/Platform/Platform.cpp"
    "src/State/StateManager.cpp"
    "src/Systems/Renderer2D.cpp"
    "src/Window/WindowContext.cpp"
)

include_directories(include)
include_directories(.)

# Platform specific
if(IS_UNIX)
    set(ENGINE_SRC
        ${ENGINE_SRC}
        Platform/UNIX/Filesystem.cpp
    )
elseif(IS_WINDOWS)
    target_source(ENGINE_SRC
        ${ENGINE_SRC}
        Platform/Windows/Filesystem.cpp
    )
else()
    message(FATAL_ERROR "Unsupported platform!")
endif()

add_library(libarclight OBJECT ${ENGINE_SRC})

find_package(fmt)
target_link_libraries(libarclight fmt::fmt)

# Process render sources afterwards
add_subdirectory(Rendering)

if(USE_VULKAN)
    add_definitions(-DARCLIGHT_VULKAN=1)

    find_package(Vulkan REQUIRED)

    include_directories(${Vulkan_INCLUDE_DIRS})
    target_link_libraries(libarclight ${Vulkan_LIBRARIES})

    target_sources(libarclight PRIVATE ${VULKAN_SRC})
endif()

if(USE_OPENGL)
    add_definitions(-DARCLIGHT_OPENGL=1)

    if(NOT IS_EMSCRIPTEN)
        find_library(OGL_ES_LIBRARY GLESv2)
        target_link_libraries(libarclight ${OGL_ES_LIBRARY})
    endif()

    target_sources(libarclight PRIVATE ${OGL_SRC})
endif()

if(USE_DUMMY_RENDERER)
    add_definitions(-DARCLIGHT_DUMMY_RENDERER=1)
endif()
