#version 450

layout(push_constant) uniform ShadowPush {
    mat4 light_view_proj;
    mat4 transform;
    mat4 normal_matrix;
} ShadowPC;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in int inTextureIndex;

layout(location = 5) out vec3 worldPos;

void main() {
    gl_Position = ShadowPC.light_view_proj * ShadowPC.transform * vec4(inPosition, 1.0);
    worldPos = (ShadowPC.transform * vec4(inPosition, 1.0)).xyz;
}
