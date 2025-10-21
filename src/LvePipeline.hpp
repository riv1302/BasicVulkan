#pragma once

#include "LveDevice.hpp"

#include <string>
#include <vector>

namespace lve {
    struct PipelineConfigInfo {
        VkViewport viewport;
        VkRect2D scissor;
        VkPipelineViewportStateCreateInfo viewport_info;
        VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
        VkPipelineRasterizationStateCreateInfo rasterization_info;
        VkPipelineMultisampleStateCreateInfo multisample_info;
        VkPipelineColorBlendAttachmentState color_blend_attachment;
        VkPipelineColorBlendStateCreateInfo color_blend_info;
        VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
        VkPipelineLayout pipeline_layout = nullptr;
        VkRenderPass render_pass = nullptr;
        uint32_t subpass = 0;
    };

    class LvePipeline {
    public:
        LvePipeline(
            LveDevice& device,
            const std::string& vert_filepath,
            const std::string& frag_filepath,
            const PipelineConfigInfo& config_info
        );
        ~LvePipeline();
        LvePipeline(const LvePipeline&) = delete;
        LvePipeline operator=(const LvePipeline&) = delete;

        static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);
    private:
        static std::vector<char> readFile(const std::string& filepath);
        
        void createGraphicsPipeline(
            const std::string& vert_filepath,
            const std::string& frag_filepath,
            const PipelineConfigInfo& config_info);
        
        void createShaderModule(const std::vector<char>& code, VkShaderModule* shader_module);

        LveDevice& lve_device;
        VkPipeline graphics_pipeline;
        VkShaderModule vert_shader_module;
        VkShaderModule frag_shader_module;
    };
}