#include "input/OrbitalCameraController.hpp"
#include "scene/LveCamera.hpp"

#include <glm/gtc/constants.hpp>
#include <cmath>

namespace lve {

    OrbitalCameraController::OrbitalCameraController(glm::vec3 target, float distance)
        : target{target}, distance{distance} {}

    void OrbitalCameraController::update(GLFWwindow* window, float dt, LveCamera& camera) {
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) yaw += orbit_speed * dt;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) yaw -= orbit_speed * dt;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) pitch -= orbit_speed * dt;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) pitch += orbit_speed * dt;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) distance -= zoom_speed * dt;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) distance += zoom_speed * dt;

        pitch = glm::clamp(pitch, -glm::half_pi<float>() + 0.1f, glm::half_pi<float>() - 0.1f);
        yaw = glm::mod(yaw, glm::two_pi<float>());
        distance = glm::clamp(distance, min_distance, max_distance);

        glm::vec3 position;
        position.x = target.x + distance * std::cos(pitch) * std::sin(yaw);
        position.y = target.y + distance * std::sin(pitch);
        position.z = target.z + distance * std::cos(pitch) * std::cos(yaw);

        camera.setViewTarget(position, target);
    }
}