#version 450

layout(location = 0) in vec2 in_uv;
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

// ─── Hash & Noise ───────────────────────────────────────────────────────────

vec2 hash2(vec2 p) {
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453);
}

float hash21(vec2 p) {
    vec2 h = hash2(p);
    return h.x;
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

float fbm(vec2 p, int octaves) {
    const mat2 rot = mat2(0.8, 0.6, -0.6, 0.8);
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < octaves; i++) {
        value += amplitude * gradientNoise(p);
        p = rot * p * 2.0;
        amplitude *= 0.5;
    }
    return value;
}

float cloudFBM(vec2 uv, float morph_factor) {
    float n1 = fbm(uv, 5);
    float n2 = fbm(uv + vec2(7.3, 3.1), 5);
    float n = mix(n1, n2, morph_factor);
    n = n * 0.5 + 0.5;  // remap [-0.5,0.5] → [0,1]

    float coverage = 0.5;
    n = smoothstep(coverage - 0.1, coverage + 0.3, n);

    float detail = fbm(uv * 4.0 + 3.7, 3) * 0.5 + 0.5;
    n *= mix(0.7, 1.0, detail);

    return clamp(n, 0.0, 1.0);
}

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

// ─── Ray Direction ──────────────────────────────────────────────────────────

vec3 getRayDirection(vec2 uv) {
    vec2 ndc = uv * 2.0 - 1.0;
    vec4 world_pos = ubo.inv_view_proj * vec4(ndc, 1.0, 1.0);
    world_pos /= world_pos.w;
    return normalize(world_pos.xyz - ubo.camera_pos.xyz);
}

// ─── Dynamic Sky Gradient ───────────────────────────────────────────────────

vec3 getSkyColor(float elevation, float sun_height) {
    // Day
    vec3 day_zenith    = vec3(0.15, 0.3, 0.65);
    vec3 day_horizon   = vec3(0.6, 0.75, 0.9);
    // Sunset/sunrise
    vec3 sunset_zenith  = vec3(0.15, 0.15, 0.35);
    vec3 sunset_horizon = vec3(0.9, 0.4, 0.15);
    // Twilight
    vec3 twilight_zenith  = vec3(0.05, 0.05, 0.15);
    vec3 twilight_horizon = vec3(0.3, 0.15, 0.3);
    // Night
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

// ─── Stars ──────────────────────────────────────────────────────────────────

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

    // Twinkling
    float twinkle_seed = fract(sin(dot(floor(vec2(atan(ray_dir.z, ray_dir.x), asin(clamp(ray_dir.y, -1.0, 1.0))) * 100.0), vec2(127.1, 311.7))) * 43758.5453);
    float twinkle = 0.7 + 0.3 * sin(ubo.elapsed_time * 3.0 + twinkle_seed * 100.0);

    float above_horizon = smoothstep(-0.02, 0.05, elevation);

    vec3 star_color = vec3(0.8, 0.85, 1.0);
    return star_color * star * twinkle * night_factor * above_horizon;
}

// ─── Moon ───────────────────────────────────────────────────────────────────

vec3 renderMoon(vec3 ray_dir, vec3 sun_dir, float sun_height) {
    vec3 moon_dir = -sun_dir;
    float moon_dot = dot(ray_dir, moon_dir);

    float moon_disc = smoothstep(0.9985, 0.999, moon_dot);
    if (moon_disc < 0.001) return vec3(0.0);

    // Project onto moon disc plane for crater UV
    vec3 moon_up = normalize(cross(moon_dir, vec3(0.0, 0.0, 1.0)));
    vec3 moon_right = cross(moon_up, moon_dir);
    vec2 moon_uv = vec2(
        dot(ray_dir - moon_dir, moon_right),
        dot(ray_dir - moon_dir, moon_up)
    ) * 30.0;

    // Voronoi craters
    float craters = voronoi(moon_uv * 3.0);
    float crater_shade = 0.7 + 0.3 * smoothstep(0.0, 0.3, craters);

    vec3 moon_color = vec3(0.8, 0.82, 0.85) * crater_shade;

    // Moon visible when sun is below horizon
    float moon_intensity = smoothstep(0.1, -0.1, sun_height) * 0.3;

    return moon_color * moon_disc * moon_intensity;
}

// ─── Clouds ─────────────────────────────────────────────────────────────────

