#include "SimpleControllerSystem.h"
#include "../Components/TransformComponent.h"
#include "../Components/InputComponent.h"
#include "../ECSManager.h"

void SimpleControllerSystem::Update(float dt)
{
	for (auto const& entity : entities) {
		auto& transform = ecsManager.GetComponent<TransformComponent>(entity);
		auto& input = ecsManager.GetComponent<InputComponent>(entity);

		auto matrix = transform.transform;
		const glm::vec3 forward = transform.GetForwardVector();
		const glm::vec3 worldUp = glm::vec3(0.f, -1.f, 0.f);
		const glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
		auto up = glm::vec3(matrix[1][0], matrix[1][1], matrix[1][2]);

		if (input.currentInput.test((int)Input::Action::StepForward)) {
			transform.translation += forward * dt * 1.f;
		}
		if (input.currentInput.test((int)Input::Action::StepBackward)) {
			transform.translation -= forward * dt * 1.f;
		}
		if (input.currentInput.test((int)Input::Action::RotateLeft)) {
			transform.AddEulerAngle(-glm::vec3(0.f, 1.f, 0.f) * dt * 1.f);
		}
		if (input.currentInput.test((int)Input::Action::RotateRight)) {
			transform.AddEulerAngle(glm::vec3(0.f, 1.f, 0.f) * dt * 1.f);
		}
		if (input.currentInput.test((int)Input::Action::PitchUp)) {
			transform.AddEulerAngle(glm::vec3(-1.f, 0.f, 0.f) * dt * 1.f);
		}
		if (input.currentInput.test((int)Input::Action::PitchDown)) {
			transform.AddEulerAngle(glm::vec3(1.f, 0.f, 0.f) * dt * 1.f);
		}
		if (input.currentInput.test((int)Input::Action::StrafeLeft)) {
			transform.translation -= right * dt * 1.f;
		}
		if (input.currentInput.test((int)Input::Action::StrafeRight)) {
			transform.translation += right * dt * 1.f;
		}
		if (input.currentInput.test((int)Input::Action::Ascend)) {
			transform.translation += glm::vec3(0.f, 1.f, 0.f) * dt * 1.f;
		}
		if (input.currentInput.test((int)Input::Action::Descend)) {
			transform.translation += glm::vec3(0.f, -1.f, 0.f) * dt * 1.f;
		}
	}

}