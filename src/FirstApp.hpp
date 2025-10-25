#pragma once

#include "LveWindow.hpp"
#include "LveDevice.hpp"
#include "LveSwapChain.hpp"
#include "LvePipeline.hpp"
#include "LveModel.hpp"

#include <memory>
#include <vector>

namespace lve {
    class FirstApp {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp &) = delete;
        FirstApp &operator = (const FirstApp &) = delete;

        void run();

    private:
        void loadModels();
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void drawFrame();

        LveWindow lve_window {WIDTH, HEIGHT, "Hello Vulkan!!"};
        LveDevice lve_device {lve_window};
        LveSwapChain lve_swap_chain {
            lve_device,
            lve_window.getExtent()
        };
        std::unique_ptr<LvePipeline> lve_pipeline;
        VkPipelineLayout pipeline_layout;
        std::vector<VkCommandBuffer> command_buffers;
        std::unique_ptr<LveModel> lve_model;
    };
}