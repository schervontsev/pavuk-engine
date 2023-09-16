#pragma once
#include <queue>
#include <vector>
#include "Component.h"
#include "System.h"

using Entity = std::uint32_t;
const Entity MAX_ENTITIES = 5000;
using ComponentType = std::uint8_t;
const ComponentType MAX_COMPONENTS = 32;
using Signature = std::bitset<MAX_COMPONENTS>;

class EntityManager
{
public:
	EntityManager();

	Entity CreateEntity();
	void DestroyEntity(Entity entity);
	void SetSignature(Entity entity, Signature signature);
	Signature GetSignature(Entity entity);
private:
	std::queue<Entity> entities;

	std::array<Signature, MAX_ENTITIES> signatures;

	uint32_t entityCount;

};

