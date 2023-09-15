#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in flat int textureIndex;

layout(location = 0) out vec4 outColor;
layout(set = 0, binding = 1) uniform sampler2D texSampler[5];

void main() {
    outColor = texture(texSampler[textureIndex], fragTexCoord);
}