#pragma once

#include "input/CameraController.hpp"
#include "core/LveWindow.hpp"

#include <glm/glm.hpp>

namespace lve {

    class FreeCameraController : public CameraController {
    public:
        struct KeyMappings {
            int move_left = GLFW_KEY_A;
            int move_right = GLFW_KEY_D;
            int move_forward = GLFW_KEY_W;
            int move_backward = GLFW_KEY_S;
            int move_up = GLFW_KEY_LEFT_SHIFT;
            int move_down = GLFW_KEY_LEFT_CONTROL;
            int look_left = GLFW_KEY_LEFT;
            int look_right = GLFW_KEY_RIGHT;
            int look_up = GLFW_KEY_UP;
            int look_down = GLFW_KEY_DOWN;
        };

        FreeCameraController(glm::vec3 initial_position = {0.f, 0.f, -2.5f});

        void update(GLFWwindow* window, float dt, LveCamera& camera) override;

        KeyMappings keys{};
        float move_speed{3.f};
        float look_speed{1.5f};

    private:
        glm::vec3 position;
        glm::vec3 rotation{0.f};
    };
}