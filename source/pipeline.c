#include "pipeline.h"

static u8* V_LoadFile(M_Arena* arena, string filepath, u64* size) {
    FILE* file = fopen((const char*)filepath.str, "rb");
    
    fseek(file, 0L, SEEK_END);
    size_t filesize = ftell(file);
    rewind(file);
    
    u8* buffer = arena_alloc(arena, filesize + 1);
    *size = fread(buffer, sizeof(u8), filesize, file);
    buffer[*size] = '\0';
    
    fclose(file);
    return buffer;
}

static VkShaderModule V_CreateShaderModule(V_VulkanContext* context, u8* data, u32 size) {
    VkShaderModuleCreateInfo module_create_info = {0};
    module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_create_info.codeSize = size;
    module_create_info.pCode = (u32*)data;
    VkShaderModule ret;
    
    VkResult res = vkCreateShaderModule(context->device, &module_create_info, nullptr, &ret);
    AssertFalse(res, "vkCreateShaderModule Failed with code %d\n", res);
    return ret;
}

static b8 V_CreateRenderpass(V_VulkanContext* context, V_VulkanPipeline* pipeline) {
    VkAttachmentDescription color_attachment = {0};
    color_attachment.format = context->swapchain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference color_attachment_ref = {0};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass0_desc = {0};
    subpass0_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass0_desc.colorAttachmentCount = 1;
    subpass0_desc.pColorAttachments = &color_attachment_ref;
    
    VkRenderPassCreateInfo renderpass_create_info = {0};
    renderpass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpass_create_info.attachmentCount = 1;
    renderpass_create_info.pAttachments = &color_attachment;
    renderpass_create_info.subpassCount = 1;
    renderpass_create_info.pSubpasses = &subpass0_desc;
    
    VkSubpassDependency subpass_dependency = {0};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    renderpass_create_info.dependencyCount = 1;
    renderpass_create_info.pDependencies = &subpass_dependency;
    
    VkResult res = vkCreateRenderPass(context->device, &renderpass_create_info, nullptr, &pipeline->renderpass);
    AssertFalse(res, "vkCreateRenderPass Failed with code %d\n", res);
    
    
    return true;
}

