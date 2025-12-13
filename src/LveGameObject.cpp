#include "LveGameObject.hpp"

namespace lve{
    LveGameObject LveGameObject::makePointLight(
        float intensity,
        float radius,
        glm::vec3 color
    ) {
        LveGameObject game_obj = LveGameObject::createGameObject();
        game_obj.color = color;
        game_obj.transform.scale.x = radius;
        game_obj.point_light = std::make_unique<PointLightComponent>();
        game_obj.point_light->light_intensity = intensity;
        return game_obj;
    }
}