#include "UpdateTransformSystem.h"
#include "../Components/TransformComponent.h"
#include <glm/gtx/quaternion.hpp>
#include "../ECSManager.h"

void UpdateTransformSystem::UpdateTransform()
{
	for (auto const& entity : entities) {
		auto& transform = ecsManager.GetComponent<TransformComponent>(entity);

		transform.transform = glm::translate(glm::mat4(1.0), transform.translation);
		transform.transform = transform.transform * toMat4(transform.rotation);
		transform.transform = glm::scale(transform.transform, transform.scale);
	}
}
