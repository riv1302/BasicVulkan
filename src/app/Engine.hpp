#pragma once

#include "core/LveWindow.hpp"
#include "core/LveDevice.hpp"
#include "core/LveBuffer.hpp"
#include "renderer/LveRenderer.hpp"
#include "renderer/LveDescriptors.hpp"
#include "renderer/LveFrameInfo.hpp"
#include "scene/Scene.hpp"

#include <memory>
#include <string>
#include <vector>

namespace lve {

    class Engine {
    public:
        Engine(int width, int height, const std::string& title);
        ~Engine();

        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;

        void run();
        void switchScene(std::unique_ptr<Scene> new_scene);

        LveDevice& getDevice() { return lve_device; }
        VkRenderPass getRenderPass() { return lve_renderer.getSwapChainRenderPass(); }
        VkDescriptorSetLayout getGlobalDescriptorSetLayout() { return global_set_layout->getDescriptorSetLayout(); }

    private:
        void initGlobalResources();
        void handleSceneSwitch();

        LveWindow lve_window;
        LveDevice lve_device{lve_window};
        LveRenderer lve_renderer{lve_window, lve_device};

        std::unique_ptr<LveDescriptorPool> global_pool;
        std::vector<std::unique_ptr<LveBuffer>> ubo_buffers;
        std::unique_ptr<LveDescriptorSetLayout> global_set_layout;
        std::vector<VkDescriptorSet> global_descriptor_sets;

        std::unique_ptr<Scene> current_scene;
        std::unique_ptr<Scene> pending_scene;

        float elapsed_time_total{0.f};
    };
}