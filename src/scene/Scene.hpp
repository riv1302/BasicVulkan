#pragma once

#include "scene/LveGameObject.hpp"
#include "scene/LveCamera.hpp"
#include "renderer/LveFrameInfo.hpp"
#include "input/CameraController.hpp"

#include <glm/glm.hpp>
#include <memory>

struct GLFWwindow;

namespace lve {

    class Engine;

    class Scene {
    public:
        virtual ~Scene() = default;

        virtual void init(Engine& engine) = 0;
        virtual void update(FrameInfo& frame_info, GlobalUbo& ubo) = 0;
        virtual void render(FrameInfo& frame_info) = 0;
        virtual void cleanup() { game_objects.clear(); }

        virtual void handleInput(GLFWwindow* window, float dt) {
            if (camera_controller) {
                camera_controller->update(window, dt, camera);
            }
        }

        void updateProjection(float aspect) {
            camera.setPerspectiveProjection(fov, aspect, near_plane, far_plane);
        }

        LveCamera& getCamera() { return camera; }
        void setEngine(Engine* eng) { engine = eng; }
        LveGameObject::Map& getGameObjects() { return game_objects; }

    protected:
        Engine* engine = nullptr;
        LveGameObject::Map game_objects;
        LveCamera camera;
        std::unique_ptr<CameraController> camera_controller;

        float fov{glm::radians(50.f)};
        float near_plane{0.1f};
        float far_plane{100.f};
    };
}