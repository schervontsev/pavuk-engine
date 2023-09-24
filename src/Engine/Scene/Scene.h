#pragma once
#include <vector>

#include "../ECS/EntityManager.h"

struct Material;

class Scene
{
public:
    Scene();
    void Init();
    void Update(float dt);

    void SetDirty(bool val) { isDirty = val; };
    bool IsDirty() { return isDirty; };

    Entity GetMainCamera() { return mainCamera; };
private:
    void InitCamera();

private:
    bool isDirty = true;
    double timeFromStart = 0.f;
    Entity mainCamera;
};
