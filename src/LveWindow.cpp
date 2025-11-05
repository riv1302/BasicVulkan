#include "LveWindow.hpp"

#include <stdexcept>
namespace lve {
    LveWindow::LveWindow(int w, int h, std::string name):
        width(w),
        height(h),
        window_name(name)
    {
        initWindow();
    }

    LveWindow::~LveWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void LveWindow::frameBufferResizedCallback(
        GLFWwindow *window,
        int width,
        int height
    ) {
        auto lve_window = reinterpret_cast<LveWindow *>(glfwGetWindowUserPointer(window));
        lve_window->frame_buffer_resized = true;
        lve_window->width = width;
        lve_window->height = height;
    }

    void LveWindow::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, frameBufferResizedCallback);
    }

    void LveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
            throw std::runtime_error("failed to create window surface");
    }
}