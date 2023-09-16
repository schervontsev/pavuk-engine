#pragma once
#include <memory>
#include <unordered_map>

#include "ComponentMap.h"
#include "EntityManager.h"

class ComponentManager
{
public:
	template<typename T>
	void RegisterComponent();

	template<typename T>
	ComponentType GetComponentType();

	template<typename T>
	void AddComponent(Entity entity, T component);

	template<typename T>
	void RemoveComponent(Entity entity);

	template<typename T>
	T& GetComponent(Entity entity);

	void OnEntityDestroyed(Entity entity);

private:
	// Map from type string pointer to a component type
	std::unordered_map<const char*, ComponentType> componentTypes;

	// Map from type string pointer to a component array
	std::unordered_map<const char*, std::shared_ptr<IComponentArray>> componentArrays;

	// The component type to be assigned to the next registered component - starting at 0
	ComponentType nextType = 0;

	// Convenience function to get the statically casted pointer to the ComponentArray of type T.
	template<typename T>
	std::shared_ptr<ComponentArray<T>> GetComponentArray();
};
