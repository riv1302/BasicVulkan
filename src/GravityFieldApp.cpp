#include "GravityFieldApp.hpp"

#include "FieldRenderSystem.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <cassert>
#include <array>

#include <iostream>

namespace lve {
    GravityFieldApp::GravityFieldApp()
    {
        loadGameObjects();
    }

    GravityFieldApp::~GravityFieldApp() {}

    void GravityFieldApp::run() {
        FieldRenderSystem field_render_system{lve_device, lve_renderer.getSwapChainRenderPass()};
        while (!lve_window.shouldClose()) {
            glfwPollEvents();
            
            if (auto command_buffer = lve_renderer.beginFrame()) {
                lve_renderer.beginSwapChainRenderPass(command_buffer);
                field_render_system.renderGameObjects(command_buffer, field_objects);
                lve_renderer.endSwapChainRenderPass(command_buffer);
                lve_renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(lve_device.device());
    }

    void GravityFieldApp::calc_next_frame() {

    }

    void GravityFieldApp::loadGameObjects() {
        
        const float width = 1.0f/(DENSITY*10.0f);
        const float height = 1.0f/(DENSITY * 3.0f);
        const float dist = 2.0f/DENSITY;
        for (int i = 0; i < DENSITY; ++i) {
            for (int j = 0; j < DENSITY; ++j) {
                glm::vec2 pos{i*dist + dist/2.0f - 1.0f, j*dist + dist/2.0f - 1.0f};
                std::vector<LveModel::Vertex> vertices {
                    {pos + glm::vec2{-width, -height}, {1.0, 0.0, 0.0}},
                    {pos + glm::vec2{width, -height}, {0.0, 0.0, 1.0}},
                    {pos + glm::vec2{-width, height}, {0.0, 1.0, 0.0}},
                    
                    {pos + glm::vec2{width, -height}, {1.0, 0.0, 0.0}},
                    {pos + glm::vec2{width, height}, {0.0, 0.0, 1.0}},
                    {pos + glm::vec2{-width, height}, {0.0, 1.0, 0.0}}
                };
                
                auto lve_model = std::make_shared<LveModel>(lve_device, vertices);

                auto field_stick = LveGameObject::createGameObject();
                field_stick.model = lve_model;
                field_stick.color = {1, 0, 0};

                field_objects.push_back(std::move(field_stick));
            }
        }
    }
}