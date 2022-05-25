/* date = May 22nd 2022 1:25 pm */

#ifndef CONTEXT_H
#define CONTEXT_H

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include "defines.h"
#include "base/ds.h"
#include "base/utils.h"
#include "window.h"

Array_Prototype(StringArray, const char*);
Array_Prototype(VkImageArray, VkImage);
Set_Prototype(U32Set, u32);
Array_Prototype(VkDeviceQueueCreateInfoArray, VkDeviceQueueCreateInfo);

typedef struct V_VulkanContext {
    VkInstance instance;
    StringArray extensions;
    StringArray layers;
    
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    
    VkDebugUtilsMessengerEXT debug_messenger;
    
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkImageView* swapchain_image_views;
    VkImage* swapchain_images;
    u32 swapchain_image_count;
    
    VkFormat swapchain_image_format;
    VkExtent2D swapchain_extent;
} V_VulkanContext;


void Vulkan_Init(W_Window* window, V_VulkanContext* context, b8 debug_mode);
void Vulkan_Free(V_VulkanContext* context, b8 debug_mode);


typedef struct V_QueueFamilyIndices {
    u32_optional graphics_family;
    u32_optional present_family;
} V_QueueFamilyIndices;

b8 V_QueueFamilyIndicesValid(V_QueueFamilyIndices indices);
V_QueueFamilyIndices V_FindQueueFamilies(V_VulkanContext* context, VkPhysicalDevice device);


#endif //CONTEXT_H
