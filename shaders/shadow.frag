#version 450

layout(binding = 0) uniform ShadowUBO {
    mat4 light_view_proj;
    vec4 light_pos;
    float far_plane;
} ubo;

layout(location = 5) in vec3 worldPos;

void main() {
    gl_FragDepth = length(worldPos - ubo.light_pos.xyz) / ubo.far_plane;
}
