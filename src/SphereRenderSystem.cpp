#include "SphereRenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <cassert>
#include <array>

namespace lve {
    struct SimplePushConstantData {
        glm::mat2 transform{1.f};
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };

    SphereRenderSystem::SphereRenderSystem(
        LveDevice& device,
        VkRenderPass render_pass
    ) :
        lve_device{device}
    {
        createPipelineLayout();
        createPipeline(render_pass);
    }

    SphereRenderSystem::~SphereRenderSystem() {
        vkDestroyPipelineLayout(lve_device.device(), pipeline_layout, nullptr);
    }

    void SphereRenderSystem::createPipelineLayout() {
        VkPushConstantRange push_constant_range{};
        push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        push_constant_range.offset = 0;
        push_constant_range.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 0;
        pipeline_layout_info.pSetLayouts = nullptr;
        pipeline_layout_info.pushConstantRangeCount = 1;
        pipeline_layout_info.pPushConstantRanges = &push_constant_range;

        if (vkCreatePipelineLayout(lve_device.device(), &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout!");
    }

    void SphereRenderSystem::createPipeline(VkRenderPass render_pass) {
        assert(pipeline_layout != nullptr && "Cannot create pipeline before pipeline layout");
            
        PipelineConfigInfo pipeline_config{};
        LvePipeline::defaultPipelineConfigInfo(pipeline_config);
        pipeline_config.render_pass = render_pass;
        pipeline_config.pipeline_layout = pipeline_layout;
        lve_pipeline = std::make_unique<LvePipeline>(
            lve_device,
            "build/shaders/ShaderGravity.vert.spv",
            "build/shaders/ShaderGravity.frag.spv",
            pipeline_config
        );
    }

    void SphereRenderSystem::renderGameObjects(VkCommandBuffer command_buffer, std::vector<LveGameObject>& game_objects) {
        lve_pipeline->bind(command_buffer);
        for (auto& obj: game_objects) {
            SimplePushConstantData push{};
            push.offset = obj.transform2d.translation;
            push.color = obj.color;
            push.transform = obj.transform2d.mat2();
            vkCmdPushConstants(
                command_buffer,
                pipeline_layout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push);
            obj.model->bind(command_buffer);
            obj.model->draw(command_buffer);
        }
    }
}