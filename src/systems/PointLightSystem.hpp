#pragma once

#include "scene/LveCamera.hpp"
#include "core/LveDevice.hpp"
#include "renderer/LvePipeline.hpp"
#include "scene/LveGameObject.hpp"
#include "renderer/LveFrameInfo.hpp"

#include <memory>
#include <vector>

namespace lve {
    class PointLightSystem {
    public:
        PointLightSystem(LveDevice& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
        ~PointLightSystem();

        PointLightSystem(const PointLightSystem &) = delete;
        PointLightSystem &operator = (const PointLightSystem &) = delete;

        void update(
            FrameInfo &frame_info,        
            GlobalUbo &ubo
        );
        void render(
            FrameInfo &frame_info
        );

    private:
        void createPipelineLayout(VkDescriptorSetLayout global_set_layout);
        void createPipeline(VkRenderPass render_pass);

        LveDevice& lve_device;

        std::unique_ptr<LvePipeline> lve_pipeline;
        VkPipelineLayout pipeline_layout;
    };
}