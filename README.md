# BasicVulkan

A small Vulkan renderer written in C++20, built to learn GPU programming. The main scene is an ocean under a dynamic sky with a full day/night cycle. The waves, sky, clouds, stars and moon are all generated in shaders, without textures.

<img width="960" height="519" alt="Ocean scene" src="https://github.com/user-attachments/assets/a1c7c329-1346-435d-b359-a0289492dec3" />

The Vulkan foundation was built by following Brendan Galea's tutorial series. The scene architecture, the sky and the ocean are my own work (see [Credits](#credits)).

## Ocean scene

### Ocean
- 256 × 256 grid that moves with the camera in whole-cell steps, so you never reach its edge while the waves stay fixed in world space
- Six Gerstner waves summed in the vertex shader, each with its speed set by the deep-water dispersion relation (ω = √(g·k)), and normals computed analytically from the wave derivatives
- Schlick Fresnel blend between the deep-water colour and the reflected sky
- Blinn-Phong specular and diffuse lighting from the sun
- Foam on the wave crests: a height mask combined with a multi-scale Voronoi (F2 − F1) bubble pattern, broken up with noise
- Exponential distance fog that blends the ocean into the sky at the horizon

### Sky
- Full-screen triangle generated from `gl_VertexIndex` (no vertex buffer); view rays are reconstructed with the inverse view-projection matrix
- Sky gradient driven by the sun's height (day, sunset, twilight, night), with sun disc, halo and a sunset glow along the horizon
- Procedural twinkling star field and a moon opposite the sun with Voronoi craters
- Clouds: each view ray is intersected with a cloud plane and sampled with FBM noise, with wind drift, slow shape morphing and single-sample Beer's-law lighting towards the sun

### Day/night cycle
The CPU advances a 24-hour clock and derives the sun's direction, colour and intensity and the ambient light each frame. All scene data reaches the shaders through one uniform buffer, and both shaders finish with Reinhard tone mapping.

## Engine
- Scene-based architecture: each scene owns its render systems, game objects and camera controller, and `Engine::switchScene()` swaps scenes between frames
- Render systems with their own pipelines and shaders: lit models, point-light billboards, sky and ocean
- Interchangeable camera controllers (free, orbital, fixed) behind a common interface
- OBJ model loading with indexed drawing
- One uniform buffer and descriptor set per frame in flight
- Swap chain recreation when the window is resized
- `VaseScene`: the lighting demo from the tutorial (OBJ models and coloured point lights)

## Requirements
- C++20 compiler and CMake 3.15+
- [Vulkan SDK](https://vulkan.lunarg.com/), with `glslc` on your `PATH`
- [GLM](https://github.com/g-truc/glm) on the include path (installable with the Vulkan SDK on Windows, or `libglm-dev` on Debian/Ubuntu)
- [GLFW](https://www.glfw.org/) 3.4, which CMake downloads automatically if it is not installed

## Building
```bash
git clone https://github.com/riv1302/BasicVulkan.git
cd BasicVulkan
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```
Shaders are compiled to SPIR-V during the build and copied next to the executable. Debug builds enable the Khronos validation layer.

## Running
Run the executable from its own folder, since shaders are loaded from `./shaders`:
```bash
cd build
./VulkanProject
```
With multi-config generators such as Visual Studio, build with `cmake --build build --config Release` and run from `build/Release`.

| Key | Action |
|-----|--------|
| `W` `A` `S` `D` | Move |
| Arrow keys | Look around |
| `Left Shift` / `Left Ctrl` | Move up / down |
| `T` / `G` | Speed up / slow down time |
| `R` | Reset time speed |
| `P` | Pause / resume the day/night cycle |

## Project structure
```
Models/              OBJ models (used by VaseScene)
libs/TinyObjLoader/  tinyobjloader (single header)
src/
├── main.cpp         Entry point (starts the ocean scene)
├── app/             Engine: main loop, global uniform buffer, scene switching
├── core/            Vulkan device, window, swap chain, buffers
├── renderer/        Pipelines, descriptors, renderer, frame info and UBO layout
├── scene/           Game objects, models, camera, base Scene class
├── scenes/          OceanScene, VaseScene
├── systems/         Render systems: Simple, PointLight, Sky, Ocean
├── input/           Camera controllers: Free, Orbital, Fixed
└── shaders/         GLSL shaders
```

## Creating a new scene
Inherit from `Scene` and implement its virtual methods:
```cpp
#include "scene/Scene.hpp"

class MyScene : public lve::Scene {
public:
    void init(lve::Engine& engine) override {
        // Create render systems and load game objects
    }
    void update(lve::FrameInfo& frame_info, lve::GlobalUbo& ubo) override {
        // Per-frame logic; write scene data into the UBO
    }
    void render(lve::FrameInfo& frame_info) override {
        // Record draw calls
    }
    void cleanup() override {
        // Release the scene's render systems, then the game objects
        Scene::cleanup();
    }
};
```
Then start it from `main.cpp`:
```cpp
lve::Engine engine(800, 600, "My App");
engine.switchScene(std::make_unique<MyScene>());
engine.run();
```

## Credits
- The Vulkan foundation (device and swap chain setup, pipelines, buffers, descriptor sets, model loading, camera and the point-light system) was built by following Brendan Galea's Vulkan game engine tutorials, [littleVulkanEngine](https://github.com/blurrypiano/littleVulkanEngine) (MIT License).
- My own work on top of it: the split into `Engine` and `Scene`, the folder structure, the camera controller interface, and the sky and ocean (render systems and shaders).
- OBJ loading uses [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) (MIT License).

## License
MIT. See [LICENSE](LICENSE), which also includes the copyright notice of littleVulkanEngine.
