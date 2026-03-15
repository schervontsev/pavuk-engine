#pragma once
#include "System.h"
#include <glm/glm.hpp>
#include <string>

namespace vk
{
	class PipelineLayout;
	class CommandBuffer;
}

struct Vertex;

class RenderSystem : public System
{
public:
	void UpdateTransform();
	void UpdateCommandBuffer(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout);
	void UpdateMeshHandles();

    std::vector<Vertex> GetVertices();
	std::vector<uint32_t> GetIndices();

	bool GetFirstEntityPositionByMesh(const std::string& meshName, glm::vec3& outPos) const;
};

