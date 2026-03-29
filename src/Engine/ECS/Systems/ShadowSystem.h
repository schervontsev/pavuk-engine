#pragma once
#include "System.h"
#include <glm/glm.hpp>
#include <vector>

struct FragmentUniformBufferObject;

class ShadowSystem : public System {
public:
    void AssignShadowCubeIndices(FragmentUniformBufferObject& ubo, const std::vector<Entity>& orderedLightEntities) const;
    glm::vec3 GetShadowLightPosition(size_t cubeIndex) const;
};
