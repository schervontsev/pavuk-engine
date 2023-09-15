#pragma once

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct MeshPushConstants {
	alignas(16) glm::mat4 transform = glm::mat4(1.f);
};

struct Mesh
{
	Mesh();

	MeshPushConstants pushConstants;
	glm::mat4 transform = glm::mat4(1.f);

	uint32_t indexCount = 0;
	uint32_t firstIndex = 0;

	glm::vec3 translation = glm::vec3(0.f);
	glm::quat rotation = glm::quat(0.f, 0.f, 0.f, 1.f);
	glm::vec3 scale = glm::vec3(1.f);

	void SetEulerAngle(glm::vec3 euler);
	void AddEulerAngle(glm::vec3 euler);

	void UpdatePushConstants();
};

