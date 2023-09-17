#pragma once
#include <set>

#include "../EntityManager.h"

class System
{
public:
	std::set<Entity> entities;
};
