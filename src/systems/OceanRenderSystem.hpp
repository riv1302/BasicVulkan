#pragma once

#include "core/LveDevice.hpp"
#include "renderer/LvePipeline.hpp"
#include "renderer/LveFrameInfo.hpp"
#include "scene/LveModel.hpp"

#include <memory>

namespace lve {
    class OceanRenderSystem {
    public:
        OceanRenderSystem(LveDevice& device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
        ~OceanRenderSystem();

        OceanRenderSystem(const OceanRenderSystem&) = delete;
        OceanRenderSystem& operator=(const OceanRenderSystem&) = delete;

        void render(FrameInfo& frame_info);

    private:
        void createPipelineLayout(VkDescriptorSetLayout global_set_layout);
        void createPipeline(VkRenderPass render_pass);
        void createOceanMesh();

        LveDevice& lve_device;
        std::unique_ptr<LvePipeline> lve_pipeline;
        VkPipelineLayout pipeline_layout;
        std::unique_ptr<LveModel> ocean_mesh;
    };
}
