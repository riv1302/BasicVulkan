#pragma once

#include "LveCamera.hpp"
#include "LveDevice.hpp"
#include "LvePipeline.hpp"
#include "LveGameObject.hpp"

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
            VkCommandBuffer commandBuffer,
            std::vector<LveGameObject>& game_objects,
            const LveCamera& camera
        );

    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass render_pass);

        LveDevice& lve_device;

        std::unique_ptr<LvePipeline> lve_pipeline;
        VkPipelineLayout pipeline_layout;
    };
}