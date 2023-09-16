#pragma once
#include <vector>
#include "../Scene/Mesh.h"
#include "../Render/Material.h"
class Scene
{
public:
    Scene();

private:
    bool isDirty = false;
    double timeFromStart = 0.f;
public:
    std::vector<Mesh> models;
    std::vector<Material> materials;

    void Update(float dt);
    void SetDirty(bool val) { isDirty = val; };
    bool IsDirty() { return isDirty; };
};