#pragma once

#include "scene/Scene.hpp"
#include "systems/SimpleRenderSystem.hpp"
#include "systems/PointLightSystem.hpp"

#include <memory>

namespace lve {

    class VaseScene : public Scene {
    public:
        void init(Engine& engine) override;
        void update(FrameInfo& frame_info, GlobalUbo& ubo) override;
        void render(FrameInfo& frame_info) override;
        void cleanup() override;

    private:
        void loadGameObjects(LveDevice& device);

        std::unique_ptr<SimpleRenderSystem> simple_render_system;
        std::unique_ptr<PointLightSystem> point_light_system;
    };
}