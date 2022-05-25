/* date = May 25th 2022 11:18 am */

#ifndef PIPELINE_H
#define PIPELINE_H

#include "base/mem.h"
#include "context.h"

typedef struct V_VulkanPipeline {
    M_Arena arena;
    
    VkPipeline handle;
    VkRenderPass renderpass;
    VkPipelineLayout layout;
    
    u32 framebuffer_count;
    VkFramebuffer* framebuffers;
    
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
} V_VulkanPipeline;

void Vulkan_PipelineInit(V_VulkanContext* context, V_VulkanPipeline* pipeline);
void Vulkan_RecordCommandBuffer(V_VulkanContext* context, V_VulkanPipeline* pipeline, u32 image_index);
void Vulkan_PipelineFree(V_VulkanContext* context, V_VulkanPipeline* pipeline);

#endif //PIPELINE_H
