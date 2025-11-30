#pragma once

#include "LveCamera.hpp"
#include "LveGameObject.hpp"

#include <vulkan/vulkan.h>

namespace lve {
    struct FrameInfo {
        int frame_index;
        float frame_time;
        VkCommandBuffer command_buffer;
        LveCamera &camera;
        VkDescriptorSet global_desriptor_set;
        LveGameObject::Map &game_objects;
    };
}