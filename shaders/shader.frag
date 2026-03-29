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

layout(set = 0, binding = 2) uniform FragmentUBO {
    LightInfo lights[32];
    float shadow_near;
    float shadow_far;
    float render_mode;
    float _pad_render_mode;
} ubo;

layout(set = 0, binding = 3) uniform samplerCubeShadow shadowCube;

#define USE_SHADOW 1

#define SHADOW_MAP_RES 1024.0
#define PCF_STEP_TEXELS 2.25
#define PCF_SAMPLES 25
// Vogel disk: PI * (3 - sqrt(5))
#define PCF_GOLDEN_ANGLE 2.39996322972865332

float samplePointLightShadowPcf(samplerCubeShadow cube, vec3 sampleDir, float depthCompare) {
    vec3 L = sampleDir;
    vec3 up = abs(L.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 T = normalize(cross(up, L));
    vec3 B = cross(L, T);

    const float faceSpanRad = 1.57079632679;
    float texelAngle = faceSpanRad / SHADOW_MAP_RES;
    float step = texelAngle * PCF_STEP_TEXELS;
    // Match old 5x5 extent: farthest taps were at offset length sqrt((2*step)^2 + (2*step)^2).
    float diskRadius = 2.8284271247461903 * step;

    float sum = 0.0;
    for (int k = 0; k < PCF_SAMPLES; k++) {
        float fi = float(k) + 0.5;
        float r = sqrt(fi / float(PCF_SAMPLES));
        float theta = float(k) * PCF_GOLDEN_ANGLE;
        vec2 o = vec2(cos(theta), sin(theta)) * r;
        vec3 off = L + T * (o.x * diskRadius) + B * (o.y * diskRadius);
        sum += texture(cube, vec4(normalize(off), depthCompare));
    }
    return sum / float(PCF_SAMPLES);
}

vec4 CalcLightResult(vec3 lightPos, vec4 lightCol) {
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragWorldPos);
    float diff = max(dot(norm, lightDir), 0.0);
    return diff * lightCol;
}

void main() {

    if (ubo.render_mode > 0.5) {
        vec3 n = normalize(fragNormal);
        outColor = vec4(n * 0.5 + 0.5, 1.0);
        return;
    }

    outColor = texture(texSampler[textureIndex], fragTexCoord);

    //ambient lighting
    float ambientStrength = 0.01;
    vec4 ambientColor = vec4(0.6, 0.6, 1.0, 1.0);
    vec4 finalLight = ambientStrength * ambientColor;

    for (int i = 0; i < 32; i++) {
        vec4 lightContrib = CalcLightResult(ubo.lights[i].light_pos, ubo.lights[i].light_col);
        if (i == 0) {
#if USE_SHADOW
            vec3 toLight = fragWorldPos - ubo.lights[0].light_pos;
            float dist = length(toLight);
            vec3 sampleDir = normalize(toLight);
            sampleDir.y *= (-1.0);

            float n = ubo.shadow_near;
            float f = ubo.shadow_far;
            float depthRange = f - n;
            float depthLinear = (dist - n) / depthRange;

            vec3 N = normalize(fragNormal);
            vec3 LtoSurf = normalize(ubo.lights[0].light_pos - fragWorldPos);
            float ndotl = max(dot(N, LtoSurf), 0.001);
            const float shadowReceiverConstWorld = 0.0005;
            const float shadowReceiverSlopeWorld = 0.04;
            float receiverBias =
                (shadowReceiverConstWorld + shadowReceiverSlopeWorld * (1.0 - ndotl)) / depthRange;
            float depthCompare = clamp(depthLinear - receiverBias, 0.0, 1.0);
            float shadowFactor = samplePointLightShadowPcf(shadowCube, sampleDir, depthCompare);
            lightContrib *= shadowFactor;
#else
            lightContrib *= 1.0;
#endif
        }
        finalLight = finalLight + lightContrib;
    }

    outColor = finalLight * outColor;

}