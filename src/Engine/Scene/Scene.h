#pragma once
#include <vector>
#include "../Scene/Mesh.h"
#include "../Render/Material.h"
class SystemManager;
class EntityManager;
class ComponentManager;

class Scene
{
public:
    Scene();
    void Init();

public:
    std::vector<Mesh> models;
    std::vector<Material> materials;

    void Update(float dt);
    void SetDirty(bool val) { isDirty = val; };
    bool IsDirty() { return isDirty; };

private:
    bool isDirty = false;
    double timeFromStart = 0.f;

    std::unique_ptr<ComponentManager> componentManager;
    std::unique_ptr<SystemManager> systemManager;
    std::unique_ptr<EntityManager> entityManager;
};
