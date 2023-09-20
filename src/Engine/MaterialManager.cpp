#include "MaterialManager.h"
#include "Utils/json.hpp"
#include <fstream>

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

void MaterialManager::LoadMaterials()
{
    std::ifstream t("resources/materials.json");
    std::stringstream buffer;
    buffer << t.rdbuf();
    json::JSON materialsJson = json::JSON::Load(buffer.str());
    for (auto& el : materialsJson.ArrayRange()) {
        Material material;
        material.texturePath = el["path"].ToString();

    }
}

uint32_t MaterialManager::AddMaterial(const std::string& id, const Material& material)
{
    materials.push_back(material);
    materialsByHandle[nextId] = materials.size();
    materialHandlesById[id] = nextId;
    nextId++;
    return nextId;
}

Material MaterialManager::GetMaterial(uint32_t val)
{
    return materials[materialsByHandle[val]];
}
