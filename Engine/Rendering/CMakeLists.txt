set(VULKAN_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Vulkan/VulkanMemory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Vulkan/VulkanPipeline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Vulkan/VulkanPrivate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Vulkan/VulkanRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Vulkan/VulkanTexture.cpp
    PARENT_SCOPE
)

set(OGL_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/GLPipeline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/GLRenderer.cpp
    PARENT_SCOPE
)

set(WEBGPU_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/WebGPU/WebGPURenderer.cpp
    PARENT_SCOPE
)
