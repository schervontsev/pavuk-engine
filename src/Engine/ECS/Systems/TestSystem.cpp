#include "TestSystem.h"

#include <glm/gtx/quaternion.hpp>

#include "../Components/TransformComponent.h"
#include "../Components/GirlComponent.h"
#include "../Components/RotateComponent.h"
#include "../Components/RenderComponent.h"
#include "../ECSManager.h"
#include "../../Scene/Scene.h"

void TestSystem::Update(float dt, Scene* scene)
{
	for (auto const& entity : entities) {
		auto& transform = ecsManager.GetComponent<TransformComponent>(entity);
		auto& girl = ecsManager.GetComponent<GirlComponent>(entity);
		auto& render = ecsManager.GetComponent<RenderComponent>(entity);

		girl.timer += dt;
		const bool wasVisible = render.isVisible;
		if (render.isVisible && girl.timer > girl.hideTimer && girl.timer < girl.showTimer) {
			render.isVisible = false;
		} else if (!render.isVisible && girl.timer > girl.showTimer) {
			render.isVisible = true;
		}
		if (wasVisible != render.isVisible) {
			scene->SetDirty(true);
		}
		
	}
}
