#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 frag_world_pos;
layout(location = 1) out vec3 frag_normal;

struct PointLight {
    vec4 position;
    vec4 color;
};

struct Wave {
    vec2 direction;
    float amplitude;
    float frequency;
    float phase;
    float steepness;
    float _pad[2];
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection_matrix;
    mat4 view_matrix;
    vec4 ambient_light_color;
    PointLight point_lights[10];
    int num_lights;

    mat4 inv_view_proj;
    vec4 camera_pos;
    vec4 sun_direction;
    vec4 sun_color;
    float elapsed_time;
    float time_of_day;
    float fog_density;
    float time_speed;
    Wave waves[6];
} ubo;

const float GRID_SPACING = 1.0;

void main() {
    // Snap grid to camera position
    vec3 world_pos = position;
    world_pos.xz += floor(ubo.camera_pos.xz / GRID_SPACING) * GRID_SPACING;

    // Store original XZ for Gerstner evaluation
    vec2 p0 = world_pos.xz;

    // Apply all 6 Gerstner waves
    vec3 displaced = world_pos;
    vec3 T = vec3(1.0, 0.0, 0.0); // tangent
    vec3 B = vec3(0.0, 0.0, 1.0); // bitangent

    for (int i = 0; i < 6; i++) {
        Wave w = ubo.waves[i];
        if (w.amplitude <= 0.0) continue;

        float theta = w.frequency * dot(w.direction, p0) + w.phase * ubo.elapsed_time;
        float s = sin(theta);
        float c = cos(theta);

        float QA = w.steepness * w.amplitude;
        float WA = w.frequency * w.amplitude;

        // Displacement
        displaced.x -= QA * w.direction.x * c;
        displaced.z -= QA * w.direction.y * c;
        displaced.y += w.amplitude * s;

        // Analytical tangent/bitangent derivatives
        T.x -= w.steepness * w.direction.x * w.direction.x * WA * s;
        T.y += w.direction.x * WA * c;
        T.z -= w.steepness * w.direction.x * w.direction.y * WA * s;

        B.x -= w.steepness * w.direction.x * w.direction.y * WA * s;
        B.y += w.direction.y * WA * c;
        B.z -= w.steepness * w.direction.y * w.direction.y * WA * s;
    }

    // Normal = cross(bitangent, tangent) for Y-up, then negate Y for Y-down convention
    vec3 N = normalize(cross(B, T));
    N.y = -N.y; // flip for engine's Y-down convention

    frag_world_pos = displaced;
    frag_normal = N;

    gl_Position = ubo.projection_matrix * ubo.view_matrix * vec4(displaced, 1.0);
}
