#include "LightControllerSystem.h"
#include "../Components/TransformComponent.h"
#include "../ECSManager.h"
#include "../../Input/InputManager.h"

void LightControllerSystem::Update(float dt)
{
	const float speed = 2.f;
	const InputManager* input = InputManager::Instance();

	for (auto const& entity : entities) {
		auto& transform = ecsManager.GetComponent<TransformComponent>(entity);

		if (input->InputPressed(Input::Action::LightStepForward))
			transform.translation.z += speed * dt;
		if (input->InputPressed(Input::Action::LightStepBackward))
			transform.translation.z -= speed * dt;
		if (input->InputPressed(Input::Action::LightStrafeLeft))
			transform.translation.x -= speed * dt;
		if (input->InputPressed(Input::Action::LightStrafeRight))
			transform.translation.x += speed * dt;
		if (input->InputPressed(Input::Action::LightAscend))
			transform.translation.y += speed * dt;
		if (input->InputPressed(Input::Action::LightDescend))
			transform.translation.y -= speed * dt;
	}
}
