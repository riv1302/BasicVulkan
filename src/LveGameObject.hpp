#pragma once

#include "LveModel.hpp"

#include <memory>

namespace lve {
    struct Transform2dComponent {
        glm::vec2 translation{};
        glm::vec2 scale{1.f, 1.f};
        float rotation;

        glm::mat2 mat2() {
            const float s = glm::sin(rotation);
            const float c = glm::cos(rotation);
            glm::mat2 rot_matrix{{c, s}, {-s, c}};
            glm::mat2 scale_matrix{
                {scale.x, .0f},
                {.0f, scale.y}
            };
            return rot_matrix * scale_matrix;
        }
    };

    class LveGameObject {
    public:
        using id_t = unsigned int;

        static LveGameObject createGameObject() {
            static id_t current_id = 0;
            return LveGameObject{current_id++};
        }

        LveGameObject(const LveGameObject&) = delete;
        LveGameObject &operator=(const LveGameObject&) = delete;
        LveGameObject(LveGameObject&&) = default;
        LveGameObject &operator=(LveGameObject&&) = default;

        inline id_t getId() {return id;}

        std::shared_ptr<LveModel> model{};
        glm::vec3 color{};
        Transform2dComponent transform2d{};

    private:
        LveGameObject(id_t obj_id) : id{obj_id} {};
        id_t id;
    };
}