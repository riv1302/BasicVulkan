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
        SimpleRenderSystem(LveDevice& device, VkRenderPass render_pass);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem &) = delete;
        SimpleRenderSystem &operator = (const SimpleRenderSystem &) = delete;

        void renderGameObjects(
            FrameInfo &frame_info,
            std::vector<LveGameObject>& game_objects
        );

    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass render_pass);

        LveDevice& lve_device;

        std::unique_ptr<LvePipeline> lve_pipeline;
        VkPipelineLayout pipeline_layout;
    };
}