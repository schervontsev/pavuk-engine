#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view_proj;
} ubo;

//push constants block
layout( push_constant ) uniform constants
{
	mat4 transform;
} PushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in int inTextureIndex;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out int textureIndex;

void main() {
    gl_Position = ubo.view_proj * PushConstants.transform * vec4(inPosition, 1.0);// + gl_InstanceIndex;
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    textureIndex = inTextureIndex;
}