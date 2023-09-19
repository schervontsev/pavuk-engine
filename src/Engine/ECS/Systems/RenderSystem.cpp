#include "RenderSystem.h"

#include <glm/gtx/quaternion.hpp>

#include "../ECSManager.h"
#include "../Components/TransformComponent.h"
#include "../Components/RenderComponent.h"
#include "../../Render/Vertex.h"

void RenderSystem::UpdateTransform(float dt)
{
	for (auto const& entity : entities) {
		auto& transform = ecsManager.GetComponent<TransformComponent>(entity);
		auto& render = ecsManager.GetComponent<RenderComponent>(entity);

		transform.transform = glm::scale(glm::mat4(1.0), transform.scale);
		transform.transform = glm::translate(transform.transform, transform.translation);
		transform.transform = transform.transform * toMat4(transform.rotation);

		render.pushConstants.transform = transform.transform;
	}

}

void RenderSystem::UpdateCommandBuffer(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout)
{
	uint32_t indicesSize = 0;
	int32_t verticesSize = 0;
	for (auto const& entity : entities) {
		auto& render = ecsManager.GetComponent<RenderComponent>(entity);
		//upload the matrix to the GPU via push constants
		commandBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(MeshPushConstants), &render.pushConstants);
		commandBuffer.drawIndexed((uint32_t)render.indices.size(), 1, indicesSize, verticesSize, 0);
		indicesSize += render.indices.size();
		verticesSize += (uint32_t)render.vertices.size();
	}
}

std::vector<Vertex> RenderSystem::GetVertices()
{
	std::vector<Vertex> result;
	for (auto const& entity : entities) {
		auto& render = ecsManager.GetComponent<RenderComponent>(entity);
		result.insert(result.end(), render.vertices.begin(), render.vertices.end());
	}
	return result;
}

std::vector<uint32_t> RenderSystem::GetIndices()
{
	std::vector<uint32_t> result;
	for (auto const& entity : entities) {
		auto& render = ecsManager.GetComponent<RenderComponent>(entity);
		result.insert(result.end(), render.indices.begin(), render.indices.end());
	}
	return result;
}
