#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in flat int textureIndex;
layout(location = 3) in vec3 fragNormal;
layout(location = 4) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;
layout(set = 0, binding = 1) uniform sampler2D texSampler[5];

void main() {
    outColor = texture(texSampler[textureIndex], fragTexCoord);

    //ambient lighting
    float ambientStrength = 0.01;
    vec4 ambientColor = vec4(0.6, 0.6, 1.0, 1.0);
    vec4 ambient = ambientStrength * ambientColor;

    //point lighting
    vec3 lightPos = vec3(10.0, -0.2, 0.0);
    vec4 lightColor = vec4(1.0, 0.5, 0.0, 1.0);

    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragWorldPos);

    float diff = max(dot(fragNormal, lightDir), 0.0);
    vec4 diffuse = diff * lightColor;

    //result
    outColor = (ambient + diffuse) * outColor;

    //debug
    //outColor = vec4(fragWorldPos, 1.0);
}