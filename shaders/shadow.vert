#version 450

layout(binding = 0) uniform ShadowUBO {
    mat4 light_view_proj;
} ubo;

layout(push_constant) uniform constants {
    mat4 transform;
    mat4 normal_matrix;
} PushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in int inTextureIndex;

layout(location = 5) out vec3 worldPos;

void main() {
    gl_Position = ubo.light_view_proj * PushConstants.transform * vec4(inPosition, 1.0);
    worldPos = (PushConstants.transform * vec4(inPosition, 1.0)).xyz;
}
