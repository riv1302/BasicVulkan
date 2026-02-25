#pragma once

#include "core/LveDevice.hpp"
#include "renderer/LvePipeline.hpp"
#include "renderer/LveFrameInfo.hpp"

#include <memory>

namespace lve {
    class SkyRenderSystem {
    public:
        SkyRenderSystem(LveDevice& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
        ~SkyRenderSystem();

        SkyRenderSystem(const SkyRenderSystem&) = delete;
        SkyRenderSystem& operator=(const SkyRenderSystem&) = delete;

        void render(FrameInfo& frame_info);

    private:
        void createPipelineLayout(VkDescriptorSetLayout global_set_layout);
        void createPipeline(VkRenderPass render_pass);

        LveDevice& lve_device;
        std::unique_ptr<LvePipeline> lve_pipeline;
        VkPipelineLayout pipeline_layout;
    };
}
