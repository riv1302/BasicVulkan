#pragma once

#include "LveWindow.hpp"
#include "LveDevice.hpp"
#include "LvePipeline.hpp"

namespace lve {
    class FirstApp {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        void run();

    private:
        LveWindow lve_window{WIDTH, HEIGHT, "Hello Vulkan!!"};
        LveDevice lve_device{lve_window};
        LvePipeline lve_pipeline{lve_device, "build/shaders/shader.vert.spv", "build/shaders/shader.frag.spv", LvePipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
    };
}