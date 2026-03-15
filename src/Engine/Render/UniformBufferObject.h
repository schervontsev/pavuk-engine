#pragma once
#include <glm/glm.hpp>

struct LightInfo {
    alignas(16) glm::vec4 light_pos = { glm::vec4(0.f) };
    alignas(16) glm::vec4 light_col = { glm::vec4(0.f) };
};

struct VertexUniformBufferObject {
    alignas(16) glm::mat4 view_proj;
};

struct FragmentUniformBufferObject {
    alignas(16) LightInfo lights[32] = { LightInfo() };
};

struct ShadowUBOData {
    alignas(16) glm::mat4 light_view_proj;
    alignas(16) glm::vec4 light_pos;
    float far_plane;
};
