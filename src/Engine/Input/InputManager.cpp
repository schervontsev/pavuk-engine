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
	keyBinds[GLFW_KEY_E] = Input::Action::Ascend;
	keyBinds[GLFW_KEY_Q] = Input::Action::Descend;

	glfwSetKeyCallback(newWindow, GLFWkeyfun(keyCallback));

}

void InputManager::keyCallback(GLFWwindow*, int key, int scancode, int action, int mods)
{
	const auto foundAction = Instance()->keyBinds.find(key);
	if (foundAction != Instance()->keyBinds.end()) {
		Instance()->inputBitmask.set(static_cast<int>(foundAction->second), action != GLFW_RELEASE);
		//Instance()->currentInputs.push_back(foundAction->second);
	}
	std::cout << "key pressed: " + std::to_string(key);
}

bool InputManager::InputPressed(Input::Action action) const
{
	return inputBitmask.test(static_cast<int>(action));
}
