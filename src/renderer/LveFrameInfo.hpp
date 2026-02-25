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