#include "ComponentMap.h"

#include <cassert>

template <typename T>
void ComponentArray<T>::InsertData(Entity entity, T component)
{
	assert(entityToIndexMap.find(entity) == entityToIndexMap.end() && "Component added to same entity more than once.");

	// Put new entry at end and update the maps
	size_t newIndex = componentsSize;
	entityToIndexMap[entity] = newIndex;
	indexToEntityMap[newIndex] = entity;
	componentArray[newIndex] = component;
	++componentsSize;
}

template<typename T>
void ComponentArray<T>::RemoveData(Entity entity)
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

template<typename T>
T& ComponentArray<T>::GetData(Entity entity)
{
	assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Retrieving non-existent component.");

	// Return a reference to the entity's component
	return componentArray[entityToIndexMap[entity]];
}

template<typename T>
void ComponentArray<T>::OnEntityDestroyed(Entity entity)
{
	if (entityToIndexMap.find(entity) != entityToIndexMap.end()) {
		// Remove the entity's component if it existed
		RemoveData(entity);
	}

}
