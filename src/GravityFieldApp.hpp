#pragma once

#include "LveWindow.hpp"
#include "LveDevice.hpp"
#include "LveGameObject.hpp"
#include "LveRenderer.hpp"

#include <memory>
#include <vector>

namespace lve {
    class GravityFieldApp {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 800;

        static constexpr int DENSITY = 40;

        GravityFieldApp();
        ~GravityFieldApp();

        GravityFieldApp(const GravityFieldApp &) = delete;
        GravityFieldApp &operator = (const GravityFieldApp &) = delete;

        void run();
        void calc_next_frame();

    private:
        void loadGameObjects();

        LveWindow lve_window {WIDTH, HEIGHT, "Hello Vulkan!!"};
        LveDevice lve_device {lve_window};
        LveRenderer lve_renderer{lve_window, lve_device};

        std::vector<LveGameObject> field_objects;
        std::vector<LveGameObject> sphere_objects;
    };
}