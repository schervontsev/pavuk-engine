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
	void UpdateTransform(float dt);
	void UpdateCommandBuffer(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout);
    std::vector<Vertex> GetVertices();

	std::vector<uint32_t> GetIndices();

};

