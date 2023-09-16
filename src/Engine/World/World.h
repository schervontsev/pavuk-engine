#pragma once
#include <vector>
#include "../World/Mesh.h"
#include "../Render/Material.h"
class World
{
public:
    World();

public:
    std::vector<Mesh> models;
    std::vector<Material> materials;

    void Update(float dt);
};