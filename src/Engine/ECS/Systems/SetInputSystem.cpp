#include "SetInputSystem.h"
#include "../ECSManager.h"
#include "../Components/InputComponent.h"
#include "../../Input/InputManager.h"

void SetInputSystem::SetInput()
{
	for (auto const& entity : entities) {
		auto& input = ecsManager.GetComponent<InputComponent>(entity);
		input.currentInput = InputManager::Instance()->GetInputBitmask();
	}
}
