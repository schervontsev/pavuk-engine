#version 450

struct LightInfo {
    vec3 light_pos;
    vec4 light_col;
};

layout(binding = 0) uniform UniformBufferObject {
    mat4 view_proj;
    LightInfo lights[16];
} ubo;

//push constants block
layout( push_constant ) uniform constants
{
	mat4 transform;
    mat3 normal_matrix;
} PushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in int inTextureIndex;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out int textureIndex;
layout(location = 3) out vec3 fragNormal;
layout(location = 4) out vec3 fragWorldPos;
layout(location = 5) out LightInfo lights[16];

void main() {
    gl_Position = ubo.view_proj * PushConstants.transform * vec4(inPosition, 1.0);// + gl_InstanceIndex;
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    textureIndex = inTextureIndex;
    
    fragNormal = normalize(inNormal * PushConstants.normal_matrix);
    fragWorldPos = vec3(PushConstants.transform * vec4(inPosition, 1.0));
    lights = ubo.lights;
}