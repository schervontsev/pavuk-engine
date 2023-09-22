#pragma once

struct MeshPushConstants {
	alignas(16) glm::mat4 transform = glm::mat4(1.f);
};

struct RenderComponent {
	//RenderComponent(const std::string& newMeshName) : meshName(newMeshName) {}
	MeshPushConstants pushConstants;

	std::string meshName;
	uint32_t meshId;

	bool isVisible = true;
};