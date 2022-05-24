#include "context.h"

#include <GLFW/glfw3.h>

Array_Impl(StringArray, const char*);
Array_Impl(VkImageArray, VkImage);

static b8 u32_eq(u32 a, u32 b) { return a == b; }
Set_Impl(U32Set, u32, u32_eq);
Array_Impl(VkDeviceQueueCreateInfoArray, VkDeviceQueueCreateInfo);

#include "vulkan_ext.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
               VkDebugUtilsMessageTypeFlagsEXT messageType,
               const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
               void* pUserData) {
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT && messageType != VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
        fprintf(stderr, "Validation: %s\n", pCallbackData->pMessage);
    return VK_FALSE;
}

//~ Helpers

static StringArray V_GetGLFWRequiredExtensions() {
    StringArray sa = {0};
    u32 intermediate_len;
    const char** intermediate = glfwGetRequiredInstanceExtensions(&intermediate_len);
    for (u32 i = 0; i < intermediate_len; i++) {
        StringArray_add(&sa, intermediate[i]);
    }
    return sa;
}

static StringArray V_GetDeviceRequiredExtensions() {
    StringArray sa = {0};
    StringArray_add(&sa, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    return sa;
}

static b8 V_ValidationLayersSupported(StringArray sa) {
    u32 layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    VkLayerProperties* props = calloc(layer_count, sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&layer_count, props);
    
    Iterate(sa, k) {
        b8 layer_is_available = false;
        for (u32 i = 0; i < layer_count; i++) {
            if (strcmp(sa.elems[k], props[i].layerName) == 0) {
                layer_is_available = true;
            }
        }
        
        if (!layer_is_available) {
            free(props);
            return false;
        }
    }
    free(props);
    return true;
}

static void V_FillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* dest) {
    dest->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    dest->messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    dest->messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    dest->pfnUserCallback = debug_callback;
}


//- Physical Device Stuff 

typedef struct V_QueueFamilyIndices {
    u32_optional graphics_family;
    u32_optional present_family;
} V_QueueFamilyIndices;

static b8 V_QueueFamilyIndicesValid(V_QueueFamilyIndices indices) {
    return indices.graphics_family.valid && indices.present_family.valid;
}

static V_QueueFamilyIndices V_FindQueueFamilies(V_VulkanContext* context, VkPhysicalDevice device) {
    V_QueueFamilyIndices indices;
    
    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
    VkQueueFamilyProperties* queue_families = calloc(queue_family_count, sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);
    
    for (u32 i = 0; i < queue_family_count; i++) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            Optional_Set(indices.graphics_family, i);
        
        b32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, context->surface, &present_support);
        if (present_support) Optional_Set(indices.present_family, i);
        
        if (V_QueueFamilyIndicesValid(indices)) break;
    }
    
    free(queue_families);
    return indices;
}

static b8 V_DeviceExtensionsSupported(VkPhysicalDevice device, StringArray sa) {
    u32 extension_count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
    VkExtensionProperties* supported_extensions = calloc(extension_count, sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, supported_extensions);
    
    Iterate(sa, k) {
        b8 found = false;
        
        for (u32 i = 0; i < extension_count; i++) {
            if (strcmp(sa.elems[k], supported_extensions[i].extensionName) == 0) {
                found = true;
            }
        }
        
        if (!found) {
            free(supported_extensions);
            return false;
        }
    }
    
    free(supported_extensions);
    return true;
}

typedef struct V_SwapchainDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    u32 format_count;
    VkPresentModeKHR* present_modes;
    u32 present_mode_count;
} V_SwapchainDetails;

static void V_FreeSwapchainDetails(V_SwapchainDetails details) {
    free(details.formats);
    free(details.present_modes);
}

static V_SwapchainDetails V_SwapchainSupportDetails(V_VulkanContext* context, VkPhysicalDevice device) {
    V_SwapchainDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, context->surface, &details.capabilities);
    
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, context->surface, &details.format_count, nullptr);
    if (details.format_count) {
        details.formats = calloc(details.format_count, sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, context->surface, &details.format_count, details.formats);
    }
    
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, context->surface, &details.present_mode_count, nullptr);
    if (details.present_mode_count) {
        details.formats = calloc(details.present_mode_count, sizeof(VkPresentModeKHR));
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, context->surface, &details.present_mode_count, details.present_modes);
    }
    
    return details;
}

static b8 V_SwapchainIsAdequate(V_SwapchainDetails details) {
    return details.format_count != 0 && details.present_mode_count != 0;
}

