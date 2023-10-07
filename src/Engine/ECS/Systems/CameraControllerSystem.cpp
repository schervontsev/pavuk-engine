#include "CameraControllerSystem.h"
#include "../Components/TransformComponent.h"
#include "../Components/InputComponent.h"
#include "../ECSManager.h"

void CameraControllerSystem::Update(float dt)
{
	for (auto const& entity : entities) {
		auto& transform = ecsManager.GetComponent<TransformComponent>(entity);
		auto& input = ecsManager.GetComponent<InputComponent>(entity);

		//const glm::mat4 inverted = glm::inverse(transform.transform);
		auto matrix = transform.transform;
		const glm::vec3 forward = transform.GetForwardVector();
		auto up = glm::vec3(matrix[1][0], matrix[1][1], matrix[1][2]);

		if (input.currentInput.test((int)Input::Action::StepForward)) {
			transform.translation += forward * dt * 1.f;
		}else if (input.currentInput.test((int)Input::Action::StepBackward)) {
			transform.translation -= forward * dt * 1.f;
		}
	}

}