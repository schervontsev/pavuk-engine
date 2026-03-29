#pragma once

struct MeshPushConstants {
	alignas(16) glm::mat4 transform = glm::mat4(1.f);
	alignas(16) glm::mat4 normal_matrix = glm::mat4(1.f);
};

struct RenderComponent {

	MeshPushConstants pushConstants;

	std::string meshName;
	uint32_t meshId;

	bool isVisible = true;

	uint32_t instances = 1;
};