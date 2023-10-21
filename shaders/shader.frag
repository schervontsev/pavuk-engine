#version 450
#extension GL_EXT_nonuniform_qualifier : enable

struct LightInfo {
    vec3 light_pos;
    vec4 light_col;
};

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in flat int textureIndex;
layout(location = 3) in vec3 fragNormal;
layout(location = 4) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D texSampler[64];

layout(binding = 2) uniform VertexUniformBufferObject {
    LightInfo lights[32];
} ubo;

vec4 CalcLightResult(vec3 lightPos, vec4 lightCol) {
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragWorldPos);

    float diff = max(dot(fragNormal, lightDir), 0.0);
    return diff * lightCol;
}

void main() {
    outColor = texture(texSampler[textureIndex], fragTexCoord);

    //ambient lighting
    float ambientStrength = 0.01;
    vec4 ambientColor = vec4(0.6, 0.6, 1.0, 1.0);
    vec4 finalLight = ambientStrength * ambientColor;

    //point lighting
    for (int i = 0; i < 32; i++) {
        finalLight = finalLight + CalcLightResult(ubo.lights[i].light_pos, ubo.lights[i].light_col);
    }

    //result
    outColor = finalLight * outColor;

    //debug
    //outColor = vec4(fragWorldPos, 1.0);
}