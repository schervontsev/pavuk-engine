#pragma once
#include <list>
#include "Entity.h"
#include <vector>
#include "Component.h"
#include "System.h"
class EntityManager
{
	std::list<Entity> entities;
	//std::vector<Component> components;
	std::vector<System> systems;
};

