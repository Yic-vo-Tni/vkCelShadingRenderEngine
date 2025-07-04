#version 450

layout(location = 0) out vec4 outColor;

// 你可以用uniform替换下面两个全局常量，下面这样方便单步调试
//uniform float iTime;
//uniform vec2 iResolution;
//const float iTime = 0;
layout(push_constant) uniform PushConstants{ float iTime;};
const vec2 iResolution = vec2(2560, 1440);

// 结构体
struct ray_t {
    vec3 origin;
    vec3 direction;
};
struct volume_sampler_t {
    vec3 origin;
    vec3 pos;
    float height;
    float coeff_absorb;
    float T;
    vec3 C;
    float alpha;
};

// 参数
const float PI = 3.14159265359;
const int cld_march_steps = 50;
//50
const float cld_coverage = 0.4;
//0.3125
const float cld_thick = 70.0;
//90
const float cld_absorb_coeff = 1.0;

// 工具函数
vec3 linear_to_srgb(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}

float smooth_band(float start, float peak, float end, float t) {
    return smoothstep(start, peak, t) * (1.0 - smoothstep(peak, end, t));
}

// Noise
float hash(float n) {
    return fract(sin(n) * 753.5453123);
}
float noise(vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
    float n = p.x + p.y * 157.0 + 113.0 * p.z;
    return mix(mix(mix(hash(n + 0.0), hash(n + 1.0), f.x),
                   mix(hash(n + 157.0), hash(n + 158.0), f.x), f.y),
               mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
                   mix(hash(n + 270.0), hash(n + 271.0), f.x), f.y), f.z);
}
float fbm_clouds(vec3 pos, float lacunarity, float init_gain, float gain) {
    vec3 p = pos;
    float H = init_gain;
    float t = 0.0;
    for (int i = 0; i < 5; i++) {
        t += abs(noise(p)) * H;
        p *= lacunarity;
        H *= gain;
    }
    return t;
}

vec3 render_sky_color(vec3 eye_dir) {
    vec3 sun_color = vec3(1.0, 0.7, 0.55);
    float sun_amount = max(dot(eye_dir, normalize(vec3(0, 0, -1))), 0.0);
    vec3 sky = mix(vec3(0.0, 0.1, 0.4), vec3(0.3, 0.6, 0.8), 1.0 - eye_dir.y);
    sky += sun_color * min(pow(sun_amount, 1500.0) * 5.0, 1.0);
    sky += sun_color * min(pow(sun_amount, 10.0) * 0.6, 1.0);
    return sky;
}

float density_func(vec3 pos, float h, float time) {
    vec3 wind_dir = vec3(0, 0, -time * 0.2);
    vec3 p = pos * 0.001 + wind_dir;
    float dens = fbm_clouds(p * 2.032, 2.6434, 0.5, 0.5);
    dens *= smoothstep(cld_coverage, cld_coverage + 0.35, dens);
    //0.035
    return dens;
}

float illuminate_volume(volume_sampler_t cloud) {
    return exp(cloud.height) / 1.95;
}

void integrate_volume(
inout volume_sampler_t vol,
      vec3 V,
      vec3 L,
      float density,
      float dt)
{
    float T_i = exp(-vol.coeff_absorb * density * dt);
    vol.T *= T_i;
    vol.C += vol.T * illuminate_volume(vol) * density * dt;
    vol.alpha += (1.0 - T_i) * (1.0 - vol.alpha);
}

volume_sampler_t begin_volume(vec3 origin, float coeff_absorb) {
    volume_sampler_t v;
    v.origin = origin;
    v.pos = origin;
    v.height = 0.0;
    v.coeff_absorb = coeff_absorb;
    v.T = 1.0;
    v.C = vec3(0.0);
    v.alpha = 0.0;
    return v;
}

vec4 render_clouds(ray_t eye, float time) {
    int steps = cld_march_steps;
    float march_step = cld_thick / float(steps);
    vec3 projection = eye.direction / eye.direction.y;
    vec3 iter = projection * march_step;
    float cutoff = dot(eye.direction, vec3(0, 1, 0));
    volume_sampler_t cloud = begin_volume(eye.origin + projection * 100.0, cld_absorb_coeff);

    for (int i = 0; i < steps; i++) {
        cloud.height = (cloud.pos.y - cloud.origin.y) / cld_thick;
        float dens = density_func(cloud.pos, cloud.height, time);
        integrate_volume(cloud, eye.direction, normalize(vec3(0, 0, -1)), dens, march_step);
        cloud.pos += iter;
        if (cloud.alpha > 0.999) break;
    }
    return vec4(cloud.C, cloud.alpha * smoothstep(0.0, 0.2, cutoff));
}

ray_t get_primary_ray(vec3 cam_local_point, vec3 cam_origin, vec3 cam_look_at) {
    vec3 fwd = normalize(cam_look_at - cam_origin);
    vec3 up = vec3(0, 1, 0);
    vec3 right = cross(up, fwd);
    up = cross(fwd, right);
    ray_t r;
    r.origin = cam_origin;
    r.direction = normalize(fwd + up * cam_local_point.y + right * cam_local_point.x);
    return r;
}

vec3 render(ray_t eye_ray, vec3 point_cam, float time) {
    vec3 sky = render_sky_color(eye_ray.direction);
    if (dot(eye_ray.direction, vec3(0, 1, 0)) < 0.05) return sky;
    vec4 cld = render_clouds(eye_ray, time);
    return mix(sky, cld.rgb, cld.a);
}

void main() {
    float time = iTime;
    vec2 fragCoord = gl_FragCoord.xy;
    vec2 res = iResolution;
    vec2 aspect_ratio = vec2(res.x / res.y, 1);
    vec3 eye = vec3(0.f, 0.f, 25.f);//
    // 0.f, 1.f, 0.f
    vec3 look_at = vec3(0.f, 0.f, 25.f) + vec3(0.f, 0.f, -1.f);
    //0.f, 1.6f, -1.f
    float FOV = 1.f;
    // 1.f
    vec2 point_ndc = fragCoord / res;
    point_ndc.y = 1.f - point_ndc.y;
    vec3 point_cam = vec3((2.0 * point_ndc - 1.0) * aspect_ratio * FOV, -1.0);
    ray_t ray = get_primary_ray(point_cam, eye, look_at);

    vec3 color;
    if (point_ndc.y < 0.45) {
        vec3 ground_color = vec3(0.42, 0.32, 0.18);
        color = ground_color;
    } else {
        color = render(ray, point_cam, time);
    }

    outColor = vec4(linear_to_srgb(color), 1.0);

//    vec3 color = render(ray, point_cam, time);
//    outColor = vec4(linear_to_srgb(color), 1.0);
}


