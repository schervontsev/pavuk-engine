#include "Mesh.h"
#include "../Render/Material.h"
#include <glm/gtx/quaternion.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include "../Render/Vertex.h"

Mesh::Mesh()
{
}

void Mesh::loadModel(int textureCount, const std::string& modelPath, const std::string& modelDir, const std::string& texturePath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> modelMaterials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &modelMaterials, &warn, &err, modelPath.c_str(), modelDir.c_str())) {
        throw std::runtime_error(warn + err);
    }

    if (modelMaterials.empty()) {
        Material material{};
        material.texturePath = texturePath;
        materials.push_back(material);
    } else {
        for (const auto& material : modelMaterials) {
            Material newMaterial{};
            newMaterial.texturePath = texturePath + material.diffuse_texname;
            materials.push_back(newMaterial);
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
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
            indexCount++;
        }
    }
}

void Mesh::LoadCube() {
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    vertices.clear();
    indices.clear();

    {
        Vertex vertex{};
        vertex.pos = { 0.f, -0.5f, 0.f };
        vertex.color = { 1.0f, 0.0f, 0.0f };
        if (uniqueVertices.count(vertex) == 0) {
            uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
            vertices.push_back(vertex);
        }
        indices.push_back(uniqueVertices[vertex]);
    }
    {
        Vertex vertex{};
        vertex.pos = { 0.5f, 0.5f, 0.f };
        vertex.color = { 0.0f, 1.0f, 0.0f };
        if (uniqueVertices.count(vertex) == 0) {
            uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
            vertices.push_back(vertex);
        }
        indices.push_back(uniqueVertices[vertex]);
    }
    {
        Vertex vertex{};
        vertex.pos = { -0.5f, 0.5f, 0.f };
        vertex.color = { 0.0f, 0.0f, 1.0f };
        if (uniqueVertices.count(vertex) == 0) {
            uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
            vertices.push_back(vertex);
        }
        indices.push_back(uniqueVertices[vertex]);
    }
    {
        Vertex vertex{};
        vertex.pos = { 0.2f, -0.5f, 0.5f };
        vertex.color = { 1.0f, 0.0f, 0.0f };
        if (uniqueVertices.count(vertex) == 0) {
            uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
            vertices.push_back(vertex);
        }
        indices.push_back(uniqueVertices[vertex]);
    }
    {
        Vertex vertex{};
        vertex.pos = { 0.7f, 0.5f, 0.5f };
        vertex.color = { 0.0f, 1.0f, 0.0f };
        if (uniqueVertices.count(vertex) == 0) {
            uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
            vertices.push_back(vertex);
        }
        indices.push_back(uniqueVertices[vertex]);
    }
    {
        Vertex vertex{};
        vertex.pos = { -0.3f, 0.5f, 0.5f };
        vertex.color = { 0.0f, 0.0f, 1.0f };
        if (uniqueVertices.count(vertex) == 0) {
            uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
            vertices.push_back(vertex);
        }
        indices.push_back(uniqueVertices[vertex]);
    }
}

void Mesh::SetEulerAngle(glm::vec3 euler)
{
    rotation = glm::quat(euler);
}

void Mesh::AddEulerAngle(glm::vec3 euler)
{
    rotation = glm::quat(euler) * rotation;
}

void Mesh::UpdatePushConstants()
{
    transform = glm::scale(glm::mat4(1.0), scale);
    transform = glm::translate(transform, translation);
    transform = transform * toMat4(rotation);
    pushConstants.transform = transform;

}
