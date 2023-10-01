#pragma once
#include <map>
#include <memory>
#include <GLFW/glfw3.h>

namespace Input
{
	
	enum class Action
	{
		StepForward,
		StepBackward
	};

	enum class Axis
	{
		
	};

	class InputManager
	{
	public:
		void Init(GLFWwindow* newWindow);

		static void keyCallback(GLFWwindow*, int, int, int, int);
	private:
		GLFWwindow* window = nullptr; //smart pointer isn't really needed here

		static std::map<int, Action> keyBinds;
	};

}
