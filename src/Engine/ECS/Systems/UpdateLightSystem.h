#pragma once
#include "System.h"
#include <glm/glm.hpp>
#include <vector>

struct FragmentUniformBufferObject;

class UpdateLightSystem :
    public System
{
public:
    void UpdateLightInUBO(FragmentUniformBufferObject& ubo);
    std::vector<Entity> GetOrderedLightEntities() const;
};

