#include "FirstApp.hpp"

#include "KeyboardMovementController.h"
#include "LveCamera.hpp"
#include "systems/SimpleRenderSystem.hpp"
#include "systems/PointLightSystem.hpp"
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

    FirstApp::FirstApp()
    {
        global_pool = LveDescriptorPool::Builder(lve_device)
            .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
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

        auto global_set_layout = LveDescriptorSetLayout::Builder(lve_device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

        std::vector<VkDescriptorSet> global_descriptor_sets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < global_descriptor_sets.size(); ++i) {
            auto buffer_info = ubo_buffers[i]->descriptorInfo();
            LveDescriptorWriter(*global_set_layout, *global_pool)
                .writeBuffer(0, &buffer_info)
                .build(global_descriptor_sets[i]);
        }


        SimpleRenderSystem simple_render_system{
            lve_device,
            lve_renderer.getSwapChainRenderPass(),
            global_set_layout->getDescriptorSetLayout()
        };
        PointLightSystem point_light_system{
            lve_device,
            lve_renderer.getSwapChainRenderPass(),
            global_set_layout->getDescriptorSetLayout()
        };
        LveCamera camera{};
        camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

        auto viewer_object = LveGameObject::createGameObject();
        viewer_object.transform.translation.z = -2.5f;
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
                    camera,
                    global_descriptor_sets[frame_index],
                    game_objects
                };
                // Update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                point_light_system.update(frame_info, ubo);
                ubo_buffers[frame_index]->writeToBuffer(&ubo);
                ubo_buffers[frame_index]->flush();

                // Render
                lve_renderer.beginSwapChainRenderPass(command_buffer);
                simple_render_system.renderGameObjects(frame_info);
                point_light_system.render(frame_info);
                lve_renderer.endSwapChainRenderPass(command_buffer);
                lve_renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(lve_device.device());
    }

    void FirstApp::loadGameObjects() {
        auto flat_vase = LveGameObject::createGameObject();
        flat_vase.model = LveModel::createModelFromFile(lve_device, "../Models/flat_vase.obj");
        flat_vase.transform.translation = {-.5f, .5f, 0.f};
        flat_vase.transform.scale = glm::vec3{3.f};
        game_objects.emplace(flat_vase.getId(), std::move(flat_vase));
        
        auto smooth_vase = LveGameObject::createGameObject();
        smooth_vase.model = LveModel::createModelFromFile(lve_device, "../Models/smooth_vase.obj");
        smooth_vase.transform.translation = {.5f, .5f, 0.f};
        smooth_vase.transform.scale = glm::vec3{3.f};
        game_objects.emplace(smooth_vase.getId(), std::move(smooth_vase));

        auto floor = LveGameObject::createGameObject();
        floor.model = LveModel::createModelFromFile(lve_device, "../Models/quad.obj");
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
                (i*glm::two_pi<float>()) / light_colors.size(),
                {0.f, -1.f, 0.f}
            );
            point_light.transform.translation = glm::vec3(rotate_light * glm::vec4(-1.f, -1.f, 1.f, 1.f));
            game_objects.emplace(point_light.getId(), std::move(point_light));
        }
        
    }
}