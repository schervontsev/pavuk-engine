#pragma once
#include "System.h"
#include <glm/glm.hpp>
struct FragmentUniformBufferObject;

class UpdateLightSystem :
    public System
{
public:
    void UpdateLightInUBO(FragmentUniformBufferObject& ubo);
    glm::vec3 GetFirstLightPosition() const;
};

