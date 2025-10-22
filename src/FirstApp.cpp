#include "FirstApp.hpp"
#include <stdexcept>

namespace lve {
    FirstApp::FirstApp()
    {
        createPipelineLayout();
        createPipeline();
        createCommandBuffers();
    }
    FirstApp::~FirstApp() {
        vkDestroyPipelineLayout(lve_device.device(), pipeline_layout, nullptr);
    }
    void FirstApp::run() {
        while (!lve_window.shouldClose()) {
            glfwPollEvents();
        }        
    }
    void FirstApp::createPipelineLayout() {
        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 0;
        pipeline_layout_info.pSetLayouts = nullptr;
        pipeline_layout_info.pushConstantRangeCount = 0;
        pipeline_layout_info.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(lve_device.device(), &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout!");
    }
    void FirstApp::createPipeline() {
        auto pipeline_config = LvePipeline::defaultPipelineConfigInfo(lve_swap_chain.width(), lve_swap_chain.height());
        pipeline_config.render_pass = lve_swap_chain.getRenderPass();
        pipeline_config.pipeline_layout = pipeline_layout;
        lve_pipeline = std::make_unique<LvePipeline>(
            lve_device,
            "build/shaders/shader.vert.spv",
            "build/shaders/shader.frag.spv",
            pipeline_config
        );
           
    }
    void FirstApp::createCommandBuffers() {
        command_buffers.resize(lve_swap_chain.imageCount());
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = lve_device.getCommandPool();
        alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

        if (vkAllocateCommandBuffers(lve_device.device(), &alloc_info, command_buffers.data()) != VK_SUCCESS)
            throw std::runtime_error("failed to allocate command buffers!");

        for (size_t i = 0; i < command_buffers.size(); ++i) {
            VkCommandBufferBeginInfo begin_info{};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(command_buffers[i], &begin_info) != VK_SUCCESS)
                throw std::runtime_error("failed to begin recordingg command buffer!");

            VkRenderPassBeginInfo render_pass_info{};
            render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        }
    }
    void FirstApp::drawFrame() {

    }
}