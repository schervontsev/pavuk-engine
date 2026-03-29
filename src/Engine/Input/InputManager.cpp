#include "InputManager.h"

#include <cassert>
#include <iostream>
#include <string>

InputManager* InputManager::_instance(nullptr);
std::mutex InputManager::_mutex;

InputManager* InputManager::Instance()
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (!_instance) {
		_instance = new InputManager();
	}
	return _instance;
}

void InputManager::ClearInput()
{
	inputBitmask.reset();
}

void InputManager::Init(GLFWwindow* newWindow)
{
	assert(newWindow);
	//window = newWindow;

	//TODO: unhardcode bindings
	keyBinds[GLFW_KEY_W] = Input::Action::StepForward;
	keyBinds[GLFW_KEY_S] = Input::Action::StepBackward;
	keyBinds[GLFW_KEY_LEFT] = Input::Action::RotateLeft;
	keyBinds[GLFW_KEY_RIGHT] = Input::Action::RotateRight;
	keyBinds[GLFW_KEY_UP] = Input::Action::PitchUp;
	keyBinds[GLFW_KEY_DOWN] = Input::Action::PitchDown;
	keyBinds[GLFW_KEY_A] = Input::Action::StrafeLeft;
	keyBinds[GLFW_KEY_D] = Input::Action::StrafeRight;
	keyBinds[GLFW_KEY_E] = Input::Action::Ascend;
	keyBinds[GLFW_KEY_Q] = Input::Action::Descend;
	// Light (world axes): I/K = Z, J/L = X, U/O = Y
	keyBinds[GLFW_KEY_I] = Input::Action::LightStepForward;
	keyBinds[GLFW_KEY_K] = Input::Action::LightStepBackward;
	keyBinds[GLFW_KEY_J] = Input::Action::LightStrafeLeft;
	keyBinds[GLFW_KEY_L] = Input::Action::LightStrafeRight;
	keyBinds[GLFW_KEY_U] = Input::Action::LightAscend;
	keyBinds[GLFW_KEY_O] = Input::Action::LightDescend;
	keyBinds[GLFW_KEY_F3] = Input::Action::ToggleNormalView;
	// Digits 1-6: shadow cubemap face toggles are read via glfwGetKey in Renderer::Update (avoids sharing inputBitmask with movement).

	glfwSetKeyCallback(newWindow, GLFWkeyfun(keyCallback));

}

void InputManager::keyCallback(GLFWwindow*, int key, int scancode, int action, int mods)
{
	const auto foundAction = Instance()->keyBinds.find(key);
	if (foundAction != Instance()->keyBinds.end()) {
		Instance()->inputBitmask.set(static_cast<int>(foundAction->second), action != GLFW_RELEASE);
		std::cout << "key " + std::to_string(action) + ": " + std::to_string(key) + "\n";
	}
	
}

bool InputManager::InputPressed(Input::Action action) const
{
	return inputBitmask.test(static_cast<int>(action));
}
