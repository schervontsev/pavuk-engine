#include "RenderSystem.h"
#include "../ECSManager.h"
#include "../Components/TransformComponent.h"
#include "../Components/RenderComponent.h"

void RenderSystem::Update(float dt)
{
	for (auto const& entity : entities) {
		auto& transform = ecsManager.GetComponent<TransformComponent>(entity);
		auto& render = ecsManager.GetComponent<RenderComponent>(entity);

		render.pushConstants.transform = transform.transform;

	}

}
