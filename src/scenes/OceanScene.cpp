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

        // 6 Gerstner waves (design doc table)
        struct WaveParam { glm::vec2 dir; float amp; float wavelength; float Q; };
        static const WaveParam wave_params[MAX_WAVES] = {
            {{1.0f, 0.0f},   1.5f,  60.f, 0.5f},
            {{0.7f, 0.7f},   1.0f,  40.f, 0.4f},
            {{-0.3f, 0.9f},  0.5f,  25.f, 0.3f},
            {{0.5f, -0.5f},  0.3f,  15.f, 0.6f},
            {{-0.8f, 0.2f},  0.15f,  8.f, 0.2f},
            {{0.1f, -0.9f},  0.08f,  4.f, 0.3f},
        };

        constexpr float g = 9.8f;
        constexpr float two_pi = glm::two_pi<float>();
        for (int i = 0; i < MAX_WAVES; i++) {
            const auto& wp = wave_params[i];
            float freq = two_pi / wp.wavelength;
            float phase_speed = std::sqrt(g * wp.wavelength / two_pi) * freq;
            ubo.waves[i].direction = glm::normalize(wp.dir);
            ubo.waves[i].amplitude = wp.amp;
            ubo.waves[i].frequency = freq;
            ubo.waves[i].phase = phase_speed;
            ubo.waves[i].steepness = wp.Q;
        }
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