static b8 V_CreatePipelineLayout(V_VulkanContext* context, V_VulkanPipeline* pipeline) {
    M_Scratch scratch = scratch_get();
    u64 vert_code_size, frag_code_size;
    u8* vert_code = V_LoadFile(&scratch.arena, str_lit("res/basic.vert.spv"), &vert_code_size);
    u8* frag_code = V_LoadFile(&scratch.arena, str_lit("res/basic.frag.spv"), &frag_code_size);
    
    VkShaderModule vertex_shader = V_CreateShaderModule(context, vert_code, vert_code_size);
    VkShaderModule fragment_shader = V_CreateShaderModule(context, frag_code, frag_code_size);
    
    //- Shader stages 
    VkPipelineShaderStageCreateInfo vert_shader_stage_create_info = {0};
    vert_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_create_info.module = vertex_shader;
    vert_shader_stage_create_info.pName = "main";
    
    VkPipelineShaderStageCreateInfo frag_shader_stage_create_info = {0};
    frag_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_create_info.module = fragment_shader;
    frag_shader_stage_create_info.pName = "main";
    
    VkPipelineShaderStageCreateInfo shader_stages[] = {
        vert_shader_stage_create_info, frag_shader_stage_create_info
    };
    
    //- Vertex Input 
    VkPipelineVertexInputStateCreateInfo vertex_input_create_info = {0};
    vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_create_info.vertexBindingDescriptionCount = 0;
    vertex_input_create_info.pVertexBindingDescriptions = nullptr;
    vertex_input_create_info.vertexAttributeDescriptionCount = 0;
    vertex_input_create_info.pVertexAttributeDescriptions = nullptr;
    
    //- Input Assembly 
    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {0};
    input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_create_info.primitiveRestartEnable = false;
    
    //- Viewport and Scissor 
    VkViewport viewport = {0};
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = (f32) context->swapchain_extent.width;
    viewport.height = (f32) context->swapchain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    VkRect2D scissor = {0};
    scissor.offset = (VkOffset2D) { 0, 0 };
    scissor.extent = context->swapchain_extent;
    
    VkPipelineViewportStateCreateInfo viewport_create_info = {0};
    viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_create_info.viewportCount = 1;
    viewport_create_info.pViewports = &viewport;
    viewport_create_info.scissorCount = 1;
    viewport_create_info.pScissors = &scissor;
    
    //- Rasterizer 
    VkPipelineRasterizationStateCreateInfo rasterization_create_info = {0};
    rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_create_info.depthClampEnable = VK_FALSE;
    rasterization_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_create_info.lineWidth = 1.f;
    rasterization_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterization_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // @deviation
    rasterization_create_info.depthBiasEnable = VK_FALSE;
    
    //- Multisampling 
    VkPipelineMultisampleStateCreateInfo multisample_create_info = {0};
    multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_create_info.sampleShadingEnable = VK_FALSE;
    multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    //- Depth Stencil (nullptr) 
    
    //- Color Blending 
    VkPipelineColorBlendAttachmentState color_blend_attachment = {0};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    
    VkPipelineColorBlendStateCreateInfo color_blend_create_info = {0};
    color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_create_info.logicOpEnable = VK_FALSE;
    color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_create_info.attachmentCount = 1;
    color_blend_create_info.pAttachments = &color_blend_attachment;
    color_blend_create_info.blendConstants[0] = 0.f;
    color_blend_create_info.blendConstants[1] = 0.f;
    color_blend_create_info.blendConstants[2] = 0.f;
    color_blend_create_info.blendConstants[3] = 0.f;
    
    //- Dynamic State (nullptr) 
    /*
u32 dynamic_state_count = 2;
    VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };
    
    VkPipelineDynamicStateCreateInfo dynamic_create_info = {0};
    dynamic_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_create_info.dynamicStateCount = dynamic_state_count;
    dynamic_create_info.pDynamicStates = dynamic_states;
    */
    
    //- Pipeline Layout 
    VkPipelineLayoutCreateInfo layout_create_info = {0};
    layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_create_info.setLayoutCount = 0;
    layout_create_info.pSetLayouts = nullptr;
    layout_create_info.pushConstantRangeCount = 0;
    layout_create_info.pPushConstantRanges = nullptr;
    
    VkResult res = vkCreatePipelineLayout(context->device, &layout_create_info, nullptr, &pipeline->layout);
    AssertFalse(res, "vkCreatePipelineLayout Failed with code %d\n", res);
    
    //- THE PIPELINE FINALLY BAYBEEE LESGOOOOO 
    VkGraphicsPipelineCreateInfo pipeline_create_info = {0};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.stageCount = 2;
    pipeline_create_info.pStages = shader_stages;
    pipeline_create_info.pVertexInputState = &vertex_input_create_info;
    pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
    pipeline_create_info.pViewportState = &viewport_create_info;
    pipeline_create_info.pRasterizationState = &rasterization_create_info;
    pipeline_create_info.pMultisampleState = &multisample_create_info;
    pipeline_create_info.pDepthStencilState = nullptr;
    pipeline_create_info.pColorBlendState = &color_blend_create_info;
    pipeline_create_info.pDynamicState = nullptr;
    pipeline_create_info.layout = pipeline->layout;
    pipeline_create_info.renderPass = pipeline->renderpass;
    pipeline_create_info.subpass = 0; // Subpass index 0
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_create_info.basePipelineIndex = -1;
    
    res = vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &pipeline->handle);
    AssertFalse(res, "vkCreateGraphicsPipelines Failed with code %d\n", res);
    
    //- End 
    
    vkDestroyShaderModule(context->device, vertex_shader, nullptr);
    vkDestroyShaderModule(context->device, fragment_shader, nullptr);
    
    scratch_return(&scratch);
    return true;
}

