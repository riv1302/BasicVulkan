#include "systems/SkyRenderSystem.hpp"

#include <stdexcept>
#include <cassert>

namespace lve {

    SkyRenderSystem::SkyRenderSystem(
        LveDevice& device,
        VkRenderPass render_pass,
        VkDescriptorSetLayout global_set_layout
    ) : lve_device{device}
    {
        createPipelineLayout(global_set_layout);
        createPipeline(render_pass);
    }

    SkyRenderSystem::~SkyRenderSystem() {
        vkDestroyPipelineLayout(lve_device.device(), pipeline_layout, nullptr);
    }

    void SkyRenderSystem::createPipelineLayout(VkDescriptorSetLayout global_set_layout) {
        std::vector<VkDescriptorSetLayout> descriptor_set_layouts{global_set_layout};

        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
        pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
        pipeline_layout_info.pushConstantRangeCount = 0;
        pipeline_layout_info.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(lve_device.device(), &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
            throw std::runtime_error("failed to create sky pipeline layout!");
    }

    void SkyRenderSystem::createPipeline(VkRenderPass render_pass) {
        assert(pipeline_layout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipeline_config{};
        LvePipeline::defaultPipelineConfigInfo(pipeline_config);

        pipeline_config.binding_descriptions.clear();
        pipeline_config.attribute_descriptions.clear();

        pipeline_config.depth_stencil_info.depthTestEnable = VK_FALSE;
        pipeline_config.depth_stencil_info.depthWriteEnable = VK_FALSE;

        pipeline_config.render_pass = render_pass;
        pipeline_config.pipeline_layout = pipeline_layout;

        lve_pipeline = std::make_unique<LvePipeline>(
            lve_device,
            "./shaders/Sky.vert.spv",
            "./shaders/Sky.frag.spv",
            pipeline_config
        );
    }

    void SkyRenderSystem::render(FrameInfo& frame_info) {
        lve_pipeline->bind(frame_info.command_buffer);

        vkCmdBindDescriptorSets(
            frame_info.command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline_layout,
            0, 1,
            &frame_info.global_descriptor_set,
            0,
            nullptr
        );

        vkCmdDraw(frame_info.command_buffer, 3, 1, 0, 0);
    }
}
