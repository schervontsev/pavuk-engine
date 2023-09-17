#pragma once
#include "EntityManager.h"
#include "ComponentManager.h"
#include "SystemManager.h"
class ECSManager;
extern ECSManager ecsManager;

class ECSManager
{
public:
	void Init();

	//entities
	Entity CreateEntity();
	void DestroyEntity(Entity entity);

	//components
	template<typename T> void RegisterComponent()
	{
		componentManager->RegisterComponent<T>();
	}

	template<typename T> void AddComponent(Entity entity, T component)
	{
		componentManager->AddComponent<T>(entity, component);

		auto signature = entityManager->GetSignature(entity);
		signature.set(componentManager->GetComponentType<T>(), true);
		entityManager->SetSignature(entity, signature);

		systemManager->OnEntitySignatureChanged(entity, signature);

	}

	template<typename T> void RemoveComponent(Entity entity)
	{
		componentManager->RemoveComponent<T>(entity);

		auto signature = entityManager->GetSignature(entity);
		signature.set(componentManager->GetComponentType<T>(), false);
		entityManager->SetSignature(entity, signature);

		systemManager->OnEntitySignatureChanged(entity, signature);
	}

	template<typename T> T& GetComponent(Entity entity)
	{
		return componentManager->GetComponent<T>(entity);
	}

	template<typename T> ComponentType GetComponentType()
	{
		return componentManager->GetComponentType<T>();
	}

	// systems
	template<typename T> std::shared_ptr<T> RegisterSystem()
	{
		return systemManager->RegisterSystem<T>();
	}
	template<typename T> void SetSystemSignature(Signature signature)
	{
		systemManager->SetSignature<T>(signature);
	}

private:
	std::unique_ptr<ComponentManager> componentManager;
	std::unique_ptr<EntityManager> entityManager;
	std::unique_ptr<SystemManager> systemManager;
};
