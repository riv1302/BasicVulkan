#pragma once

#include "LveCamera.hpp"
#include "LveDevice.hpp"
#include "LvePipeline.hpp"
#include "LveGameObject.hpp"
#include "LveFrameInfo.hpp"

#include <memory>
#include <vector>

namespace lve {
    class SimpleRenderSystem {
    public:
        SimpleRenderSystem(LveDevice& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem &) = delete;
        SimpleRenderSystem &operator = (const SimpleRenderSystem &) = delete;

        void renderGameObjects(
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