#include "app/Engine.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <cassert>
#include <chrono>

namespace lve {

    Engine::Engine(int width, int height, const std::string& title)
        : lve_window(width, height, title)
    {
        initGlobalResources();
    }

    Engine::~Engine() {
        if (current_scene) {
            current_scene->cleanup();
        }
        vkDeviceWaitIdle(lve_device.device());
    }

    void Engine::initGlobalResources() {
        global_pool = LveDescriptorPool::Builder(lve_device)
            .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();

        ubo_buffers.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
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

        global_set_layout = LveDescriptorSetLayout::Builder(lve_device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

        global_descriptor_sets.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < global_descriptor_sets.size(); ++i) {
            auto buffer_info = ubo_buffers[i]->descriptorInfo();
            LveDescriptorWriter(*global_set_layout, *global_pool)
                .writeBuffer(0, &buffer_info)
                .build(global_descriptor_sets[i]);
        }

    }

    void Engine::switchScene(std::unique_ptr<Scene> new_scene) {
        pending_scene = std::move(new_scene);
    }

    void Engine::handleSceneSwitch() {
        if (!pending_scene) return;

        if (current_scene) {
            current_scene->cleanup();
            current_scene.reset();
        }
        vkDeviceWaitIdle(lve_device.device());

        current_scene = std::move(pending_scene);
        current_scene->setEngine(this);
        current_scene->init(*this);
    }

    void Engine::run() {
        auto current_time = std::chrono::steady_clock::now();

        while (!lve_window.shouldClose()) {
            glfwPollEvents();
            handleSceneSwitch();

            if (!current_scene) continue;

            auto new_time = std::chrono::steady_clock::now();
            float frame_time = std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).count();
            current_time = new_time;

            current_scene->handleInput(lve_window.getGLFWwindow(), frame_time);

            float aspect = lve_renderer.getAspectRatio();
            current_scene->updateProjection(aspect);

            LveCamera& camera = current_scene->getCamera();

            if (auto command_buffer = lve_renderer.beginFrame()) {
                int frame_index = lve_renderer.getFrameIndex();
                FrameInfo frame_info{
                    frame_index,
                    frame_time,
                    command_buffer,
                    camera,
                    global_descriptor_sets[frame_index],
                    current_scene->getGameObjects()
                };

                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();

                glm::mat4 view_proj = ubo.projection * ubo.view;
                ubo.inv_view_proj = glm::inverse(view_proj);
                glm::mat4 inv_view = glm::inverse(ubo.view);
                ubo.camera_pos = glm::vec4(glm::vec3(inv_view[3]), 0.f);

                elapsed_time_total += frame_time;
                ubo.elapsed_time = elapsed_time_total;

                current_scene->update(frame_info, ubo);
                ubo_buffers[frame_index]->writeToBuffer(&ubo);
                ubo_buffers[frame_index]->flush();

                lve_renderer.beginSwapChainRenderPass(command_buffer);
                current_scene->render(frame_info);
                lve_renderer.endSwapChainRenderPass(command_buffer);
                lve_renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(lve_device.device());
    }
}