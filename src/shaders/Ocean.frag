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

// ─── Noise Utilities (duplicated from Sky.frag) ─────────────────────────────

vec2 hash2(vec2 p) {
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453);
}

vec2 voronoi2(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    float F1 = 1.0;
    float F2 = 1.0;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            vec2 neighbor = vec2(float(x), float(y));
            vec2 point = hash2(i + neighbor);
            float d = length(neighbor + point - f);
            if (d < F1) {
                F2 = F1;
                F1 = d;
            } else if (d < F2) {
                F2 = d;
            }
        }
    }
    return vec2(F1, F2);
}

float gradientNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);
    float a = dot(hash2(i + vec2(0.0, 0.0)) * 2.0 - 1.0, f - vec2(0.0, 0.0));
    float b = dot(hash2(i + vec2(1.0, 0.0)) * 2.0 - 1.0, f - vec2(1.0, 0.0));
    float c = dot(hash2(i + vec2(0.0, 1.0)) * 2.0 - 1.0, f - vec2(0.0, 1.0));
    float d = dot(hash2(i + vec2(1.0, 1.0)) * 2.0 - 1.0, f - vec2(1.0, 1.0));
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

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

// ─── Voronoi (single distance, for moon craters) ─────────────────────────────

float voronoi(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    float min_dist = 1.0;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            vec2 neighbor = vec2(float(x), float(y));
            vec2 point = hash2(i + neighbor);
            vec2 diff = neighbor + point - f;
            min_dist = min(min_dist, length(diff));
        }
    }
    return min_dist;
}

// ─── Stars ───────────────────────────────────────────────────────────────────

float starField(vec3 ray_dir) {
    vec3 dir = normalize(ray_dir);
    float theta = atan(dir.z, dir.x);
    float phi = asin(clamp(dir.y, -1.0, 1.0));

    vec2 cell = vec2(theta, phi) * 100.0;
    vec2 cell_id = floor(cell);
    vec2 cell_frac = fract(cell);

    float h = fract(sin(dot(cell_id, vec2(127.1, 311.7))) * 43758.5453);
    if (h > 0.02) return 0.0;

    vec2 star_pos = hash2(cell_id);
    float dist = length(cell_frac - star_pos);
    float star = smoothstep(0.08, 0.02, dist);
    float brightness = fract(h * 1000.0);

    return star * (0.3 + 0.7 * brightness);
}

vec3 renderStars(vec3 ray_dir, float sun_height, float elevation) {
    float night_factor = smoothstep(0.0, -0.15, sun_height);
    if (night_factor < 0.001) return vec3(0.0);

    float star = starField(ray_dir);

    float twinkle_seed = fract(sin(dot(floor(vec2(atan(ray_dir.z, ray_dir.x), asin(clamp(ray_dir.y, -1.0, 1.0))) * 100.0), vec2(127.1, 311.7))) * 43758.5453);
    float twinkle = 0.7 + 0.3 * sin(ubo.elapsed_time * 3.0 + twinkle_seed * 100.0);

    float above_horizon = smoothstep(-0.02, 0.05, elevation);

    vec3 star_color = vec3(0.8, 0.85, 1.0);
    return star_color * star * twinkle * night_factor * above_horizon;
}

// ─── Moon ────────────────────────────────────────────────────────────────────

vec3 renderMoon(vec3 ray_dir, vec3 sun_dir, float sun_height) {
    vec3 moon_dir = -sun_dir;
    float moon_dot = dot(ray_dir, moon_dir);

    float moon_disc = smoothstep(0.9985, 0.999, moon_dot);
    if (moon_disc < 0.001) return vec3(0.0);

    vec3 moon_up = normalize(cross(moon_dir, vec3(0.0, 0.0, 1.0)));
    vec3 moon_right = cross(moon_up, moon_dir);
    vec2 moon_uv = vec2(
        dot(ray_dir - moon_dir, moon_right),
        dot(ray_dir - moon_dir, moon_up)
    ) * 30.0;

    float craters = voronoi(moon_uv * 3.0);
    float crater_shade = 0.7 + 0.3 * smoothstep(0.0, 0.3, craters);

    vec3 moon_color = vec3(0.8, 0.82, 0.85) * crater_shade;

    float moon_intensity = smoothstep(0.1, -0.1, sun_height) * 0.8;

    return moon_color * moon_disc * moon_intensity;
}

// ─── Evaluate Sky (gradient + sun + stars + moon) ────────────────────────────

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

    // Stars
    sky_color += renderStars(ray_dir, sun_height, elevation);

    // Moon
    sky_color += renderMoon(ray_dir, sun_dir, sun_height);

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
    vec3 water_base = vec3(0.0, 0.05, 0.1);

    // Surface: blend between deep water and sky reflection
    vec3 surface = mix(water_base, sky_reflection, fresnel);

    // Ambient lighting
    vec3 ambient = ubo.ambient_light_color.xyz * ubo.ambient_light_color.w * water_base;

    // Sun specular (Blinn-Phong)
    vec3 L = normalize(ubo.sun_direction.xyz);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 256.0);
    vec3 specular = ubo.sun_color.xyz * ubo.sun_color.w * spec * 0.5;

    vec3 color = surface + specular + ambient;

    // Foam — crest detection + cellular bubble pattern
    float max_wave_h = 0.0;
    for (int i = 0; i < 6; i++) {
        max_wave_h += ubo.waves[i].amplitude;
    }
    float foam_mask = smoothstep(max_wave_h * 0.7, max_wave_h * 0.9, -frag_world_pos.y);

    // Multi-scale Voronoi F2-F1 for bubble edges
    vec2 foam_uv = frag_world_pos.xz;
    float time_anim = ubo.elapsed_time * 0.3;

    vec2 v1 = voronoi2(foam_uv * 2.0 + time_anim);
    float edges1 = smoothstep(0.0, 0.15, v1.y - v1.x);

    vec2 v2 = voronoi2(foam_uv * 6.0 - time_anim * 0.7);
    float edges2 = smoothstep(0.0, 0.12, v2.y - v2.x);

    float bubble_pattern = max(edges1, edges2 * 0.6);

    // FBM modulation for organic patch shapes
    float coverage_noise = gradientNoise(foam_uv * 0.5 + time_anim * 0.2) * 0.5 + 0.5;
    float modulated_mask = foam_mask * smoothstep(0.2, 0.6, coverage_noise + foam_mask * 0.5);

    float foam = modulated_mask * bubble_pattern;
    color = mix(color, vec3(0.9, 0.95, 1.0), foam * 0.8);

    // Fog — exponential atmospheric fog, blends ocean into sky at horizon
    float dist = length(frag_world_pos - ubo.camera_pos.xyz);
    float fog_factor = 1.0 - exp(-dist * ubo.fog_density);
    vec3 fog_dir = normalize(frag_world_pos - ubo.camera_pos.xyz);
    vec3 fog_color = evaluateSky(fog_dir);
    color = mix(color, fog_color, fog_factor);

    // Reinhard tone mapping
    color = color / (color + 1.0);

    out_color = vec4(color, 1.0);
}
