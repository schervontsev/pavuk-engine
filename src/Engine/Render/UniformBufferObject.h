#pragma once
#include <glm/glm.hpp>

struct UniformBufferObject {
    alignas(16) glm::mat4 view_proj;
};
