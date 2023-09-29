#include "InputManager.h"

#include <cassert>
#include <iostream>

void InputManager::Init(GLFWwindow* newWindow)
{
	assert(newWindow);
	window = newWindow;
	glfwSetKeyCallback(window, GLFWkeyfun(keyCallback));
}

void InputManager::keyCallback(GLFWwindow*, int, int, int)
{
	std::cout << "key pressed";
}
