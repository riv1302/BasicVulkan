# BasicVulkan

A lightweight Vulkan graphics engine built for learning and experimenting with GPU programming. The engine supports multiple scenes with runtime switching, custom render systems per scene, and OBJ model loading.
<img width="960" height="519" alt="{20E838D2-19ED-4D2D-A219-EEAD0DE143C8}" src="https://github.com/user-attachments/assets/a1c7c329-1346-435d-b359-a0289492dec3" />

## Features

- Scene-based architecture with runtime scene switching
- Configurable render systems per scene (custom shaders/pipelines)
- OBJ model loading with indexed drawing
- Point light system with billboard rendering
- Camera with keyboard controls (WASD + arrow keys)
- Descriptor set management with builder pattern
- Double-buffered rendering with swap chain recreation on resize

## Requirements

- C++20 compiler
- [Vulkan SDK](https://vulkan.lunarg.com/) (includes `glslc` shader compiler)
- [GLFW](https://www.glfw.org/)
- [GLM](https://github.com/g-truc/glm)
- CMake 3.15+

## Building

```bash
git clone <repository-url>
cd BasicVulkan
mkdir build && cd build
cmake ..
make -j$(nproc)
```

The shaders are compiled automatically during the build process.

## Running

```bash
cd build
./VulkanProject
```

**Controls:**
- `WASD` — Move camera
- `Arrow keys` — Look around
- `Shift` / `Ctrl` — Move up / down

## Project Structure

```
src/
├── main.cpp              Entry point
├── app/                  Engine core (game loop, resource management)
├── core/                 Vulkan infrastructure (device, window, swap chain, buffers)
├── renderer/             Pipeline, descriptors, renderer, frame info
├── scene/                Game objects, models, camera, base Scene class
├── scenes/               Concrete scene implementations
├── systems/              Render systems (SimpleRender, PointLight)
├── input/                Input controllers
└── shaders/              GLSL vertex/fragment shaders
```

## Creating a New Scene

Inherit from `Scene` and implement the virtual methods:

```cpp
#include "scene/Scene.hpp"

class MyScene : public Scene {
public:
    void init(Engine& engine) override {
        // Create render systems and load game objects
    }
    void update(FrameInfo& frame_info, GlobalUbo& ubo) override {
        // Update logic per frame
    }
    void render(FrameInfo& frame_info) override {
        // Issue draw calls
    }
    void cleanup() override {
        // Release resources
    }
};
```

Then set it in `main.cpp`:

```cpp
Engine engine(800, 600, "My App");
engine.switchScene(std::make_unique<MyScene>());
engine.run();
```

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
