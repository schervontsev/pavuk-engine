#include "ECSManager.h"

ECSManager ecsManager;

void ECSManager::Init()
{
}

Entity ECSManager::CreateEntity()
{
	return Entity();
}

void ECSManager::DestroyEntity(Entity entity)
{
	entityManager->DestroyEntity(entity);
	componentManager->OnEntityDestroyed(entity);
	systemManager->OnEntityDestroyed(entity);

}

template<typename T> void ECSManager::RegisterComponent()
{
	componentManager->RegisterComponent<T>();
}

template<typename T> void ECSManager::AddComponent(Entity entity, T component)
{
	componentManager->AddComponent<T>(entity, component);

	auto signature = entityManager->GetSignature(entity);
	signature.set(componentManager->GetComponentType<T>(), true);
	entityManager->SetSignature(entity, signature);

	systemManager->OnEntitySignatureChanged(entity, signature);

}

template<typename T> void ECSManager::RemoveComponent(Entity entity)
{
	componentManager->RemoveComponent<T>(entity);

	auto signature = entityManager->GetSignature(entity);
	signature.set(componentManager->GetComponentType<T>(), false);
	entityManager->SetSignature(entity, signature);

	systemManager->OnEntitySignatureChanged(entity, signature);
}

template<typename T> T& ECSManager::GetComponent(Entity entity)
{
	return componentManager->GetComponent<T>(entity);
}

template<typename T> ComponentType ECSManager::GetComponentType()
{
	return componentManager->GetComponentType<T>();
}

template<typename T> std::shared_ptr<T> ECSManager::RegisterSystem()
{
	return systemManager->RegisterSystem<T>();
}

template<typename T> void ECSManager::SetSystemSignature(Signature signature)
{
	systemManager->SetSignature<T>(signature);
}