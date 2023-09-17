#pragma once
#include "EntityManager.h"
#include "ComponentManager.h"
#include "SystemManager.h"

class ECSManager
{
public:
	void Init();

	//entities
	Entity CreateEntity();
	void DestroyEntity(Entity entity);

	//components
	template<typename T> void RegisterComponent();
	template<typename T> void AddComponent(Entity entity, T component);
	template<typename T> void RemoveComponent(Entity entity);
	template<typename T> T& GetComponent(Entity entity);
	template<typename T> ComponentType GetComponentType();

	// systems
	template<typename T> std::shared_ptr<T> RegisterSystem();
	template<typename T> void SetSystemSignature(Signature signature);

private:
	std::unique_ptr<ComponentManager> componentManager;
	std::unique_ptr<EntityManager> entityManager;
	std::unique_ptr<SystemManager> systemManager;
};
