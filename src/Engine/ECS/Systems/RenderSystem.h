#pragma once
#include "System.h"

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

};

