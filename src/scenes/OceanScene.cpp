#include "scenes/OceanScene.hpp"
#include "app/Engine.hpp"
#include "input/FreeCameraController.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace lve {

    void OceanScene::init(Engine& engine) {
        sky_render_system = std::make_unique<SkyRenderSystem>(
            engine.getDevice(),
            engine.getRenderPass(),
            engine.getGlobalDescriptorSetLayout()
        );

        camera_controller = std::make_unique<FreeCameraController>(
            glm::vec3{0.f, -2.f, 0.f}
        );
    }

    void OceanScene::update(FrameInfo& frame_info, GlobalUbo& ubo) {
    }

    void OceanScene::render(FrameInfo& frame_info) {
        sky_render_system->render(frame_info);
    }

    void OceanScene::cleanup() {
        sky_render_system.reset();
        Scene::cleanup();
    }
}
