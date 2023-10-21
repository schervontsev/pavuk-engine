#pragma once
#include "System.h"
struct FragmentUniformBufferObject;

class UpdateLightSystem :
    public System
{
public:
    void UpdateLightInUBO(FragmentUniformBufferObject& ubo);
};