static u32 V_ScoreDevice(V_VulkanContext* context, VkPhysicalDevice device) {
    V_QueueFamilyIndices queue_families = V_FindQueueFamilies(context, device);
    if (!V_QueueFamilyIndicesValid(queue_families)) return 0;
    
    StringArray required_device_extensions = V_GetDeviceRequiredExtensions();
    if (!V_DeviceExtensionsSupported(device, required_device_extensions)) return 0;
    
    V_SwapchainDetails swapchain_details = V_SwapchainSupportDetails(context, device);
    if (!V_SwapchainIsAdequate(swapchain_details)) return 0;
    
    VkPhysicalDeviceProperties device_props;
    vkGetPhysicalDeviceProperties(device, &device_props);
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(device, &device_features);
    
    u32 score = 0;
    switch (device_props.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: score += 1000;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: score += 1000;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: score += 1000;
        case VK_PHYSICAL_DEVICE_TYPE_CPU: score += 1000;
        default:;
    }
    
    V_FreeSwapchainDetails(swapchain_details);
    StringArray_free(&required_device_extensions);
    return score;
}

static VkSurfaceFormatKHR V_ChooseSwapSurfaceFormat(V_SwapchainDetails* detail) {
    for (u32 i = 0; i < detail->format_count; i++) {
        if (detail->formats[i].format == VK_FORMAT_B8G8R8A8_SRGB
            && detail->formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return detail->formats[i];
        }
    }
    return detail->formats[0];
}

