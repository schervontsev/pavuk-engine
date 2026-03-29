#include "ShadowSystem.h"
#include "../../Render/UniformBufferObject.h"
#include <unordered_map>
#include "../Components/TransformComponent.h"
#include "../ECSManager.h"

void ShadowSystem::AssignShadowCubeIndices(FragmentUniformBufferObject& ubo,
    const std::vector<Entity>& orderedLightEntities) const
{
    for (size_t i = 0; i < 32; ++i) {
        ubo.lights[i].shadow_cube_index = -1;
    }

    std::unordered_map<Entity, int32_t> entityToCube;
    int32_t cube = 0;
    for (Entity e : entities) {
        if (cube >= MAX_SHADOW_LIGHTS) {
            break;
        }
        entityToCube[e] = cube++;
    }

    for (size_t i = 0; i < orderedLightEntities.size() && i < 32; ++i) {
        auto it = entityToCube.find(orderedLightEntities[i]);
        if (it != entityToCube.end()) {
            ubo.lights[i].shadow_cube_index = it->second;
        }
    }
}

glm::vec3 ShadowSystem::GetShadowLightPosition(size_t cubeIndex) const
{
    size_t i = 0;
    for (Entity e : entities) {
        if (i == cubeIndex) {
            return ecsManager.GetComponent<TransformComponent>(e).translation;
        }
        ++i;
    }
    return glm::vec3(0.f);
}
