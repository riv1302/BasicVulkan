#pragma once

#include "LveWindow.hpp"
#include "LveDevice.hpp"
#include "LveGameObject.hpp"
#include "LveRenderer.hpp"

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
        void loadGameObjects();

        LveWindow lve_window {WIDTH, HEIGHT, "Hello Vulkan!!"};
        LveDevice lve_device {lve_window};
        LveRenderer lve_renderer{lve_window, lve_device};

        std::vector<LveGameObject> game_objects;
    };
}