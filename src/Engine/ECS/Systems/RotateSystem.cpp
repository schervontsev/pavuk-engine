#include "RotateSystem.h"
#include "../Components/RotateComponent.h"
#include "../Components/TransformComponent.h"
#include "../ECSManager.h"

void RotateSystem::Update(float dt)
{
	for (auto const& entity : entities) {
		auto& transform = ecsManager.GetComponent<TransformComponent>(entity);
		auto& rotate = ecsManager.GetComponent<RotateComponent>(entity);
		transform.AddEulerAngle(rotate.unitRotation * rotate.speed * dt);
	}
}
