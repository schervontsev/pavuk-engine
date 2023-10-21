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