static VkPresentModeKHR V_ChoosePresentMode(V_SwapchainDetails* detail) {
    for (u32 i = 0; i < detail->present_mode_count; i++) {
        if (detail->present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return detail->present_modes[i];
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D V_ChooseSwapExtent(W_Window* window, V_SwapchainDetails* detail) {
    if (detail->capabilities.currentExtent.width != 0) {
        return detail->capabilities.currentExtent;
    } else {
        i32 width, height;
        glfwGetFramebufferSize(window->handle, &width, &height);
        
        VkExtent2D extent = { (u32) width, (u32) height };
        extent.width  = Clamp(detail->capabilities.minImageExtent.width,  width, detail->capabilities.maxImageExtent.width);
        extent.height = Clamp(detail->capabilities.minImageExtent.height, height, detail->capabilities.maxImageExtent.height);
        return extent;
    }
}

//~ Main Initialization steps

static b8 V_CreateInstance(V_VulkanContext* context, b8 debug_mode) {
    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Vulkan App";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "VisualX";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.engineVersion = VK_VERSION_1_3;
    
    context->extensions = V_GetGLFWRequiredExtensions();
    if (debug_mode) {
        StringArray_add(&context->extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        StringArray_add(&context->layers, "VK_LAYER_KHRONOS_validation");
        V_ValidationLayersSupported(context->layers);
    }
    
    VkInstanceCreateInfo instance_create_info = {0};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledLayerCount = context->layers.len;
    instance_create_info.ppEnabledLayerNames = context->layers.elems;
    instance_create_info.enabledExtensionCount = context->extensions.len;
    instance_create_info.ppEnabledExtensionNames = context->extensions.elems;
    
    VkDebugUtilsMessengerCreateInfoEXT temp_debug_messenger = {0};
    if (debug_mode) {
        V_FillDebugMessengerCreateInfo(&temp_debug_messenger);
        instance_create_info.pNext = &temp_debug_messenger;
    }
    
    VkResult res = vkCreateInstance(&instance_create_info, nullptr, &context->instance);
    AssertFalse(res, "vkCreateInstance Failed with code %d\n", res);
    
    return true;
}

static b8 V_CreateDebugMessenger(V_VulkanContext* context, b8 debug_mode) {
    if (!debug_mode) return true;
    
    VkDebugUtilsMessengerCreateInfoEXT messenger_create_info = {0};
    V_FillDebugMessengerCreateInfo(&messenger_create_info);
    
    VkResult res = vkCreateDebugUtilsMessengerEXT(context->instance, &messenger_create_info, nullptr, &context->debug_messenger);
    AssertFalse(res, "vkCreateDebugUtilsMessengerEXT Failed with code %d\n", res);
    
    return true;
}

static b8 V_PickPhysicalDevice(V_VulkanContext* context, b8 debug_mode) {
    u32 physical_device_count;
    vkEnumeratePhysicalDevices(context->instance, &physical_device_count, nullptr);
    VkPhysicalDevice* devices = calloc(physical_device_count, sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(context->instance, &physical_device_count, devices);
    
    u32 max = 0;
    i32 max_idx = -1;
    for (u32 i = 0; i < physical_device_count; i++) {
        u32 score = V_ScoreDevice(context, devices[i]);
        if (score > max) {
            max = score;
            max_idx = i;
        }
    }
    if (max_idx == -1) {
        free(devices);
        return false;
    }
    
    context->physical_device = devices[max_idx];
    free(devices);
    
    return true;
}

static b8 V_CreateLogicalDevice(V_VulkanContext* context, b8 debug_mode) {
    V_QueueFamilyIndices indices = V_FindQueueFamilies(context, context->physical_device);
    
    U32Set unique_queue_set = {0};
    U32Set_add(&unique_queue_set, indices.graphics_family.value);
    U32Set_add(&unique_queue_set, indices.present_family.value);
    
    f32 queue_priority = 1.f;
    VkDeviceQueueCreateInfoArray queue_create_infos;
    for (u32 i = 0; i < unique_queue_set.len; i++) {
        VkDeviceQueueCreateInfo queue_create_info = {0};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = unique_queue_set.elems[i];
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_info.queueCount = 1;
        VkDeviceQueueCreateInfoArray_add(&queue_create_infos, queue_create_info);
    }
    
    StringArray required_device_extensions = V_GetDeviceRequiredExtensions();
    
    VkPhysicalDeviceFeatures physical_device_features = {0};
    VkDeviceCreateInfo device_create_info = {0};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = queue_create_infos.elems;
    device_create_info.queueCreateInfoCount = queue_create_infos.len;
    device_create_info.pEnabledFeatures = &physical_device_features;
    device_create_info.enabledExtensionCount = 0;
    device_create_info.enabledLayerCount = 0;
    
    VkResult res = vkCreateDevice(context->physical_device, &device_create_info, nullptr, &context->device);
    AssertFalse(res, "vkCreateDevice Failed with code %d\n", res);
    vkGetDeviceQueue(context->device, indices.graphics_family.value, 0, &context->graphics_queue);
    vkGetDeviceQueue(context->device, indices.present_family.value, 0, &context->present_queue);
    
    StringArray_free(&required_device_extensions);
    
    return true;
}

static b8 V_CreateSurface(W_Window* window, V_VulkanContext* context, b8 debug_mode) {
    VkResult res = glfwCreateWindowSurface(context->instance, window->handle, nullptr, &context->surface);
    AssertFalse(res, "vkCreateWin32SurfaceKHR Failed with code %d\n", res);
    return true;
}

static b8 V_CreateSwapchain(W_Window* window, V_VulkanContext* context, b8 debug_mode) {
    V_SwapchainDetails details = V_SwapchainSupportDetails(context, context->physical_device);
    
    VkSurfaceFormatKHR surface_format = V_ChooseSwapSurfaceFormat(&details);
    VkPresentModeKHR present_mode = V_ChoosePresentMode(&details);
    VkExtent2D extent = V_ChooseSwapExtent(window, &details);
    
    u32 image_count = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount) {
        image_count = details.capabilities.maxImageCount;
    }
    
    VkSwapchainCreateInfoKHR swapchain_create_info = {0};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = context->surface;
    
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = surface_format.format;
    swapchain_create_info.imageColorSpace = surface_format.colorSpace;
    swapchain_create_info.imageExtent = extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    V_QueueFamilyIndices indices = V_FindQueueFamilies(context, context->physical_device);
    u32 indices_packed[] = { indices.graphics_family.value, indices.present_family.value };
    
    if (indices.graphics_family.value != indices.present_family.value) {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices = indices_packed;
    } else {
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    
    swapchain_create_info.preTransform = details.capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;
    
    VkResult res = vkCreateSwapchainKHR(context->device, &swapchain_create_info, nullptr, &context->swapchain);
    AssertFalse(res, "vkCreateSwapchainKHR Failed with code %d\n", res);
    
    vkGetSwapchainImagesKHR(context->device, context->swapchain, &image_count, nullptr);
    context->swapchain_images = calloc(image_count, sizeof(VkImage));
    vkGetSwapchainImagesKHR(context->device, context->swapchain, &image_count, context->swapchain_images);
    context->swapchain_image_count = image_count;
    
    context->swapchain_image_format = surface_format.format;
    context->swapchain_extent = extent;
    
    return true;
}

void Vulkan_Init(W_Window* window, V_VulkanContext* context, b8 debug_mode) {
    Assert(V_CreateInstance(context, debug_mode), "Instance Creation Failed\n");
    Assert(V_CreateDebugMessenger(context, debug_mode), "Debug Messenger Creation Failed\n");
    Assert(V_CreateSurface(window, context, debug_mode), "Debug Messenger Creation Failed\n");
    Assert(V_PickPhysicalDevice(context, debug_mode), "Physical Device Picking Failed\n");
    Assert(V_CreateLogicalDevice(context, debug_mode), "Logical Device Creation Failed\n");
    Assert(V_CreateSwapchain(window, context, debug_mode), "Logical Device Creation Failed\n");
}

void Vulkan_Free(V_VulkanContext* context, b8 debug_mode) {
    if (context->swapchain_images)
        free(context->swapchain_images);
    vkDestroySwapchainKHR(context->device, context->swapchain, nullptr);
    vkDestroyDevice(context->device, nullptr);
    if (debug_mode)
        vkDestroyDebugUtilsMessengerEXT(context->instance, context->debug_messenger, nullptr);
    vkDestroySurfaceKHR(context->instance, context->surface, nullptr);
    vkDestroyInstance(context->instance, nullptr);
    StringArray_free(&context->extensions);
    StringArray_free(&context->layers);
}
