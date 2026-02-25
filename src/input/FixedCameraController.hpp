#pragma once

#include "input/CameraController.hpp"

#include <glm/glm.hpp>

namespace lve {

    class FixedCameraController : public CameraController {
    public:
        FixedCameraController(glm::vec3 position, glm::vec3 target);

        void update(GLFWwindow* window, float dt, LveCamera& camera) override;

    private:
        glm::vec3 position;
        glm::vec3 target;
        bool initialized = false;
    };
}