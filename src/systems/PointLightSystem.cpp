#include "systems/PointLightSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <cassert>
#include <array>

namespace lve {

    struct PointLightPushConstants {
        glm::vec4 position{};
        glm::vec4 color{};
        float radius;
    };

    PointLightSystem::PointLightSystem(
        LveDevice& device,
        VkRenderPass render_pass,
        VkDescriptorSetLayout global_set_layout
    ) :
        lve_device{device}
    {
        createPipelineLayout(global_set_layout);
        createPipeline(render_pass);
    }

    PointLightSystem::~PointLightSystem() {
        vkDestroyPipelineLayout(lve_device.device(), pipeline_layout, nullptr);
    }

    void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout global_set_layout) {
        VkPushConstantRange push_constant_range{};
        push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        push_constant_range.offset = 0;
        push_constant_range.size = sizeof(PointLightPushConstants);

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

    void PointLightSystem::createPipeline(VkRenderPass render_pass) {
        assert(pipeline_layout != nullptr && "Cannot create pipeline before pipeline layout");
            
        PipelineConfigInfo pipeline_config{};
        LvePipeline::defaultPipelineConfigInfo(pipeline_config);
        pipeline_config.binding_descriptions.clear();
        pipeline_config.attribute_descriptions.clear();
        pipeline_config.render_pass = render_pass;
        pipeline_config.pipeline_layout = pipeline_layout;
        lve_pipeline = std::make_unique<LvePipeline>(
            lve_device,
            "./shaders/PointLight.vert.spv",
            "./shaders/PointLight.frag.spv",
            pipeline_config
        );
    }
    
    void PointLightSystem::update(
        FrameInfo &frame_info,        
        GlobalUbo &ubo
    ) {
        auto rotate_light = glm::rotate(
            glm::mat4(1.f),
            frame_info.frame_time,
            {0.f, -1.f, 0.f}
        );

        int light_index = 0;
        for (auto& [_, obj]: frame_info.game_objects) {
            if (obj.point_light == nullptr)
                continue;

            assert(light_index < MAX_LIGHTS && "Point lights exceed maximum specified");

            // Update light position
            obj.transform.translation = glm::vec3(rotate_light * glm::vec4(obj.transform.translation, 1.f));

            // copylight to ubo
            ubo.point_lights[light_index].position = glm::vec4(obj.transform.translation, 1.f);
            ubo.point_lights[light_index].color = glm::vec4(obj.color, obj.point_light->light_intensity);

            light_index++;
        }

        ubo.num_lights = light_index;
    }

    void PointLightSystem::render(
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

        for (auto& [_, obj]: frame_info.game_objects) {
            if (obj.point_light == nullptr) continue;
            
            PointLightPushConstants push{};
            push.position = glm::vec4(obj.transform.translation, 1.f);            
            push.color = glm::vec4(obj.color, obj.point_light->light_intensity);
            push.radius = obj.transform.scale.x;

            vkCmdPushConstants(
                frame_info.command_buffer,
                pipeline_layout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(PointLightPushConstants),
                &push
            );
            vkCmdDraw(frame_info.command_buffer, 6, 1, 0, 0);
        }
    }
}