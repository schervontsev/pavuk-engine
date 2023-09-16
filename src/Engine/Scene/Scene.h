#pragma once
#include <vector>
#include "../Scene/Mesh.h"
#include "../Render/Material.h"
class Scene
{
public:
    Scene();

public:
    std::vector<Mesh> models;
    std::vector<Material> materials;

    void Update(float dt);
};