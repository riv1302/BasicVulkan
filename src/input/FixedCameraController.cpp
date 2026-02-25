#include "input/FixedCameraController.hpp"
#include "scene/LveCamera.hpp"

namespace lve {

    FixedCameraController::FixedCameraController(glm::vec3 position, glm::vec3 target)
        : position{position}, target{target} {}

    void FixedCameraController::update(GLFWwindow* window, float dt, LveCamera& camera) {
        if (!initialized) {
            camera.setViewTarget(position, target);
            initialized = true;
        }
    }
}