#include "InputManager.h"

#include <cassert>
#include <iostream>
#include <string>

namespace Input {
	void InputManager::Init(GLFWwindow* newWindow)
	{
		assert(newWindow);
		window = newWindow;

		//TODO: unhardcode bindings
		keyBinds[GLFW_KEY_W] = Action::StepForward;
		keyBinds[GLFW_KEY_S] = Action::StepBackward;

		glfwSetKeyCallback(window, GLFWkeyfun(keyCallback));

	}

	void InputManager::keyCallback(GLFWwindow*, int key, int, int, int)
	{
		const auto foundAction = keyBinds.find(key);
		if (foundAction != keyBinds.end()) {
			
		}
		std::cout << "key pressed: " + std::to_string(key);
	}
}
