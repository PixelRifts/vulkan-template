#include <GLFW/glfw3.h>

#include "defines.h"
#include "window.h"
#include "base/input.h"
#include "context.h"
#include "pipeline.h"
#include "base/vmath.h"

VkSemaphore image_available_semaphore;
VkSemaphore render_finished_semaphore;
VkFence in_flight_fence;

static void CreateSyncObjects(V_VulkanContext* context, V_VulkanPipeline* pipeline) {
    VkSemaphoreCreateInfo semaphore_create_info = {0};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fence_create_info = {0};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    VkResult res;
    res =
        vkCreateSemaphore(context->device,
                          &semaphore_create_info, nullptr, &image_available_semaphore);
    AssertFalse(res, "vkCreateSemaphore (1) Failed with code %d\n", res);
    res =
        vkCreateSemaphore(context->device,
                          &semaphore_create_info, nullptr, &render_finished_semaphore);
    AssertFalse(res, "vkCreateSemaphore (2) Failed with code %d\n", res);
    res = vkCreateFence(context->device, &fence_create_info, nullptr, &in_flight_fence);
    AssertFalse(res, "vkCreateFence Failed with code %d\n", res);
}

static void FreeSyncObjects(V_VulkanContext* context, V_VulkanPipeline* pipeline) {
    vkDestroyFence(context->device, in_flight_fence, nullptr);
    vkDestroySemaphore(context->device, image_available_semaphore, nullptr);
    vkDestroySemaphore(context->device, render_finished_semaphore, nullptr);
}

static void Draw(V_VulkanContext* context, V_VulkanPipeline* pipeline) {
    vkWaitForFences(context->device, 1, &in_flight_fence, VK_TRUE, U32_MAX);
    vkResetFences(context->device, 1, &in_flight_fence);
    
    u32 image_index;
    vkAcquireNextImageKHR(context->device, context->swapchain, U32_MAX, image_available_semaphore, VK_NULL_HANDLE, &image_index);
    
    vkResetCommandBuffer(pipeline->command_buffer, 0);
    Vulkan_RecordCommandBuffer(context, pipeline, image_index);
    
    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore wait_semaphores[] = { image_available_semaphore };
    VkPipelineStageFlags wait_stage_flags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stage_flags;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &pipeline->command_buffer;
    
    VkSemaphore signal_semaphores[] = { render_finished_semaphore };
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;
    
    VkResult res = vkQueueSubmit(context->graphics_queue, 1, &submit_info, in_flight_fence);
    AssertFalse(res, "vkQueueSubmit Failed with code %d\n", res);
    
    VkPresentInfoKHR present_info = {0};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &context->swapchain;
    present_info.pImageIndices = &image_index;
    
    res = vkQueuePresentKHR(context->present_queue, &present_info);
    AssertFalse(res, "vkQueuePresentKHR Failed with code %d\n", res);
}

int main() {
    M_ScratchInit();
    
    W_Window window = {0};
    V_VulkanContext context = {0};
    V_VulkanPipeline pipeline = {0};
    
    Window_Init(&window);
    I_Init(window.handle);
    
    Vulkan_Init(&window, &context, DEBUG);
    Vulkan_PipelineInit(&context, &pipeline);
    
    CreateSyncObjects(&context, &pipeline);
    
    while (Window_IsOpen(&window)) {
        I_Reset();
        Window_PollEvents(&window);
        
        Draw(&context, &pipeline);
    }
    vkDeviceWaitIdle(context.device);
    
    FreeSyncObjects(&context, &pipeline);
    Vulkan_PipelineFree(&context, &pipeline);
    Vulkan_Free(&context, DEBUG);
    
    Window_Free(&window);
    
    M_ScratchFree();
}
