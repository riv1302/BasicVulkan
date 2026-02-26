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

// ─── Sky Gradient (duplicated from Sky.frag) ────────────────────────────────

vec3 getSkyColor(float elevation, float sun_height) {
    vec3 day_zenith    = vec3(0.15, 0.3, 0.65);
    vec3 day_horizon   = vec3(0.6, 0.75, 0.9);
    vec3 sunset_zenith  = vec3(0.15, 0.15, 0.35);
    vec3 sunset_horizon = vec3(0.9, 0.4, 0.15);
    vec3 twilight_zenith  = vec3(0.05, 0.05, 0.15);
    vec3 twilight_horizon = vec3(0.3, 0.15, 0.3);
    vec3 night_zenith  = vec3(0.01, 0.01, 0.03);
    vec3 night_horizon = vec3(0.02, 0.02, 0.05);

    vec3 zenith_color, horizon_color;

    if (sun_height > 0.3) {
        zenith_color  = day_zenith;
        horizon_color = day_horizon;
    } else if (sun_height > 0.0) {
        float t = sun_height / 0.3;
        zenith_color  = mix(sunset_zenith,  day_zenith,  t);
        horizon_color = mix(sunset_horizon, day_horizon, t);
    } else if (sun_height > -0.1) {
        float t = (sun_height + 0.1) / 0.1;
        zenith_color  = mix(twilight_zenith,  sunset_zenith,  t);
        horizon_color = mix(twilight_horizon, sunset_horizon, t);
    } else {
        float t = clamp((sun_height + 0.3) / 0.2, 0.0, 1.0);
        zenith_color  = mix(night_zenith,  twilight_zenith,  t);
        horizon_color = mix(night_horizon, twilight_horizon, t);
    }

    vec3 ground_color = vec3(0.02, 0.02, 0.02);

    float sky_t = smoothstep(0.0, 0.8, elevation);
    vec3 sky = mix(horizon_color, zenith_color, sky_t);

    float ground_t = smoothstep(0.0, -0.15, elevation);
    sky = mix(sky, ground_color, ground_t);

    return sky;
}

// ─── Evaluate Sky (gradient + sun, no clouds) ───────────────────────────────

vec3 evaluateSky(vec3 ray_dir) {
    vec3 sun_dir = normalize(ubo.sun_direction.xyz);
    float sun_height = -sun_dir.y;
    float elevation = -ray_dir.y;

    // Sky gradient
    vec3 sky_color = getSkyColor(elevation, sun_height);

    // Sun disc + halo + glow
    float sun_dot = dot(ray_dir, sun_dir);
    float sun_disc = smoothstep(0.9995, 0.9999, sun_dot);
    float sun_halo = pow(max(sun_dot, 0.0), 256.0) * 0.6;
    float sun_glow = pow(max(sun_dot, 0.0), 32.0) * 0.15;
    vec3 sun_rgb = ubo.sun_color.xyz * ubo.sun_color.w;

    sky_color += sun_rgb * (sun_disc + sun_halo + sun_glow);

    // Sunset glow on horizon
    float sunset_glow_factor = smoothstep(0.3, 0.0, sun_height) * smoothstep(-0.1, 0.0, sun_height);
    float horizon_proximity = exp(-abs(elevation) * 5.0);
    float sun_azimuth_factor = pow(max(dot(
        normalize(vec3(ray_dir.x, 0.0, ray_dir.z)),
        normalize(vec3(sun_dir.x, 0.0, sun_dir.z))
    ), 0.0), 4.0);
    vec3 sunset_glow = vec3(1.0, 0.4, 0.1) * sunset_glow_factor * horizon_proximity * sun_azimuth_factor * 0.4;
    sky_color += sunset_glow;

    return sky_color;
}

// ─── Main ───────────────────────────────────────────────────────────────────

void main() {
    vec3 N = normalize(frag_normal);
    vec3 V = normalize(ubo.camera_pos.xyz - frag_world_pos);

    // Reflection direction
    vec3 reflect_dir = reflect(-V, N);

    // Sky reflection
    vec3 sky_reflection = evaluateSky(reflect_dir);

    // Fresnel (Schlick)
    float NdotV = max(dot(N, V), 0.0);
    float fresnel = 0.02 + 0.98 * pow(1.0 - NdotV, 5.0);

    // Water base color
    vec3 water_base = vec3(0.01, 0.03, 0.05);

    // Surface: blend between deep water and sky reflection
    vec3 surface = mix(water_base, sky_reflection, fresnel);

    // Sun specular (Blinn-Phong)
    vec3 L = normalize(-ubo.sun_direction.xyz);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 256.0);
    vec3 specular = ubo.sun_color.xyz * ubo.sun_color.w * spec * 0.5;

    vec3 color = surface + specular;

    // Reinhard tone mapping
    color = color / (color + 1.0);

    out_color = vec4(color, 1.0);
}
