#pragma once

#include "scene/LveCamera.hpp"
#include "scene/LveGameObject.hpp"

#include <vulkan/vulkan.h>

#define MAX_LIGHTS 10
namespace lve {

    struct PointLight {
        glm::vec4 position{};
        glm::vec4 color{};
    };

    struct GlobalUbo {
        alignas(16) glm::mat4 projection{1.f};
        alignas(16) glm::mat4 view{1.f};
        glm::vec4 ambient_light_color{1.f, 1.f, 1.f, .02f};
        PointLight point_lights[MAX_LIGHTS];
        int num_lights;

        // Ocean/Sky fields (Phase 1+)
        alignas(16) glm::mat4 inv_view_proj{1.f};
        alignas(16) glm::vec4 camera_pos{0.f};
        alignas(16) glm::vec4 sun_direction{0.f, -1.f, 0.f, 0.f};
        alignas(16) glm::vec4 sun_color{1.f, 0.95f, 0.8f, 1.f};
        float elapsed_time{0.f};
        float time_of_day{12.f};
    };

    struct FrameInfo {
        int frame_index;
        float frame_time;
        VkCommandBuffer command_buffer;
        LveCamera &camera;
        VkDescriptorSet global_descriptor_set;
        LveGameObject::Map &game_objects;
    };
}