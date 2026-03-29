#include "MeshManager.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <utility>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

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

        const bool flipNormals = (el["id"].ToString() == "maphome");
        LoadModel(mesh, modelPath, materials, flipNormals);
        meshesByHandle[nextId] = meshes.size();
        meshHandlesById[el["id"].ToString()] = nextId;
        meshes.push_back(std::make_shared<Mesh>(mesh));
        nextId++;
    }
}

void MeshManager::LoadModel(Mesh& mesh, const std::string& path, const std::vector<uint32_t>& materials, bool flipNormals) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> modelMaterials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &modelMaterials, &warn, &err, path.c_str(), std::filesystem::path{path}.parent_path().string().c_str())) {
        throw std::runtime_error(warn + err);
    }
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for (const auto& shape : shapes) {
        int shapeVertexIndex = 0;
        const size_t numIndices = shape.mesh.indices.size();
        const size_t numTris = numIndices / 3;
        const size_t numMatIds = shape.mesh.material_ids.size();
        std::cerr << "[MeshManager] LoadModel: \"" << path << "\" shape=\"" << shape.name << "\""
                  << " indices=" << numIndices << " tris=" << numTris << " material_ids=" << numMatIds << "\n";
        if (numMatIds != numTris && numMatIds != 0) {
            std::cerr << "[MeshManager] WARNING: material_ids count (" << numMatIds
                      << ") != triangle count (" << numTris << ") — will clamp access\n";
        }
        if (numMatIds == 0) {
            std::cerr << "[MeshManager] WARNING: material_ids is empty — every face will use matId=0\n";
        }

        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                -attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };
            vertex.normal = {
                attrib.normals[3 * index.normal_index + 0],
                -attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2],
            };
            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            const size_t triIndex = static_cast<size_t>(shapeVertexIndex / 3);
            int matId = 0;
            if (shape.mesh.material_ids.empty()) {
                matId = 0;
            } else if (triIndex >= shape.mesh.material_ids.size()) {
                std::cerr << "[MeshManager] ERROR: triIndex=" << triIndex << " >= material_ids.size()="
                          << shape.mesh.material_ids.size() << " path=\"" << path << "\" shape=\"" << shape.name
                          << "\" — clamping to last id\n";
                matId = shape.mesh.material_ids.back();
            } else {
                matId = shape.mesh.material_ids[triIndex];
            }

            if (materials.empty()) {
                vertex.textureIndex = 0;
                std::cerr << "[MeshManager] WARNING: materials[] empty for \"" << path << "\" — textureIndex=0\n";
            } else {
                int slot = matId;
                if (slot < 0 || static_cast<size_t>(slot) >= materials.size()) {
                    slot = slot < 0 ? 0 : static_cast<int>(materials.size() - 1);
                }
                vertex.textureIndex = materials[static_cast<size_t>(slot)];
            }

            vertex.color = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
                mesh.vertices.push_back(vertex);
            }

            mesh.indices.push_back(uniqueVertices[vertex]);
            shapeVertexIndex++;
        }
    }

    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
        std::swap(mesh.indices[i + 1], mesh.indices[i + 2]);

    if (flipNormals) {
        for (auto& v : mesh.vertices)
            v.normal = -v.normal;
    }
}