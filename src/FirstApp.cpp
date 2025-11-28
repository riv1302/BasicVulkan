#include "FirstApp.hpp"

#include "KeyboardMovementController.h"
#include "LveCamera.hpp"
#include "SimpleRenderSystem.hpp"
#include "LveBuffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <cassert>
#include <array>
#include <chrono>
#include <numeric>

namespace lve {
    struct GlobalUbo {
        glm::mat4 projection_view{1.f};
        glm::vec3 light_direction = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
    };

    FirstApp::FirstApp()
    {
        loadGameObjects();
    }

    FirstApp::~FirstApp() {}

    void FirstApp::run() {
        std::vector<std::unique_ptr<LveBuffer>> ubo_buffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < ubo_buffers.size(); ++i) {
            ubo_buffers[i] = std::make_unique<LveBuffer>(
                lve_device,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );
            ubo_buffers[i]->map();
        }

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
                int frame_index = lve_renderer.getFrameIndex();
                FrameInfo frame_info {
                    frame_index,
                    frame_time,
                    command_buffer,
                    camera
                };
                // Update
                GlobalUbo ubo{};
                ubo.projection_view = camera.getProjection() * camera.getView();
                ubo_buffers[frame_index]->writeToBuffer(&ubo);
                ubo_buffers[frame_index]->flush();

                // Render
                lve_renderer.beginSwapChainRenderPass(command_buffer);
                simple_render_system.renderGameObjects(frame_info, game_objects);
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