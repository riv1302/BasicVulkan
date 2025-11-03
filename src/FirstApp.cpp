#include "FirstApp.hpp"

#include "SimpleRenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <cassert>
#include <array>

namespace lve {
    FirstApp::FirstApp()
    {
        loadGameObjects();
    }

    FirstApp::~FirstApp() {}

    void FirstApp::run() {
        SimpleRenderSystem simple_render_system{lve_device, lve_renderer.getSwapChainRenderPass()};
        while (!lve_window.shouldClose()) {
            glfwPollEvents();
            
            if (auto command_buffer = lve_renderer.beginFrame()) {
                lve_renderer.beginSwapChainRenderPass(command_buffer);
                simple_render_system.renderGameObjects(command_buffer, game_objects);
                lve_renderer.endSwapChainRenderPass(command_buffer);
                lve_renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(lve_device.device());
    }

    void FirstApp::loadGameObjects() {
        std::vector<LveModel::Vertex> vertices {
            {{0.0f, -0.5f}, {1.0, 0.0, 0.0}},
            {{0.5f, 0.5f}, {0.0, 0.0, 1.0}},
            {{-0.5f, 0.5f}, {0.0, 1.0, 0.0}}
        };

        auto lve_model = std::make_shared<LveModel>(lve_device, vertices);

        auto triangle = LveGameObject::createGameObject();
        triangle.model = lve_model;
        triangle.color = {.1f, .8f, .1f};
        triangle.transform2d.translation.x = .2f;
        triangle.transform2d.scale = {2.f, .5f};
        triangle.transform2d.rotation = .25f * glm::two_pi<float>();

        game_objects.push_back(std::move(triangle));
    }
}