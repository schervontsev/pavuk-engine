#include "MaterialManager.h"

MaterialManager* MaterialManager::_instance(nullptr);
std::mutex MaterialManager::_mutex;

MaterialManager* MaterialManager::Instance()
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (!_instance) {
        _instance = new MaterialManager();
    }
    return _instance;
}

uint32_t MaterialManager::AddMaterial(const Material& material)
{
    const auto result = lastId;
    materials.push_back(material);
	materials[lastId++] = material;
    return result;
}

Material MaterialManager::GetMaterial(uint32_t val)
{
    return materials[materialsById[val]];
}
