#pragma once

struct GLFWwindow;

namespace lve {

    class LveCamera;

    class CameraController {
    public:
        virtual ~CameraController() = default;
        virtual void update(GLFWwindow* window, float dt, LveCamera& camera) = 0;
    };
}