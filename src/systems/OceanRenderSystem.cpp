#include "systems/OceanRenderSystem.hpp"

#include <stdexcept>
#include <cassert>

namespace lve {

    static constexpr int GRID_SIZE = 256;
    static constexpr int VERTEX_COUNT = GRID_SIZE + 1; // 257
    static constexpr float GRID_SPACING = 1.0f;

    OceanRenderSystem::OceanRenderSystem(
        LveDevice& device,
        VkRenderPass render_pass,
        VkDescriptorSetLayout global_set_layout
    ) : lve_device{device}
    {
        createPipelineLayout(global_set_layout);
        createPipeline(render_pass);
        createOceanMesh();
    }

    OceanRenderSystem::~OceanRenderSystem() {
        vkDestroyPipelineLayout(lve_device.device(), pipeline_layout, nullptr);
    }

    void OceanRenderSystem::createPipelineLayout(VkDescriptorSetLayout global_set_layout) {
        std::vector<VkDescriptorSetLayout> descriptor_set_layouts{global_set_layout};

        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
        pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
        pipeline_layout_info.pushConstantRangeCount = 0;
        pipeline_layout_info.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(lve_device.device(), &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS)
            throw std::runtime_error("failed to create ocean pipeline layout!");
    }

    void OceanRenderSystem::createPipeline(VkRenderPass render_pass) {
        assert(pipeline_layout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipeline_config{};
        LvePipeline::defaultPipelineConfigInfo(pipeline_config);

        pipeline_config.binding_descriptions = LveModel::Vertex::getBindingDescriptions();
        pipeline_config.attribute_descriptions = LveModel::Vertex::getAttributeDescriptions();

        pipeline_config.depth_stencil_info.depthTestEnable = VK_TRUE;
        pipeline_config.depth_stencil_info.depthWriteEnable = VK_TRUE;

        pipeline_config.render_pass = render_pass;
        pipeline_config.pipeline_layout = pipeline_layout;

        lve_pipeline = std::make_unique<LvePipeline>(
            lve_device,
            "./shaders/Ocean.vert.spv",
            "./shaders/Ocean.frag.spv",
            pipeline_config
        );
    }

    void OceanRenderSystem::createOceanMesh() {
        LveModel::Builder builder{};

        float half_extent = (GRID_SIZE * GRID_SPACING) * 0.5f;

        // Generate vertices: 257 x 257 grid on XZ plane, Y = 0
        builder.vertices.reserve(VERTEX_COUNT * VERTEX_COUNT);
        for (int z = 0; z < VERTEX_COUNT; z++) {
            for (int x = 0; x < VERTEX_COUNT; x++) {
                LveModel::Vertex vertex{};
                vertex.position = {
                    x * GRID_SPACING - half_extent,
                    0.0f,
                    z * GRID_SPACING - half_extent
                };
                vertex.normal = {0.f, -1.f, 0.f}; // up in Y-down convention
                vertex.color = {0.f, 0.f, 1.f};
                builder.vertices.push_back(vertex);
            }
        }

        // Generate indices: two triangles per quad
        builder.indices.reserve(GRID_SIZE * GRID_SIZE * 6);
        for (int z = 0; z < GRID_SIZE; z++) {
            for (int x = 0; x < GRID_SIZE; x++) {
                uint32_t top_left = z * VERTEX_COUNT + x;
                uint32_t top_right = top_left + 1;
                uint32_t bottom_left = (z + 1) * VERTEX_COUNT + x;
                uint32_t bottom_right = bottom_left + 1;

                // First triangle
                builder.indices.push_back(top_left);
                builder.indices.push_back(bottom_left);
                builder.indices.push_back(top_right);

                // Second triangle
                builder.indices.push_back(top_right);
                builder.indices.push_back(bottom_left);
                builder.indices.push_back(bottom_right);
            }
        }

        ocean_mesh = std::make_unique<LveModel>(lve_device, builder);
    }

    void OceanRenderSystem::render(FrameInfo& frame_info) {
        lve_pipeline->bind(frame_info.command_buffer);

        vkCmdBindDescriptorSets(
            frame_info.command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline_layout,
            0, 1,
            &frame_info.global_descriptor_set,
            0,
            nullptr
        );

        ocean_mesh->bind(frame_info.command_buffer);
        ocean_mesh->draw(frame_info.command_buffer);
    }
}
