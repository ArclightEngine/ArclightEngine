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

if(IS_WINDOWS)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

add_library(libarclight SHARED)
target_compile_definitions(libarclight PRIVATE ARCLIGHT_API_SHOULD_EXPORT=1)

find_package(fmt)
target_link_libraries(libarclight fmt::fmt-header-only)

if(NOT IS_EMSCRIPTEN)
    set(ENGINE_SRC
        ${ENGINE_SRC}
        ${ICU_SOURCES}
    )
endif()

# Platform specific
if(IS_UNIX)
    set(ENGINE_SRC
        ${ENGINE_SRC}
        Platform/UNIX/Filesystem.cpp
        Platform/UNIX/Time.cpp
    )
elseif(IS_WINDOWS)
    set(ENGINE_SRC
        ${ENGINE_SRC}
        Platform/Windows/Filesystem.cpp
        Platform/Windows/Time.cpp
    )
else()
    message(FATAL_ERROR "Unsupported platform!")
endif()

target_sources(libarclight PRIVATE ${ENGINE_SRC})

target_link_libraries(libarclight ${ENGINE_LIBS})

target_include_directories(libarclight PUBLIC include)
target_include_directories(libarclight PRIVATE .)

# Process render sources afterwards
add_subdirectory(Rendering)

if(USE_VULKAN)
    find_package(Vulkan REQUIRED)

    target_include_directories(libarclight PUBLIC ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(libarclight ${Vulkan_LIBRARIES})

    target_sources(libarclight PRIVATE ${VULKAN_SRC})
    target_compile_definitions(libarclight PUBLIC ARCLIGHT_VULKAN=1)
endif()

if(USE_OPENGL)
    target_compile_definitions(libarclight PUBLIC ARCLIGHT_OPENGL=1)

    if(NOT IS_EMSCRIPTEN)
        find_library(OGL_ES_LIBRARY GLESv2)
        target_link_libraries(libarclight ${OGL_ES_LIBRARY})
    endif()

    target_sources(libarclight PRIVATE ${OGL_SRC})
endif()

if(USE_WEBGPU)
    add_definitions(-DARCLIGHT_WEBGPU=1)

    if(IS_EMSCRIPTEN)

    elseif(IS_LINUX)
        target_link_libraries(libarclight ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/dawn/bin_linux_x86_64)
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/dawn/include)
    elseif(IS_WINDOWS)
        target_link_libraries(libarclight ${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/dawn/bin_windows_x86_64)
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/dawn/include)
    else()
        
    endif()

    target_sources(libarclight PRIVATE ${WEBGPU_SRC})
endif()

if(USE_DUMMY_RENDERER)
    add_definitions(-DARCLIGHT_DUMMY_RENDERER=1)
endif()
