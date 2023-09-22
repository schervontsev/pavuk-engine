#include "ECSManager.h"

ECSManager ecsManager;

void ECSManager::Init()
{
	componentManager = std::make_unique<ComponentManager>();
	entityManager = std::make_unique<EntityManager>();
	systemManager = std::make_unique<SystemManager>();

}

Entity ECSManager::CreateEntity()
{
	return entityManager->CreateEntity();
}

void ECSManager::DestroyEntity(Entity entity)
{
	entityManager->DestroyEntity(entity);
	componentManager->OnEntityDestroyed(entity);
	systemManager->OnEntityDestroyed(entity);

}

