#pragma once

#include "scene/LveGameObject.hpp"
#include "core/LveWindow.hpp"

namespace lve {
    class KeyboardMovementController {
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

        void moveInPlaneXZ(GLFWwindow* window, float dt, LveGameObject& game_object);

        KeyMappings keys{};
        float move_speed{3.f};
        float look_speed{1.5f};
    };
}