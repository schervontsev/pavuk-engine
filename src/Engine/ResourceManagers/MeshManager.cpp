#include "MeshManager.h"

#include <filesystem>

#include <fstream>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "../Render/Vertex.h"
#include "MaterialManager.h"
#include "../../../include/json.hpp"

MeshManager* MeshManager::_instance(nullptr);
std::mutex MeshManager::_mutex;

using giri::json::JSON;

MeshManager* MeshManager::Instance()
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (!_instance) {
        _instance = new MeshManager();
    }
    return _instance;
}

void MeshManager::LoadMeshes()
{
    std::ifstream ifs("resources/data/meshes.json");
    const std::string content((std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>()));
    JSON materialsJson = JSON::Load(content);
    for (auto& el : materialsJson.ArrayRange()) {
        Mesh mesh;
        auto modelPath = "resources/" + el["path"].ToString();
        std::vector<uint32_t> materials;
        for (const auto& mat : el["materials"].ArrayRange()) {
            const auto materialHandle = MaterialManager::Instance()->GetMaterialHandle(mat.ToString());
            const auto material = MaterialManager::Instance()->GetMaterial(materialHandle);
            materials.push_back(material.gpuNumber);
        }
        LoadModel(mesh, modelPath, materials);
        meshesByHandle[nextId] = meshes.size();
        meshHandlesById[el["id"].ToString()] = nextId;
        meshes.push_back(std::make_shared<Mesh>(mesh));
        nextId++;
    }
}

void MeshManager::LoadModel(Mesh& mesh, const std::string& path, const std::vector<uint32_t>& materials) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> modelMaterials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &modelMaterials, &warn, &err, path.c_str(), std::filesystem::path{path}.parent_path().string().c_str())) {
        throw std::runtime_error(warn + err);
    }
    int indexCount = 0;
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                -attrib.vertices[3 * index.vertex_index + 1], //need to negate to convert from opengl format
                attrib.vertices[3 * index.vertex_index + 2]
            };
            vertex.normal = {
                attrib.normals[3 * index.normal_index + 0],
                -attrib.normals[3 * index.normal_index + 1], //need to negate to convert from opengl format
                attrib.normals[3 * index.normal_index + 2],
            };
            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]  //need to negate to convert from opengl format
            };

            int matId = shape.mesh.material_ids[indexCount / 3];

            vertex.textureIndex = materials[std::max(0, matId)];

            vertex.color = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
                mesh.vertices.push_back(vertex);
            }

            mesh.indices.push_back(uniqueVertices[vertex]);
            indexCount++;
        }
    }
}