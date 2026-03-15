#version 450

layout(binding = 0) uniform VertexUniformBufferObject {
    mat4 view_proj;
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

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = ubo.view_proj * PushConstants.transform * vec4(inPosition, 1.0);
    fragColor = inColor;
}