vec4 renderClouds(vec3 ray_dir, vec3 sun_dir, float sun_height, float elevation) {
    if (elevation < 0.01) return vec4(0.0);

    // Ray-plane intersection: cloud layer at Y = -cloudHeight (Y-down)
    const float cloud_height = 2000.0;
    float t = min((cloud_height + ubo.camera_pos.y) / elevation, 50000.0);
    vec2 cloud_uv = (ubo.camera_pos.xz + ray_dir.xz * t) * 0.00015;

    // Wind animation
    vec2 wind_dir = normalize(vec2(1.0, 0.3));
    cloud_uv += wind_dir * ubo.elapsed_time * 0.005;

    // Shape morphing
    float morph_factor = sin(ubo.elapsed_time * 0.02) * 0.5 + 0.5;

    // Cloud density
    float density = cloudFBM(cloud_uv, morph_factor);
    if (density < 0.001) return vec4(0.0);

    // Beer's Law illumination
    vec2 sun_uv_offset = sun_dir.xz * 0.002;
    float density_towards_sun = cloudFBM(cloud_uv + sun_uv_offset, morph_factor);
    float light_attenuation = exp(-density_towards_sun * 3.0);

    // Cloud colors
    vec3 cloud_bright = vec3(1.0, 0.98, 0.95);
    vec3 cloud_dark   = vec3(0.4, 0.42, 0.5);

    // Sunset tinting
    float sunset_factor = smoothstep(0.3, 0.0, sun_height) * smoothstep(-0.1, 0.0, sun_height);
    vec3 sun_tint = mix(vec3(1.0), vec3(1.5, 0.7, 0.3), sunset_factor);
    cloud_bright *= sun_tint;

    vec3 cloud_color = mix(cloud_dark, cloud_bright, light_attenuation);

    // Night dimming
    float day_factor = smoothstep(-0.1, 0.1, sun_height);
    cloud_color *= mix(0.05, 1.0, day_factor);

    // Horizon fade
    float horizon_fade = smoothstep(0.0, 0.15, elevation);
    float cloud_alpha = density * horizon_fade;

    return vec4(cloud_color, cloud_alpha);
}

// ─── Main ───────────────────────────────────────────────────────────────────

void main() {
    vec3 ray_dir = getRayDirection(in_uv);

    // Elevation: negate Y because engine uses Y-down (negative Y = up)
    float elevation = -ray_dir.y;

    vec3 sun_dir = normalize(ubo.sun_direction.xyz);
    float sun_height = -sun_dir.y;  // positive = above horizon

    // Dynamic sky gradient
    vec3 sky_color = getSkyColor(elevation, sun_height);

    // Sun disc + halo + glow
    float sun_dot = dot(ray_dir, sun_dir);
    float sun_disc = smoothstep(0.9995, 0.9999, sun_dot);
    float sun_halo = pow(max(sun_dot, 0.0), 256.0) * 0.6;
    float sun_glow = pow(max(sun_dot, 0.0), 32.0) * 0.15;
    vec3 sun_rgb = ubo.sun_color.xyz * ubo.sun_color.w;

    // Sunset glow on horizon
    float sunset_glow_factor = smoothstep(0.3, 0.0, sun_height) * smoothstep(-0.1, 0.0, sun_height);
    float horizon_proximity = exp(-abs(elevation) * 5.0);
    float sun_azimuth_factor = pow(max(dot(
        normalize(vec3(ray_dir.x, 0.0, ray_dir.z)),
        normalize(vec3(sun_dir.x, 0.0, sun_dir.z))
    ), 0.0), 4.0);
    vec3 sunset_glow = vec3(1.0, 0.4, 0.1) * sunset_glow_factor * horizon_proximity * sun_azimuth_factor * 0.4;

    // Stars
    vec3 stars = renderStars(ray_dir, sun_height, elevation);

    // Moon
    vec3 moon = renderMoon(ray_dir, sun_dir, sun_height);

    // Clouds
    vec4 clouds = renderClouds(ray_dir, sun_dir, sun_height, elevation);

    // Composite
    vec3 final_color = sky_color;
    final_color += sun_rgb * (sun_disc + sun_halo + sun_glow);
    final_color += sunset_glow;
    final_color += stars;
    final_color += moon;

    // Clouds blend over everything (occlude stars naturally)
    final_color = mix(final_color, clouds.rgb, clouds.a);

    // Reinhard tone mapping
    final_color = final_color / (final_color + vec3(1.0));

    out_color = vec4(final_color, 1.0);
}
