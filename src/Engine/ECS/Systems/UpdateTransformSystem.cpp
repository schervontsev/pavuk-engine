#include "UpdateTransformSystem.h"
#include "../Components/TransformComponent.h"
#include "../ECSManager.h"
#include <glm/gtx/quaternion.hpp>

void UpdateTransformSystem::Update(float dt)
{
	for (auto const& entity : entities) {
		auto& transform = ecsManager.GetComponent<TransformComponent>(entity);

		transform.transform = glm::scale(glm::mat4(1.0), transform.scale);
		transform.transform = glm::translate(transform.transform, transform.translation);
		transform.transform = transform.transform * toMat4(transform.rotation);
	}
}
