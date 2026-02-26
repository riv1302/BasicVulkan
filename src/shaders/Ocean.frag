#version 450

layout(location = 0) in vec3 frag_world_pos;
layout(location = 1) in vec3 frag_normal;

layout(location = 0) out vec4 out_color;

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

void main() {
    vec3 N = normalize(frag_normal);
    vec3 V = normalize(ubo.camera_pos.xyz - frag_world_pos);
    vec3 L = normalize(-ubo.sun_direction.xyz); // direction toward sun

    // Water base color
    vec3 water_color = vec3(0.01, 0.03, 0.05);

    // Simple diffuse
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = water_color * ubo.sun_color.xyz * ubo.sun_color.w * NdotL;

    // Ambient
    vec3 ambient = water_color * 0.15;

    // Basic Fresnel (Schlick) for slight sky tint
    float NdotV = max(dot(N, V), 0.0);
    float fresnel = 0.02 + 0.98 * pow(1.0 - NdotV, 5.0);
    vec3 sky_tint = vec3(0.3, 0.4, 0.5); // approximate sky color
    vec3 surface = mix(ambient + diffuse, sky_tint, fresnel);

    // Sun specular (Blinn-Phong)
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 256.0);
    vec3 specular = ubo.sun_color.xyz * ubo.sun_color.w * spec * 0.5;

    vec3 color = surface + specular;

    // Simple tone mapping
    color = color / (color + 1.0);

    out_color = vec4(color, 1.0);
}
