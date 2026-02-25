#include "app/Engine.hpp"
#include "scenes/OceanScene.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
    lve::Engine engine(800, 600, "Vulkan Engine");
    engine.switchScene(std::make_unique<lve::OceanScene>());

    try {
        engine.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}