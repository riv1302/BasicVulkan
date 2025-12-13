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
layout(location = 1) out vec3 frag_pos_world;
layout(location = 2) out vec3 frag_normal_world;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection_matrix;
    mat4 view_matrix;
    vec4 ambient_light_color;
    vec3 light_position;
    vec4 light_color;
} ubo;

layout(push_constant) uniform Push {
    mat4 model_Matrix;
    mat4 normal_matrix;
} push;

void main() {
    vec4 position_world = push.model_Matrix * vec4(position, 1.0);
    gl_Position = ubo.projection_matrix * ubo.view_matrix * position_world;

    frag_normal_world = normalize(mat3(push.normal_matrix) * normal);
    frag_pos_world = position_world.xyz;
    frag_color = color;
}