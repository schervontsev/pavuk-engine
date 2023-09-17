#pragma once
#include <cassert>

#include "EntityManager.h"
#include <unordered_map>

class IComponentArray
{
public:
	virtual ~IComponentArray() = default;
	virtual void OnEntityDestroyed(Entity entity) = 0;
};


template<typename T>
class ComponentArray : public IComponentArray
{
public:
	void InsertData(Entity entity, T component)
	{
		assert(entityToIndexMap.find(entity) == entityToIndexMap.end() && "Component added to same entity more than once.");

		// Put new entry at end and update the maps
		size_t newIndex = componentsSize;
		entityToIndexMap[entity] = newIndex;
		indexToEntityMap[newIndex] = entity;
		componentArray[newIndex] = component;
		++componentsSize;
	}
	
	void RemoveData(Entity entity)
	{
		assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Removing non-existent component.");

		// Copy element at end into deleted element's place to maintain density
		size_t indexOfRemovedEntity = entityToIndexMap[entity];
		size_t indexOfLastElement = componentsSize - 1;
		componentArray[indexOfRemovedEntity] = componentArray[indexOfLastElement];

		// Update map to point to moved spot
		Entity entityOfLastElement = indexToEntityMap[indexOfLastElement];
		entityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
		indexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

		entityToIndexMap.erase(entity);
		indexToEntityMap.erase(indexOfLastElement);

		--componentsSize;
	}

	T& GetData(Entity entity)
	{
		assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Retrieving non-existent component.");

		// Return a reference to the entity's component
		return componentArray[entityToIndexMap[entity]];
	}

	void OnEntityDestroyed(Entity entity) override
	{
		if (entityToIndexMap.find(entity) != entityToIndexMap.end()) {
			// Remove the entity's component if it existed
			RemoveData(entity);
		}

	}

private:
	// The packed array of components (of generic type T),
	// set to a specified maximum amount, matching the maximum number
	// of entities allowed to exist simultaneously, so that each entity
	// has a unique spot.
	std::array<T, MAX_ENTITIES> componentArray;

	// Map from an entity ID to an array index.
	std::unordered_map<Entity, size_t> entityToIndexMap;

	// Map from an array index to an entity ID.
	std::unordered_map<size_t, Entity> indexToEntityMap;

	// Total size of valid entries in the array.
	size_t componentsSize = 0;
};