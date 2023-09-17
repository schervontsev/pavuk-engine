#pragma once
#include <cassert>
#include <memory>
#include <unordered_map>

#include "Systems/System.h"
class SystemManager
{
public:
	template <typename T>
	std::shared_ptr<T> RegisterSystem()
	{
		const char* typeName = typeid(T).name();

		assert(systems.find(typeName) == systems.end() && "Registering system more than once.");

		// Create a pointer to the system and return it so it can be used externally
		auto system = std::make_shared<T>();
		systems.insert({ typeName, system });
		return system;
	}

	template<typename T>
	void SetSignature(Signature signature)
	{
		const char* typeName = typeid(T).name();

		assert(systems.find(typeName) != systems.end() && "System used before registered.");

		// Set the signature for this system
		signatures.insert({ typeName, signature });
	}

	void OnEntityDestroyed(Entity entity);

	void OnEntitySignatureChanged(Entity entity, Signature entitySignature);

private:
	// Map from system type string pointer to a signature
	std::unordered_map<const char*, Signature> signatures;

	// Map from system type string pointer to a system pointer
	std::unordered_map<const char*, std::shared_ptr<System>> systems;

};

