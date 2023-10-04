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

void InputManager::Init(GLFWwindow* newWindow)
{
	assert(newWindow);
	//window = newWindow;

	//TODO: unhardcode bindings
	keyBinds[GLFW_KEY_W] = Input::Action::StepForward;
	keyBinds[GLFW_KEY_S] = Input::Action::StepBackward;

	glfwSetKeyCallback(newWindow, GLFWkeyfun(keyCallback));

}

void InputManager::keyCallback(GLFWwindow*, int key, int, int, int)
{
	const auto foundAction = Instance()->keyBinds.find(key);
	if (foundAction != Instance()->keyBinds.end()) {
		Instance()->inputBitmask.set(static_cast<int>(foundAction->second));
		//Instance()->currentInputs.push_back(foundAction->second);
	}
	std::cout << "key pressed: " + std::to_string(key);
}

bool InputManager::InputPressed(Input::Action action) const
{
	return inputBitmask.test(static_cast<int>(action));
}
