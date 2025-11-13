#include "LveModel.hpp"

#include "LveUtils.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <TinyObjLoader/TinyObjLoader.h>

// #define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <cassert>
#include <cstring>
#include <unordered_map>

namespace std {
    template <>
    struct hash<lve::LveModel::Vertex> {
        size_t operator()(const lve::LveModel::Vertex& vertex) const {
            size_t seed = 0;
            lve::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

namespace lve {
    LveModel::LveModel(
        LveDevice& device,
        const LveModel::Builder& builder
    ): lve_device(device) {
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
    }

    LveModel::~LveModel() {
        vkDestroyBuffer(lve_device.device(), vertex_buffer, nullptr);
        vkFreeMemory(lve_device.device(), vertex_buffer_memory, nullptr);

        if (has_index_buffer) {
            vkDestroyBuffer(lve_device.device(), index_buffer, nullptr);
            vkFreeMemory(lve_device.device(), index_buffer_memory, nullptr);
        }
    }
    
    std::unique_ptr<LveModel> LveModel::createModelFromFile(
        LveDevice& device,
        const std::string& filepath
    ) {
        Builder builder{};
        builder.loadModel(filepath);
        return std::make_unique<LveModel>(device, builder);
    }
    
    void LveModel::createVertexBuffers(const std::vector<Vertex>& vertices) {
        vertex_count = static_cast<uint32_t>(vertices.size());
        assert(vertex_count >= 3 && "Vertex count must be at least 3");
        VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count;

        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;
        lve_device.createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer,
            staging_buffer_memory
        );

        void *data;
        vkMapMemory(lve_device.device(), staging_buffer_memory, 0, buffer_size, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
        vkUnmapMemory(lve_device.device(), staging_buffer_memory);
        
        lve_device.createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertex_buffer,
            vertex_buffer_memory
        );

        lve_device.copyBuffer(staging_buffer, vertex_buffer, buffer_size);

        vkDestroyBuffer(lve_device.device(), staging_buffer, nullptr);
        vkFreeMemory(lve_device.device(), staging_buffer_memory, nullptr);
    }

    void LveModel::createIndexBuffers(const std::vector<uint32_t>& indices) {
        index_count = static_cast<uint32_t>(indices.size());
        has_index_buffer = index_count > 0;
        if (!has_index_buffer)
            return;

        VkDeviceSize buffer_size = sizeof(indices[0]) * index_count;

        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;
        lve_device.createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer,
            staging_buffer_memory
        );

        void *data;
        vkMapMemory(lve_device.device(), staging_buffer_memory, 0, buffer_size, 0, &data);
        memcpy(data, indices.data(), static_cast<size_t>(buffer_size));
        vkUnmapMemory(lve_device.device(), staging_buffer_memory);
        
        lve_device.createBuffer(
            buffer_size,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT |VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            index_buffer,
            index_buffer_memory
        );

        lve_device.copyBuffer(staging_buffer, index_buffer, buffer_size);

        vkDestroyBuffer(lve_device.device(), staging_buffer, nullptr);
        vkFreeMemory(lve_device.device(), staging_buffer_memory, nullptr);
    }
    
    void LveModel::bind(VkCommandBuffer command_buffer) {
        VkBuffer buffers[] = {vertex_buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

        if (has_index_buffer)
        vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void LveModel::draw(VkCommandBuffer command_buffer) {
        if (has_index_buffer)
            vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);
        else
            vkCmdDraw(command_buffer, vertex_count, 1, 0, 0);
    }
    
    std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
        binding_descriptions[0].binding = 0;
        binding_descriptions[0].stride = sizeof(Vertex);
        binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding_descriptions;
    }
    std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions(2);
        attribute_descriptions[0].binding = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[0].offset = offsetof(Vertex, position);

        attribute_descriptions[1].binding = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[1].offset = offsetof(Vertex, color);

        return attribute_descriptions;
    }

    void LveModel::Builder::loadModel(
        const std::string&filepath
    ) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string warn, err;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str()))
            throw std::runtime_error(warn + err);

        vertices.clear();
        indices.clear();

        std::unordered_map<Vertex, uint32_t> unique_verteces{};
        for (const auto& shape: shapes) {
            for (const auto& index: shape.mesh.indices) {
                Vertex vertex{};
                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3*index.vertex_index + 0],
                        attrib.vertices[3*index.vertex_index + 1],
                        attrib.vertices[3*index.vertex_index + 2]
                    };

                    auto color_index = 3*index.vertex_index + 2;
                    if (color_index < attrib.colors.size()) {
                        vertex.color =  {
                            attrib.colors[color_index - 2],
                            attrib.colors[color_index - 1],
                            attrib.colors[color_index - 0]
                        };
                    }
                    else {
                        // Default color
                        vertex.color = {1.f, 1.f, 1.f};
                    }
                }

                if (index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3*index.normal_index + 0],
                        attrib.normals[3*index.normal_index + 1],
                        attrib.normals[3*index.normal_index + 2]
                    };
                }
                
                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                        attrib.texcoords[2*index.texcoord_index + 0],
                        attrib.texcoords[2*index.texcoord_index + 1]
                    };
                }

                if (unique_verteces.count(vertex) == 0) {
                    unique_verteces[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(unique_verteces[vertex]);
            }
        }
    }
}