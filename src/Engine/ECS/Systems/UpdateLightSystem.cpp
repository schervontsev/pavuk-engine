#include "UpdateLightSystem.h"
#include "../../Render/UniformBufferObject.h"
#include "../Components/TransformComponent.h"
#include "../Components/PointLightComponent.h"
#include "../ECSManager.h"

void UpdateLightSystem::UpdateLightInUBO(FragmentUniformBufferObject& ubo)
{
	const size_t maxLight = 32;
	size_t curLight = 0;
	for (auto const& entity : entities) {
		auto& transform = ecsManager.GetComponent<TransformComponent>(entity);
		auto& light = ecsManager.GetComponent<PointLightComponent>(entity);
		ubo.lights[curLight].light_pos = glm::vec4(transform.translation, 0.f);
		ubo.lights[curLight].light_col = glm::vec4(light.color);
		curLight++;
		if (curLight >= maxLight) {
			assert(false);
			return;
		}
	}
}