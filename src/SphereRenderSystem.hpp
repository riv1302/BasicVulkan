#pragma once

#include "LveDevice.hpp"
#include "LvePipeline.hpp"
#include "LveGameObject.hpp"

#include <memory>
#include <vector>

namespace lve {
    class SphereRenderSystem {
    public:
        SphereRenderSystem(LveDevice& device, VkRenderPass render_pass);
        ~SphereRenderSystem();

        SphereRenderSystem(const SphereRenderSystem &) = delete;
        SphereRenderSystem &operator = (const SphereRenderSystem &) = delete;

        void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<LveGameObject>& game_objects);

    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass render_pass);

        LveDevice& lve_device;

        std::unique_ptr<LvePipeline> lve_pipeline;
        VkPipelineLayout pipeline_layout;
    };
}