#pragma once
#include <set>
#include "Component.h"
class System
{
	std::set<ComponentType> reqTypes;
public:
	virtual void Update(float dt);
};

