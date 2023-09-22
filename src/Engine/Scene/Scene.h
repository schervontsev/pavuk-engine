#pragma once
#include <vector>

struct Material;

class Scene
{
public:
    Scene();
    void Init();

public:
    void Update(float dt);
    void SetDirty(bool val) { isDirty = val; };
    bool IsDirty() { return isDirty; };

private:
    bool isDirty = true;
    double timeFromStart = 0.f;
};
