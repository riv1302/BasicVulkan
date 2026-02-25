#pragma once

#include "scene/LveGameObject.hpp"
#include "renderer/LveFrameInfo.hpp"

namespace lve {

    class Engine;

    class Scene {
    public:
        virtual ~Scene() = default;

        virtual void init(Engine& engine) = 0;
        virtual void update(FrameInfo& frame_info, GlobalUbo& ubo) = 0;
        virtual void render(FrameInfo& frame_info) = 0;
        virtual void cleanup() { game_objects.clear(); }

        void setEngine(Engine* eng) { engine = eng; }
        LveGameObject::Map& getGameObjects() { return game_objects; }

    protected:
        Engine* engine = nullptr;
        LveGameObject::Map game_objects;
    };
}