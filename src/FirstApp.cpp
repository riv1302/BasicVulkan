#include "FirstApp.hpp"

namespace lve {
    void FirstApp::run() {
        while (!lve_window.shouldClose()) {
            glfwPollEvents();
        }        
    }
}