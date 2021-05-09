#pragma once

#include <vulkan/vulkan_core.h>
#include <stdexcept>

#define vkCheck(x) { VkResult r = (x); if(r != VK_SUCCESS) throw std::runtime_error(std::string("[Fatal error]") + __PRETTY_FUNCTION__ + " vkCheck failed!"); }