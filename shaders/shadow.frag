#version 450

layout(binding = 0) uniform ShadowUBO {
    vec4 light_pos;
    vec2 near_far;
    vec2 _pad;
} ubo;

layout(location = 5) in vec3 worldPos;

void main() {
    float dist = length(worldPos - ubo.light_pos.xyz);
    float n = ubo.near_far.x;
    float f = ubo.near_far.y;
    float depthRange = f - n;
    float depth = (dist - n) / depthRange;

    const float shadowMapDepthBiasWorld = 0.0004;
    gl_FragDepth = clamp(depth + shadowMapDepthBiasWorld / depthRange, 0.0, 1.0);
}
