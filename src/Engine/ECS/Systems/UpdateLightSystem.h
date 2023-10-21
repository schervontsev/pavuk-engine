#pragma once
#include "System.h"
struct UniformBufferObject;

class UpdateLightSystem :
    public System
{
public:
    void UpdateLightInUBO(UniformBufferObject& ubo);
};

