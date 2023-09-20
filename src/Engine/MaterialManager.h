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

	void LoadMaterials();

	uint32_t AddMaterial(const std::string& id, const Material& material);

	Material GetMaterial(uint32_t val); //TODO: do not return by value
	std::vector<Material>& GetMaterials() { return materials; };


	uint32_t GetMaterialCount() const { return materials.size(); }

private:
	std::unordered_map<uint32_t, uint32_t> materialsByHandle;
	std::unordered_map<std::string, uint32_t> materialHandlesById;
	std::vector<Material> materials;
	uint32_t nextId = 0;
};

