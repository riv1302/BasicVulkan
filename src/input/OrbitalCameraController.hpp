#pragma once

#include "input/CameraController.hpp"
#include "core/LveWindow.hpp"

#include <glm/glm.hpp>

namespace lve {

    class OrbitalCameraController : public CameraController {
    public:
        OrbitalCameraController(glm::vec3 target = {0.f, 0.f, 0.f}, float distance = 5.f);

        void update(GLFWwindow* window, float dt, LveCamera& camera) override;

        float orbit_speed{1.5f};
        float zoom_speed{2.f};
        float min_distance{1.f};
        float max_distance{20.f};

    private:
        glm::vec3 target;
        float distance;
        float yaw{0.f};
        float pitch{-0.5f};
    };
}