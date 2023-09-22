#pragma once
/*
#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../Render/Vertex.h"

struct MeshPushConstants {
	alignas(16) glm::mat4 transform = glm::mat4(1.f);
};

struct MeshOld
{
	MeshOld();

	MeshPushConstants pushConstants;
	glm::mat4 transform = glm::mat4(1.f);

	uint32_t indexCount = 0;

	glm::vec3 translation = glm::vec3(0.f);
	glm::quat rotation = glm::quat(0.f, 0.f, 0.f, 1.f);
	glm::vec3 scale = glm::vec3(1.f);

	void SetEulerAngle(glm::vec3 euler);
	void AddEulerAngle(glm::vec3 euler);

	void UpdatePushConstants();




	void loadModel(int textureCount, const std::string& modelPath, const std::string& modelDir, const std::string& texturePath = "");

	void LoadCube();

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	std::vector<Material> materials;
};

*/