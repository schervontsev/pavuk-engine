#pragma once
#include "System.h"
class Scene;

class TestSystem :
    public System
{
public:
    void Update(float dt, Scene* scene);
};

