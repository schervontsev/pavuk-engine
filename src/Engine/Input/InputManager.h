#pragma once
#include <bitset>
#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include <GLFW/glfw3.h>

#define MAX_INPUTS 4

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
}

class InputManager
{
private:
	static InputManager* _instance;
	static std::mutex _mutex;

protected:
	InputManager() = default;
public:
	InputManager(InputManager& other) = delete;
	void operator=(const InputManager&) = delete;

	static InputManager* Instance();

	void Init(GLFWwindow* newWindow);

	static void keyCallback(GLFWwindow*, int, int, int, int);
	
	std::bitset<MAX_INPUTS> GetInputBitmask() const { return inputBitmask; }

	bool InputPressed(Input::Action action) const;

protected:
	std::map<int, Input::Action> keyBinds;
	std::vector<Input::Action> currentInputs;
	std::bitset<MAX_INPUTS> inputBitmask;
};
