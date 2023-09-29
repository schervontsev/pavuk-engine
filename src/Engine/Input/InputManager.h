#pragma once
#include <memory>
#include <GLFW/glfw3.h>

class InputManager
{
public:
	void Init(GLFWwindow* newWindow);

	static void keyCallback(GLFWwindow*, int, int, int);
private:
	GLFWwindow* window = nullptr; //weak_ptr or shared_ptr isn't really needed here
};

