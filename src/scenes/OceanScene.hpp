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

    private:
        std::unique_ptr<SkyRenderSystem> sky_render_system;
    };
}
