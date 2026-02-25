#include "scenes/VaseScene.hpp"
#include "app/Engine.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lve {

    void VaseScene::init(Engine& engine) {
        simple_render_system = std::make_unique<SimpleRenderSystem>(
            engine.getDevice(),
            engine.getRenderPass(),
            engine.getGlobalDescriptorSetLayout()
        );
        point_light_system = std::make_unique<PointLightSystem>(
            engine.getDevice(),
            engine.getRenderPass(),
            engine.getGlobalDescriptorSetLayout()
        );

        loadGameObjects(engine.getDevice());
    }

    void VaseScene::loadGameObjects(LveDevice& device) {
        auto flat_vase = LveGameObject::createGameObject();
        flat_vase.model = LveModel::createModelFromFile(device, "../Models/flat_vase.obj");
        flat_vase.transform.translation = {-.5f, .5f, 0.f};
        flat_vase.transform.scale = glm::vec3{3.f};
        game_objects.emplace(flat_vase.getId(), std::move(flat_vase));

        auto smooth_vase = LveGameObject::createGameObject();
        smooth_vase.model = LveModel::createModelFromFile(device, "../Models/smooth_vase.obj");
        smooth_vase.transform.translation = {.5f, .5f, 0.f};
        smooth_vase.transform.scale = glm::vec3{3.f};
        game_objects.emplace(smooth_vase.getId(), std::move(smooth_vase));

        auto floor = LveGameObject::createGameObject();
        floor.model = LveModel::createModelFromFile(device, "../Models/quad.obj");
        floor.transform.translation = {0.f, .5f, 0.f};
        floor.transform.scale = glm::vec3{3.f, 1.f, 3.f};
        game_objects.emplace(floor.getId(), std::move(floor));

        std::vector<glm::vec3> light_colors{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f},
        };
        for (int i = 0; i < light_colors.size(); ++i) {
            auto point_light = LveGameObject::makePointLight(0.2f);
            point_light.color = light_colors[i];
            auto rotate_light = glm::rotate(
                glm::mat4(1.f),
                (i * glm::two_pi<float>()) / light_colors.size(),
                {0.f, -1.f, 0.f}
            );
            point_light.transform.translation = glm::vec3(rotate_light * glm::vec4(-1.f, -1.f, 1.f, 1.f));
            game_objects.emplace(point_light.getId(), std::move(point_light));
        }
    }

    void VaseScene::update(FrameInfo& frame_info, GlobalUbo& ubo) {
        point_light_system->update(frame_info, ubo);
    }

    void VaseScene::render(FrameInfo& frame_info) {
        simple_render_system->renderGameObjects(frame_info);
        point_light_system->render(frame_info);
    }

    void VaseScene::cleanup() {
        simple_render_system.reset();
        point_light_system.reset();
        Scene::cleanup();
    }
}