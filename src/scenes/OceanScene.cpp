#include "scenes/OceanScene.hpp"
#include "app/Engine.hpp"
#include "input/FreeCameraController.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

namespace lve {

    void OceanScene::init(Engine& engine) {
        sky_render_system = std::make_unique<SkyRenderSystem>(
            engine.getDevice(),
            engine.getRenderPass(),
            engine.getGlobalDescriptorSetLayout()
        );

        ocean_render_system = std::make_unique<OceanRenderSystem>(
            engine.getDevice(),
            engine.getRenderPass(),
            engine.getGlobalDescriptorSetLayout()
        );

        camera_controller = std::make_unique<FreeCameraController>(
            glm::vec3{0.f, -2.f, 0.f}
        );
    }

    void OceanScene::handleInput(GLFWwindow* window, float dt) {
        Scene::handleInput(window, dt);

        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            time_speed *= (1.0f + 2.0f * dt);
        }
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
            time_speed *= (1.0f - 2.0f * dt);
            time_speed = glm::max(time_speed, 0.001f);
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            time_speed = 0.01f;
        }

        bool pause_key_down = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
        if (pause_key_down && !pause_key_was_down) {
            time_paused = !time_paused;
        }
        pause_key_was_down = pause_key_down;
    }

    void OceanScene::update(FrameInfo& frame_info, GlobalUbo& ubo) {
        // Advance time of day
        if (!time_paused) {
            time_of_day += frame_info.frame_time * time_speed;
            time_of_day = std::fmod(time_of_day, 24.0f);
            if (time_of_day < 0.0f) time_of_day += 24.0f;
        }
        ubo.time_of_day = time_of_day;

        // Sun direction from time of day
        // t=6 sunrise (horizon), t=12 noon (overhead), t=18 sunset, t=0 midnight (below)
        float sun_angle = ((time_of_day - 6.0f) / 24.0f) * glm::two_pi<float>();
        glm::vec3 sun_dir = glm::normalize(glm::vec3(
            std::cos(sun_angle),
            -std::sin(sun_angle),   // negate for Y-down (negative Y = up)
            0.3f
        ));
        ubo.sun_direction = glm::vec4(sun_dir, 0.0f);

        // Sun color based on height above horizon
        float sun_height = -sun_dir.y;  // positive = above horizon in Y-down

        glm::vec3 sun_color;
        float sun_intensity;
        if (sun_height > 0.3f) {
            sun_color = glm::vec3(1.0f, 0.95f, 0.85f);
            sun_intensity = 1.0f;
        } else if (sun_height > 0.0f) {
            float t = sun_height / 0.3f;
            sun_color = glm::mix(glm::vec3(1.5f, 0.7f, 0.3f), glm::vec3(1.0f, 0.95f, 0.85f), t);
            sun_intensity = glm::mix(0.8f, 1.0f, t);
        } else {
            float t = glm::clamp(sun_height / -0.2f, 0.0f, 1.0f);
            sun_color = glm::vec3(1.5f, 0.7f, 0.3f);
            sun_intensity = glm::mix(0.8f, 0.0f, t);
        }
        ubo.sun_color = glm::vec4(sun_color, sun_intensity);

        // Wave 0: dominant wave for Phase 4
        constexpr float wavelength = 60.0f;
        constexpr float amplitude = 1.5f;
        constexpr float steepness = 0.5f;
        float frequency = glm::two_pi<float>() / wavelength;
        float phase_speed = std::sqrt(9.8f * wavelength / glm::two_pi<float>()) * frequency;

        ubo.waves[0].direction = glm::vec2(1.0f, 0.0f);
        ubo.waves[0].amplitude = amplitude;
        ubo.waves[0].frequency = frequency;
        ubo.waves[0].phase = phase_speed;
        ubo.waves[0].steepness = steepness;
    }

    void OceanScene::render(FrameInfo& frame_info) {
        sky_render_system->render(frame_info);
        ocean_render_system->render(frame_info);
    }

    void OceanScene::cleanup() {
        ocean_render_system.reset();
        sky_render_system.reset();
        Scene::cleanup();
    }
}
