#pragma once
#include <vector>
#include "../../Input/InputManager.h"

namespace Input
{
	enum class Action;
}

struct InputComponent
{
	std::bitset<MAX_INPUTS> currentInput;
};
