#include "MaterialManager.h"

#include <fstream>
#include "../Utils/json.hpp"

MaterialManager* MaterialManager::_instance(nullptr);
std::mutex MaterialManager::_mutex;

using giri::json::JSON;

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
    std::ifstream ifs("resources/materials.json");
    const std::string content((std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>()));
    JSON materialsJson = JSON::Load(content);
    for (auto& el : materialsJson.ArrayRange()) {
        Material material;
        material.texturePath = el["path"].ToString();
        AddMaterial(el["id"].ToString(), material);
    }
}

uint32_t MaterialManager::AddMaterial(const std::string& id, const Material& material)
{
    materialsByHandle[nextId] = materials.size();
    materials.push_back(material);
    materials.back().gpuNumber = nextId;
    materialHandlesById[id] = nextId;
    nextId++;
    return nextId;
}

uint32_t MaterialManager::GetMaterialHandle(const std::string& id)
{
    return materialHandlesById[id];
}

Material MaterialManager::GetMaterial(uint32_t val)
{
    return materials[materialsByHandle[val]];
}

void MaterialManager::DestroyMaterials(vk::UniqueDevice& device)
{
    for (const auto& mat : materials) {
        device->destroyImageView(mat.textureImageView, nullptr);
        device->destroyImage(mat.textureImage, nullptr);
        device->freeMemory(mat.textureImageMemory);
    }
    materials.clear();
}
