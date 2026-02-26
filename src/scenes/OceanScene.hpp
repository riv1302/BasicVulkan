#pragma once

#include "scene/Scene.hpp"
#include "systems/SkyRenderSystem.hpp"

#include <memory>

namespace lve {

    class OceanScene : public Scene {
    public:
        void init(Engine& engine) override;
        void update(FrameInfo& frame_info, GlobalUbo& ubo) override;
        void render(FrameInfo& frame_info) override;
        void cleanup() override;
        void handleInput(GLFWwindow* window, float dt) override;

    private:
        std::unique_ptr<SkyRenderSystem> sky_render_system;

        float time_of_day{6.0f};
        float time_speed{0.01f};
        bool time_paused{false};
        bool pause_key_was_down{false};
    };
}
