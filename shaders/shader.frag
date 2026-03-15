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

layout(set = 0, binding = 1) uniform sampler2D texSampler[1];

layout(set = 0, binding = 2) uniform VertexUniformBufferObject {
    LightInfo lights[32];
} ubo;

layout(set = 0, binding = 3) uniform samplerCubeShadow shadowCube;

const float shadowFarPlane = 100.0;
#define USE_SHADOW 0  // set to 1 to enable shadow map for first light

vec4 CalcLightResult(vec3 lightPos, vec4 lightCol) {
    vec3 norm = normalize(fragNormal);
    // Y-flip at load: we draw back face, normal points inward. Outward = -norm points toward viewer.
    // Use direction from light to fragment so it aligns with outward when surface faces the light.
    vec3 outward = -norm;
    vec3 lightToFrag = normalize(fragWorldPos - lightPos);
    float diff = max(dot(outward, lightToFrag), 0.0);
    return diff * lightCol;
}

void main() {
    outColor = texture(texSampler[textureIndex], fragTexCoord);

    //ambient lighting
    float ambientStrength = 0.01;
    vec4 ambientColor = vec4(0.6, 0.6, 1.0, 1.0);
    vec4 finalLight = ambientStrength * ambientColor;

    //point lighting with shadow for first light
    for (int i = 0; i < 32; i++) {
        vec4 lightContrib = CalcLightResult(ubo.lights[i].light_pos, ubo.lights[i].light_col);
        if (i == 0) {
#if USE_SHADOW
            vec3 toLight = fragWorldPos - ubo.lights[0].light_pos;
            float dist = length(toLight);
            float shadow = texture(shadowCube, vec4(normalize(toLight), dist / shadowFarPlane));
            lightContrib *= shadow;
#else
            lightContrib *= 1.0;  // shadow disabled
#endif
        }
        finalLight = finalLight + lightContrib;
    }

    //result
    outColor = finalLight * outColor;

    //debug
    //outColor = vec4(fragWorldPos, 1.0);
}