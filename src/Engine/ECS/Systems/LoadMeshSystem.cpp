#include "LoadMeshSystem.h"

#include <tiny_obj_loader.h>

#include "../ECSManager.h"
#include "../../Render/Material.h"
#include "../../Render/Vertex.h"
#include "../Components/RenderComponent.h"
#include "../../MaterialManager.h"

void LoadMeshSystem::Load()
{
    int indexCount = 0;
    int textureCount = 0;
	for (auto const& entity : entities) {
		auto& render = ecsManager.GetComponent<RenderComponent>(entity);
        LoadModel(render, indexCount, textureCount);
	}
}

void LoadMeshSystem::LoadModel(RenderComponent& render, int& indexCount, int& textureCount) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> modelMaterials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &modelMaterials, &warn, &err, render.modelPath.c_str(), render.modelDir.c_str())) {
        throw std::runtime_error(warn + err);
    }

    if (modelMaterials.empty()) {
        Material material{};
        material.texturePath = render.texturesDir;
        render.materials.push_back(MaterialManager::Instance()->AddMaterial(material));
    }
    else {
        for (const auto& material : modelMaterials) {
            Material newMaterial{};
            newMaterial.texturePath = render.texturesDir + material.diffuse_texname;
            render.materials.push_back(MaterialManager::Instance()->AddMaterial(newMaterial));
        }
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};
            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            int matId = shape.mesh.material_ids[indexCount / 3];
            if (matId < 0) {
                matId = 0;
            }
            vertex.textureIndex = textureCount + matId;

            vertex.color = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(render.vertices.size());
                render.vertices.push_back(vertex);
            }

            render.indices.push_back(uniqueVertices[vertex]);
            indexCount++;
        }
    }
}