static b8 V_CreateFramebuffers(V_VulkanContext* context, V_VulkanPipeline* pipeline) {
    pipeline->framebuffers = arena_alloc(&pipeline->arena, context->swapchain_image_count * sizeof(VkFramebuffer));
    
    for (u32 i = 0; i < context->swapchain_image_count; i++) {
        VkFramebufferCreateInfo framebuffer_create_info = {0};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = pipeline->renderpass;
        framebuffer_create_info.attachmentCount = 1;
        framebuffer_create_info.pAttachments = &context->swapchain_image_views[i];
        framebuffer_create_info.width = context->swapchain_extent.width;
        framebuffer_create_info.height = context->swapchain_extent.height;
        framebuffer_create_info.layers = 1;
        
        
        VkResult res = vkCreateFramebuffer(context->device, &framebuffer_create_info, nullptr, &pipeline->framebuffers[i]);
        AssertFalse(res, "Pipeline vkCreateFramebuffer Failed with code %d\n", res);
    }
    
    return true;
}

static b8 V_CreateCommandPoolAndBuffer(V_VulkanContext* context, V_VulkanPipeline* pipeline) {
    V_QueueFamilyIndices indices = V_FindQueueFamilies(context, context->physical_device);
    
    VkCommandPoolCreateInfo command_pool_create_info = {0};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = indices.graphics_family.value;
    
    VkResult res = vkCreateCommandPool(context->device, &command_pool_create_info, nullptr, &pipeline->command_pool);
    AssertFalse(res, "vkCreateCommandPool Failed with code %d\n", res);
    
    VkCommandBufferAllocateInfo command_buffer_allocation_info = {0};
    command_buffer_allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocation_info.commandPool = pipeline->command_pool;
    command_buffer_allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocation_info.commandBufferCount = 1;
    
    res = vkAllocateCommandBuffers(context->device, &command_buffer_allocation_info, &pipeline->command_buffer);
    AssertFalse(res, "vkAllocateCommandBuffers Failed with code %d\n", res);
    return true;
}

void Vulkan_PipelineInit(V_VulkanContext* context, V_VulkanPipeline* pipeline) {
    arena_init(&pipeline->arena);
    
    Assert(V_CreateRenderpass(context, pipeline), "Renderpass Creation Failed\n");
    Assert(V_CreatePipelineLayout(context, pipeline), "Pipeline Layout Creation Failed\n");
    Assert(V_CreateFramebuffers(context, pipeline), "Framebuffer Creation Failed\n");
    Assert(V_CreateCommandPoolAndBuffer(context, pipeline), "Command Pool Creation Failed\n");
}

void Vulkan_RecordCommandBuffer(V_VulkanContext* context, V_VulkanPipeline* pipeline, u32 image_index) {
    VkResult res;
    
    VkCommandBufferBeginInfo command_buffer_begin_info = {0};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    res = vkBeginCommandBuffer(pipeline->command_buffer, &command_buffer_begin_info);
    AssertFalse(res, "vkBeginCommandBuffer Failed with code %d\n", res);
    
    VkRenderPassBeginInfo render_pass_begin_info = {0};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = pipeline->renderpass;
    render_pass_begin_info.framebuffer = pipeline->framebuffers[image_index];
    render_pass_begin_info.renderArea.offset = (VkOffset2D) { 0.f, 0.f };
    render_pass_begin_info.renderArea.extent = context->swapchain_extent;
    
    // CLEAR COLOR
    VkClearValue clear_color = { .color = { .float32 = { 0.0f, 0.0f, 0.0f, 1.0f } } };
    render_pass_begin_info.clearValueCount = 1;
    render_pass_begin_info.pClearValues = &clear_color;
    
    vkCmdBeginRenderPass(pipeline->command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(pipeline->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);
    vkCmdDraw(pipeline->command_buffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(pipeline->command_buffer);
    
    res = vkEndCommandBuffer(pipeline->command_buffer);
    AssertFalse(res, "vkEndCommandBuffer Failed with code %d\n", res);
}

void Vulkan_PipelineFree(V_VulkanContext* context, V_VulkanPipeline* pipeline) {
    vkDestroyCommandPool(context->device, pipeline->command_pool, nullptr);
    
    for (u32 i = 0; i < context->swapchain_image_count; i++) {
        vkDestroyFramebuffer(context->device, pipeline->framebuffers[i], nullptr);
    }
    
    vkDestroyPipeline(context->device, pipeline->handle, nullptr);
    vkDestroyPipelineLayout(context->device, pipeline->layout, nullptr);
    vkDestroyRenderPass(context->device, pipeline->renderpass, nullptr);
    
    arena_free(&pipeline->arena);
}
