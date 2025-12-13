#include "SimpleRenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <cassert>
#include <array>

namespace lve {
    struct SimplePushConstantData {
        glm::mat4 model_matrix{1.f};
        glm::mat4 normal_matrix{1.f};
        //alignas(16) glm::vec3 color;
    };

    SimpleRenderSystem::SimpleRenderSystem(
        LveDevice& device,
        VkRenderPass render_pass,
        VkDescriptorSetLayout global_set_layout
    ) :
        lve_device{device}
    {
        createPipelineLayout(global_set_layout);
        createPipeline(render_pass);
    }

    SimpleRenderSystem::~SimpleRenderSystem() {
        vkDestroyPipelineLayout(lve_device.device(), pipeline_layout, nullptr);
    }

    void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout global_set_layout) {
        VkPushConstantRange push_constant_range{};
        push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        push_constant_range.offset = 0;
        push_constant_range.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptor_set_layouts{global_set_layout};

        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
        pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
        pipeline_layout_info.pushConstantRangeCount = 1;
        pipeline_layout_info.pPushConstantRanges = &push_constant_range;

        if (vkCreatePipelineLayout(lve_device.device(), &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout!");
    }

    void SimpleRenderSystem::createPipeline(VkRenderPass render_pass) {
        assert(pipeline_layout != nullptr && "Cannot create pipeline before pipeline layout");
            
        PipelineConfigInfo pipeline_config{};
        LvePipeline::defaultPipelineConfigInfo(pipeline_config);
        pipeline_config.render_pass = render_pass;
        pipeline_config.pipeline_layout = pipeline_layout;
        lve_pipeline = std::make_unique<LvePipeline>(
            lve_device,
            "./shaders/shader.vert.spv",
            "./shaders/shader.frag.spv",
            pipeline_config
        );
    }

    void SimpleRenderSystem::renderGameObjects(
        FrameInfo &frame_info
    ) {
        lve_pipeline->bind(frame_info.command_buffer);

        vkCmdBindDescriptorSets(
            frame_info.command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline_layout,
            0, 1,
            &frame_info.global_desriptor_set,
            0,
            nullptr
        );

        for (auto& [id, obj]: frame_info.game_objects) {
            if (obj.model == nullptr) continue;

            SimplePushConstantData push{};
            push.model_matrix = obj.transform.mat4();
            push.normal_matrix = obj.transform.normal_matrix();
            vkCmdPushConstants(
                frame_info.command_buffer,
                pipeline_layout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push);
            obj.model->bind(frame_info.command_buffer);
            obj.model->draw(frame_info.command_buffer);
        }
    }
}