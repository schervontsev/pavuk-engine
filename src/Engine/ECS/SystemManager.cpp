#include "SystemManager.h"

void SystemManager::OnEntityDestroyed(Entity entity)
{
	// Erase a destroyed entity from all system lists
	// mEntities is a set so no check needed
	for (auto const& pair : systems) {
		auto const& system = pair.second;

		system->entities.erase(entity);
	}
}

void SystemManager::OnEntitySignatureChanged(Entity entity, Signature entitySignature)
{
	// Notify each system that an entity's signature changed
	for (auto const& pair : systems) {
		auto const& type = pair.first;
		auto const& system = pair.second;
		auto const& systemSignature = signatures[type];

		// Entity signature matches system signature - insert into set
		if ((entitySignature & systemSignature) == systemSignature) {
			system->entities.insert(entity);
		}
		// Entity signature does not match system signature - erase from set
		else {
			system->entities.erase(entity);
		}
	}
}
