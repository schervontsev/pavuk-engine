#pragma once
#include <mutex>

#include "Render/Material.h"
class MaterialManager
{
private:
	static MaterialManager* _instance;
	static std::mutex _mutex;
protected:
	MaterialManager() = default;
public:
	MaterialManager(MaterialManager& other) = delete;
	void operator=(const MaterialManager&) = delete;

	static MaterialManager* Instance();

	uint32_t AddMaterial(const Material& material);
	Material GetMaterial(uint32_t val); //TODO: do not return by value

	uint32_t GetMaterialCount() const { return materials.size(); }

private:
	std::unordered_map<uint32_t, Material> materials;
	uint32_t lastId = 0;
};

