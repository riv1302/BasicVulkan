#version 450

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

struct PointLight {
    vec4 position;
    vec4 color;
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
} ubo;

vec3 getRayDirection(vec2 uv) {
    vec2 ndc = uv * 2.0 - 1.0;
    vec4 world_pos = ubo.inv_view_proj * vec4(ndc, 1.0, 1.0);
    world_pos /= world_pos.w;
    return normalize(world_pos.xyz - ubo.camera_pos.xyz);
}

void main() {
    vec3 ray_dir = getRayDirection(in_uv);

    // Elevation: negate Y because engine uses Y-down (negative Y = up)
    float elevation = -ray_dir.y;

    // Atmospheric gradient
    vec3 zenith_color  = vec3(0.15, 0.3, 0.65);
    vec3 horizon_color = vec3(0.6, 0.75, 0.9);
    vec3 ground_color  = vec3(0.25, 0.25, 0.22);

    float sky_t = smoothstep(0.0, 0.8, elevation);
    vec3 sky_color = mix(horizon_color, zenith_color, sky_t);

    // Below horizon fade
    float ground_t = smoothstep(0.0, -0.15, elevation);
    sky_color = mix(sky_color, ground_color, ground_t);

    // Sun
    vec3 sun_dir = normalize(ubo.sun_direction.xyz);
    float sun_dot = dot(ray_dir, sun_dir);

    float sun_disc = smoothstep(0.9995, 0.9999, sun_dot);
    float sun_halo = pow(max(sun_dot, 0.0), 256.0) * 0.6;
    float sun_glow = pow(max(sun_dot, 0.0), 32.0) * 0.15;

    vec3 sun_rgb = ubo.sun_color.xyz * ubo.sun_color.w;

    vec3 final_color = sky_color;
    final_color += sun_rgb * (sun_disc + sun_halo + sun_glow);

    // Reinhard tone mapping
    final_color = final_color / (final_color + vec3(1.0));

    out_color = vec4(final_color, 1.0);
}
