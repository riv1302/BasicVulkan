#pragma once

#include "scene/LveCamera.hpp"
#include "scene/LveGameObject.hpp"

#include <vulkan/vulkan.h>

#define MAX_LIGHTS 10
#define MAX_WAVES 6
namespace lve {

    struct PointLight {
        glm::vec4 position{};
        glm::vec4 color{};
    };

    struct GpuWave {
        alignas(8) glm::vec2 direction{1.f, 0.f};
        float amplitude{0.f};
        float frequency{0.f};
        float phase{0.f};
        float steepness{0.f};
        float _pad[2]{};
    };

    struct GlobalUbo {
        alignas(16) glm::mat4 projection{1.f};
        alignas(16) glm::mat4 view{1.f};
        glm::vec4 ambient_light_color{1.f, 1.f, 1.f, .02f};
        PointLight point_lights[MAX_LIGHTS];
        int num_lights;

        // Ocean/Sky fields
        alignas(16) glm::mat4 inv_view_proj{1.f};
        alignas(16) glm::vec4 camera_pos{0.f};
        alignas(16) glm::vec4 sun_direction{0.f, -1.f, 0.f, 0.f};
        alignas(16) glm::vec4 sun_color{1.f, 0.95f, 0.8f, 1.f};
        float elapsed_time{0.f};
        float time_of_day{12.f};
        float fog_density{0.002f};
        float time_speed{0.01f};
        GpuWave waves[MAX_WAVES];
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