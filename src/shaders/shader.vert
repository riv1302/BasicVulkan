// GLSL version
#version 450
/*
gl_Position
(-1, -1)        (1, -1)
     ______________
    |              |
    |       *      |
    |              |
    |  *        *  |
    |______________|
(-1, 1)            (1, 1)
*/

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 frag_color;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection_view_matrix;
    vec3 direction_to_light;
} ubo;

layout(push_constant) uniform Push {
    mat4 model_Matrix;
    mat4 normal_matrix;
} push;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0));
const float AMBIENT = 0;

void main() {
    gl_Position = ubo.projection_view_matrix * push.model_Matrix * vec4(position, 1.0);

    vec3 normal_world_space = normalize(mat3(push.normal_matrix) * normal);
    float light_intencity = AMBIENT + max(dot(normal_world_space, ubo.direction_to_light), 0);

    frag_color = light_intencity * color;
}