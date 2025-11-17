#include "FirstApp.hpp"

#include "KeyboardMovementController.h"
#include "LveCamera.hpp"
#include "SimpleRenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <cassert>
#include <array>
#include <chrono>

namespace lve {
    FirstApp::FirstApp()
    {
        loadGameObjects();
    }

    FirstApp::~FirstApp() {}

    void FirstApp::run() {
        SimpleRenderSystem simple_render_system{lve_device, lve_renderer.getSwapChainRenderPass()};
        LveCamera camera{};
        camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

        auto viewer_object = LveGameObject::createGameObject();
        KeyboardMovementController camera_controller{};

        auto current_time = std::chrono::steady_clock::now();

        while (!lve_window.shouldClose()) {
            glfwPollEvents();

            auto new_time = std::chrono::steady_clock::now();
            float frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).count();
            current_time = new_time;

            camera_controller.moveInPlaneXZ(lve_window.getGLFWwindow(), frame_time, viewer_object);
            camera.setViewYXZ(viewer_object.transform.translation, viewer_object.transform.rotation);

            float aspect = lve_renderer.getAspectRation();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
            
            if (auto command_buffer = lve_renderer.beginFrame()) {
                lve_renderer.beginSwapChainRenderPass(command_buffer);
                simple_render_system.renderGameObjects(command_buffer, game_objects, camera);
                lve_renderer.endSwapChainRenderPass(command_buffer);
                lve_renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(lve_device.device());
    }

    void FirstApp::loadGameObjects() {
        std::shared_ptr<LveModel> lve_model = LveModel::createModelFromFile(lve_device, "../Models/flat_vase.obj");

        auto game_object = LveGameObject::createGameObject();
        game_object.model = lve_model;
        game_object.transform.translation = {.0f, .5f, 2.5f};
        game_object.transform.scale = glm::vec3{3.f};
        game_objects.push_back(std::move(game_object));
    }
}